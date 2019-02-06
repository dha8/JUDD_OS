/* filesystem.h - Defines functions and initialiation for the filesystem 
 * vim:ts=4 noexpandtab
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H 

#include "types.h"
#include "lib.h"

#define FILENAME_SIZE      32
#define RESERVED_SIZE      24
#define DENTRY_SIZE		   64
#define TOTAL_DENTRY_NUM   63
#define BOOTBLOCK_SIZE   4096
#define INODE_SIZE		 4096
#define DATABLOCK_SIZE	 4096
#define BOOTBLOCK_RESERVED_SIZE 52

/* struct holding dir. entry, specifid in Appendix A.
 * at most 63 dir.entries exist in the boot block. 
 * includes: 32B filename, 4B filetype, 4B inode#,
 *           24B reserved.
 */
typedef struct {
	uint8_t filename[FILENAME_SIZE];
	uint32_t filetype;
	uint32_t inode_num;
	uint8_t reserved[RESERVED_SIZE];
} dentry_t;

typedef struct {
	uint32_t num_dentries;
	uint32_t num_inodes;
	uint32_t num_datablocks;
	uint8_t reserved[BOOTBLOCK_RESERVED_SIZE];
} bootblock_t;

/* holds starting addr of the filesystem, set in kernel.c module loading */
extern uint32_t FILESYSTEM_ADDR;

/* holds info regarding opened file */
extern dentry_t opened_file;

extern void filesystem_init();

void clear_dentry(dentry_t * dentry);

/* functions for dentry reading */
int32_t read_dentry_by_name(const uint8_t * fname, dentry_t * dentry);
int32_t read_dentry_by_index(const uint32_t index, dentry_t * dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length);

/* file-related system calls */
int32_t file_open(const uint8_t * filename);
int32_t file_close(int32_t fd);
int32_t file_read(int32_t fd, void * buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void * buf, int32_t nbytes);

int32_t dir_open(const uint8_t * filename);
int32_t dir_close(int32_t fd);
int32_t dir_read(int32_t fd, void * buf, int32_t nbytes);
int32_t dir_write(int32_t fd, const void * buf, int32_t nbytes);


#endif /* FILESYSTEM_H */
