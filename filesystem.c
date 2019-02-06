/* filesystem.c - includes function and initializers for the filesystem
 * vim:ts=4 noexpandtab
 */
#include "filesystem.h"
#include "syscalls.h"
/*
 * NOTE ON POINTER ARITHMETIC:
 *   In C, adding an integer to a pointer increases the pointer by
 *   the integer multiplied by the sizeof the type the pointer points to.
 */

/* extern variable in filesystem.h */
uint32_t FILESYSTEM_ADDR;

/* boot block information */
static bootblock_t bootblock;

/* holds the most recently opened file info, for file_open and read */
dentry_t opened_file;

/* holds the idx of most recently opened file, for dir_read */
static uint32_t file_index = 0;


/*filesystem_init
 *  DESCRIPTION: initializes bootblock and opened_file struct
 *  INPUT: none
 *  OUTPUT: none
 */
void filesystem_init(){

	/* initialize our bootblock values */
	bootblock_t * ptr = (bootblock_t*)FILESYSTEM_ADDR;
	bootblock.num_dentries = ptr->num_dentries;
	bootblock.num_inodes = ptr->num_inodes;
	bootblock.num_datablocks = ptr->num_datablocks;
	memcpy(&bootblock.reserved,ptr->reserved,BOOTBLOCK_RESERVED_SIZE);

	/* clear our opened_file struct */
	clear_dentry(&opened_file); // dentry holding opened file info
	file_index = 0; //used for dir_read; indicates idx of newest filename copied for dir_read

	return;
}

/*clear_dentry
 *	DESCRIPTION: helper function, clears the contents of dentry struct
 *  INPUT: pointer to dentry
 *  OUTPUT: none
 */
void clear_dentry(dentry_t * dentry){
	int i;
	for(i=0;i<FILENAME_SIZE;++i)
		dentry->filename[i] = 0x00;
	dentry->filetype = 0;
	dentry->inode_num = 0;		// should probably pick a better default value
	for(i=0;i<RESERVED_SIZE;++i)
		dentry->reserved[i] = 0x00; // for debugging; delete later

	return;
}

/* read_dentry_by_name
 *   DESCRIPTION: copies desired directory entry values into dentry,
 *				  searched by filename.
 *	 INPUT: fname - filename to be read
 * 			dentry - directory entry to copy the values into
 *   OUTPUT: 0 for success, -1 for fail
 */
int32_t read_dentry_by_name(const uint8_t * fname, dentry_t * dentry){

	/* handle bad cases */
	if(dentry == NULL) return -1;

	/* clear out dentry */
	clear_dentry(dentry);

	/* get the ptr to our filesystem */
	uint8_t * ptr = (uint8_t*)FILESYSTEM_ADDR;

	/* find our desired directory entry */
	int i = 0;
	ptr += DENTRY_SIZE; // skip first 64B - contains boot block info

	// skip current file if filename len doesnt align, or str doesnt match
	uint32_t target_file_len;
	while(i <= bootblock.num_dentries){

		// if target filename length > 32, cap it at 32
		target_file_len = (strlen((const int8_t*)ptr) > FILENAME_SIZE) ? FILENAME_SIZE : strlen((const int8_t*)ptr);

	  	// check if file found
		if(
		  (strncmp((int8_t*)fname, (int8_t*)ptr, target_file_len) == 0) &&
		  (target_file_len == strlen((const int8_t*)fname))
		  ) break;

		// move to next file
		ptr += DENTRY_SIZE;
		i++;
	}

	/* check if we traversed beyond bounds */
	if(i >= bootblock.num_dentries) return -1;

	/* otherwise, found our dentry; copy values over */
	dentry_t * found = (dentry_t*)ptr;
	strcpy((int8_t*)dentry->filename, (int8_t*)found->filename);
	dentry->filetype = found->filetype;
	dentry->inode_num = found->inode_num;
	memcpy(dentry->reserved, found->reserved, RESERVED_SIZE);

	/* success */
	return 0;
}

/* read_dentry_by_index
 *   DESCRIPTION: copies desired directory entry values into dentry,
 *				  searched by the index
 *	 INPUT: index - index of file to be read (not inode #)
 * 			dentry - directory entry to copy the values into
 *   OUTPUT: 0 for success, -1 for fail
 */
int32_t read_dentry_by_index(const uint32_t index, dentry_t * dentry){

	/* handle bad cases; index goes from 0 ~ num_inodes-1 */
	if(dentry == NULL || index >= bootblock.num_dentries) return -1;

	/* clear out dentry */
	clear_dentry(dentry);

	/* get the ptr to our filesystem */
	uint8_t * ptr = (uint8_t*)FILESYSTEM_ADDR;

	/* find our desired directory entry */
	ptr += DENTRY_SIZE; // skip first 64B - contains boot block info
	ptr += DENTRY_SIZE * index; // skip until desired dentry index

	/* found our dentry; copy values over */
	dentry_t * found = (dentry_t*)ptr;
	strcpy((int8_t*)dentry->filename, (int8_t*)found->filename);
	dentry->filetype = found->filetype;
	dentry->inode_num = found->inode_num;
	memcpy(dentry->reserved, found->reserved, RESERVED_SIZE);

	/* success */
	return 0;
}

/* read_data
 *   DESCRIPTION: copies desired data found by inode num, offset and length
 *				  into specified buffer.
 *	 INPUT: inode - index of inode to be read
 *			offset - offset added to inode addr to start reading on
 *			length - num of bytes to be read
 *			buf - buffer to copy the data into
 *	 OUTPUT: size of copied data for success, -1 for fail
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length){
	/* check if inode index is beyond what we have */
	if(inode > bootblock.num_inodes-1 || buf == NULL) return -1;

	/* get the ptr to start of inodes */
	uint8_t * inode_ptr = (uint8_t*)FILESYSTEM_ADDR + BOOTBLOCK_SIZE;

	/* get ptr to the inode of desired index */
	inode_ptr += inode * INODE_SIZE;

	/* get the data length of file to be read */
	uint32_t data_length = *((uint32_t*)inode_ptr);

	/* offset should not be longer than filesize */
	if(offset >= data_length) return -1;

	/* calculate how many blocks to skip, and offset within starting block */
	uint32_t block_offset = offset / DATABLOCK_SIZE;
	uint32_t inblock_offset = offset % DATABLOCK_SIZE;

	/* choose minimum size to copy, between actual data len and desired len */
	if (data_length < (offset + length)){
		data_length = data_length - offset;
	}
	data_length = (length > data_length)? data_length : length;
	/* points to datablock_num being copied */
	uint8_t * datablock_num_ptr = inode_ptr + 4*(block_offset+1);

	/* points to start of datablocks */
	uint8_t * datablock_base_ptr = (uint8_t*)FILESYSTEM_ADDR +
		BOOTBLOCK_SIZE + bootblock.num_inodes*INODE_SIZE;

	uint32_t bytes_copied = 0;
	uint32_t datablock_num = *((int32_t*)datablock_num_ptr);
	uint8_t * datablock_ptr = datablock_base_ptr +
								datablock_num*DATABLOCK_SIZE + inblock_offset;
	while(bytes_copied < data_length){
		uint32_t bytes_to_copy =
			((data_length - bytes_copied) >= DATABLOCK_SIZE)?
				DATABLOCK_SIZE : (data_length - bytes_copied);
		memcpy((buf+bytes_copied), datablock_ptr, bytes_to_copy);
		bytes_copied += bytes_to_copy;

		datablock_num_ptr += 4; // point to next datablock_num
		datablock_num = *((int32_t*)datablock_num_ptr);
		datablock_ptr = datablock_base_ptr + datablock_num*DATABLOCK_SIZE;
	}

	return (int32_t)bytes_copied;
}

/* file_open
 *   DESCRIPTION: initialize any temporary structures for file-related functions.
 *   			  for now, read desired file info into our opened_file. Also
 *   			  serves to check if file does exist, and could be opened/read.
 *   INPUT:	filename
 *   OUTPUT: file size upon success; -1 for fail
 */
int32_t file_open(const uint8_t * filename){
	if(read_dentry_by_name(filename, &opened_file) == 0){
		/* get the ptr to start of inodes */
		uint8_t * inode_ptr = (uint8_t*)FILESYSTEM_ADDR + BOOTBLOCK_SIZE;

		/* get ptr to the inode of desired index */
		inode_ptr += opened_file.inode_num * INODE_SIZE;

		/* traverse through each data block #, and copy over the data_length */
		uint32_t data_length = *((uint32_t*)inode_ptr);

		return data_length;
	}
 	return -1;
}

/* file_close
 *   DESCRIPTION:  undo what was initialized in file_open
 *   INPUT: file descriptor
 *   OUTPUT: 0 for success; -1 for fail
 */
int32_t file_close(int32_t fd){
	clear_dentry(&opened_file);
	return 0;
}

/* file_read
 *   DESCRIPTION: reads nbytes of data from file into buf. Must file_open first.
 *   INPUT: fd - file descriptor, buf - buffer to cpy into, nbytes - num bytes
 *   OUTPUT: 0 for success; -1 for fail
 */
int32_t file_read(int32_t fd, void * buf, int32_t nbytes){

	/* clear buf first */
	memset(buf, 0, nbytes);
	pcb_t* current_pcb = get_curr_pcb();
	uint32_t current_inode = current_pcb->fd_array[fd].inode;
	uint32_t current_position = current_pcb->fd_array[fd].file_pos;
	/* read nbytes from file into buf */

	int32_t read_bytes = read_data(current_inode, current_position, buf, nbytes);
	if (read_bytes != -1){
		current_pcb->fd_array[fd].file_pos = current_position + read_bytes;
		return read_bytes;
	}
	return 0;
}

/* file_write
 *   DESCRIPTION: does nothing; this is a read-only filesystem
 *   INPUT: file descriptor, buffer, num bytes
 *   OUTPUT: -1
 */
int32_t file_write(int32_t fd, const void * buf, int32_t nbytes){
	return -1;
}

/* dir_open
 *   DESCRIPTION: supposedly init necessary vars for dir reading, but does
 *   			  nothing fow now;
 *   INPUT: filename
 *   OUTPUT: 0
 */
int32_t dir_open(const uint8_t * filename){
	return 0;
}

/* dir_close
 *   DESCRIPTION: does nothing, undo dir_open if needed
 *   INPUT: fd - file descriptor
 *   OUTPUT: 0
 */
int32_t dir_close(int32_t fd){
	return 0;
}

/* dir_read
 *   DESCRIPTION: reads files filename by filename(one at a time), including "."
 *   INPUT: fd - file descriptor, buf - buffer to cpy content to, nbytes-unused
 *   OUTPUT: number of bytes copied, -1 for fail. 0 indicates end of directory
 */
int32_t dir_read(int32_t fd, void * buf, int32_t nbytes){

	// reached end of directory: return 0 to indicate so
	if(file_index == bootblock.num_dentries){
		file_index = 0;
		return 0;
	}

	// temporary dentry to hold the read file
	dentry_t read_file;

	// check if file could be read at the specified index
	if(read_dentry_by_index(file_index,&read_file) == 0){
		// copy the name into our buffer
		int32_t copied_bytes = strlen((int8_t*)read_file.filename);
		if (copied_bytes > nbytes){
			copied_bytes = nbytes;
		}
		memcpy((int8_t*)buf,(int8_t*)read_file.filename,FILENAME_SIZE/*copied_bytes*/);
		++file_index;
		return copied_bytes;
		//return 0;
	}else{
		printf("dir_read error:read_dentry_by_idx failed @ idx %d\n",file_index);
		return -1;
	}
}

/* dir_write
 *   DESCRIPTION: does nothing
 *   INPUT: none of these matter
 *   OUTPUT: -1
 */
int32_t dir_write(int32_t fd, const void * buf, int32_t nbytes){
	return -1;
}
