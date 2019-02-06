#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "terminal.h"
#include "filesystem.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test
 *
 * Asserts that first 15 IDT entries are not NULL and system call
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;
	int i;
	int result = PASS;
	for (i = 0; i < 16; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	if ((idt[0x80].offset_15_00 == NULL) &&				//System call index in IDT table
			(idt[0x80].offset_31_16 == NULL)){
				assertion_failure();
				result = FAIL;
			}
	return result;
}

// add more tests here

/* div_by_zero Test
 * 	DESCRIPTION: tests div_by_zero exception by invoking that scenario
 *  Inputs: None
 *  Outputs: print statement to display exception if happening
 *  Side Effects: None
 */
int div_by_zero_test(){
	int result = PASS;
	int i = 1;
	int zero = 0;
	i = i / zero;
	return result;
}

/* overflow Test
 * 	DESCRIPTION: tests overflow exception by invoking that scenario
 *  Inputs: None
 *  Outputs: print statement to display exception if happening
 *  Side Effects: None
 */
int overflow(){
	int result = PASS;
	asm("int $4;");
	return result;
}

/* bound_range_exceeded Test
 * 	DESCRIPTION: tests bound overflow exception by invoking that scenario
 *  Inputs: None
 *  Outputs: print statement to display exception if happening
 *  Side Effects: None
 */
int bound_range_exceeded(){
	asm("int $5;");
	/*
	int test[10];
	test[11] = 10;
	*/
	return PASS;
}

/* invalid opcode Test
 * 	DESCRIPTION: tests invalid opcode exception by invoking invalid opcode
 *  Inputs: None
 *  Outputs: print statement to display exception if happening
 *  Side Effects: None
 */
int invalid_opcode(){
	asm("int $6;");
	return PASS;
}

/* System call Test
 * 	DESCRIPTION: tests System call by invoking one
 *  Inputs: None
 *  Outputs: print statement to display System call happening
 *  Side Effects: None
 */
int system_call(){
	asm("int $0x80;");
	return PASS;

}

/* page fault Test
 * 	DESCRIPTION: tests page fault by invoking a null pointer dereferencing
 *  Inputs: None
 *  Outputs: print statement to display exception if happening
 *  Side Effects: None
 */
int page_fault(){

	int* test_ptr = (int*)0x000000;
	int test = *test_ptr;
	test_ptr = NULL;
	test = *test_ptr;
	(void)test;
	return PASS;

}

/* Checkpoint 2 tests */

/* rtc_test
 * 	DESCRIPTION: check if user can get the frequency in the terminal
 *  Inputs: None
 *  Outputs: None
 *  Side Effects: prints numbers 0-9 in a frequency set by the user
 */
void rtc_test(){
	rtc_open(0);
	char buf[100];
	int32_t size;
	char input[] = "Please put frequency: "; //message to show
	terminal_write(0,input,22); // writes the message in the terminal
	size = terminal_read(0,buf,100); // accept user input as data in buffer
	int sum = 0;
	int i;
	for(i = 0; i < size - 1; i++){ // convert the user input to integer
		sum = sum* 10 + (buf[i] - '0');
	}

    uint32_t rate = sum;
	char output[1];
	if(rtc_write(0,(void*)(&rate),4) == -1){ // put new frequency into rtc
		printf("wrong frequency");
		return;
	}
	i = 0;
	while(1){
		/* spam out numbers according to the set frequency */
		output[0] = '0' + (i + 1) % 10;
		if(0 == rtc_read(0,(void*)(&rate),4)){
			terminal_write(0,output,1);
		}
		i = i + 1;
	}
}

/* terminal_test
 * 	DESCRIPTION: checks if keyboard to terminal print works
 *  Inputs: None
 *  Outputs: None
 *  Side Effects: changes data in screen buffer and video memory
 */
void terminal_test(){
	char buf[1024];
	int32_t size;
	while(1){
		size = terminal_read(0,buf,500); // read keyboard values until enter is pressed
		if (size == -1){ // error handling
			printf("wrong input");
		}
		else{
			terminal_write(0,buf,size); // write data in the buffer again
		}
	}
}

/* dir_read_test
 * 	DESCRIPTION: read filenames in the directory
 *  Inputs: None
 *  Outputs: None
 *  Side Effects: changes data in video memory and writes on the terminal
 */
void dir_read_test(){
	// temp buffer; each filename is at most 32bytes
	uint8_t buf[FILENAME_SIZE];
	uint8_t space = '\n';

	// each call of dir_read reads 1 file. Ret val of 0 indicates end of dir
	while(dir_read(0,buf,0) != 0){
		terminal_write(0,buf,FILENAME_SIZE);
		terminal_write(0,&space,1);
	}
}

/* file_read_test
 * 	DESCRIPTION: checks if we can read the file data
 *  Inputs: None
 *  Outputs: None
 *  Side Effects: changes data in video memory and writes on the terminal
 */
void file_read_test(uint8_t * fname){

	/* file_open tells us the our filesize, while initializing structs needed */
	int32_t file_size = file_open(fname);

	/* check if file failed to open */
	if(file_size == -1){
		printf("file_read fail!\n");
		return;
	}

	/* allocate buffer just big enough to hold file content */
	uint8_t buf[file_size];

	/* read file */
	file_read(0,buf,file_size);

	/* output to screen */
	terminal_write(0,buf,file_size);

}


/*vidmap_test*/

void vidmap_test(){
	int32_t virtual_address = 0x08500000;
	//set_up_virtual_to_video(virtual_address);
	char* addr = (char*) virtual_address;
	addr[0] = 'c';
	int i = 0;
	while(i < 3000){
		i = i + 1;
		addr[i] = 'c';
	}
	while(1){}
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("div_by_zero test", div_by_zero_test());
	//TEST_OUTPUT("overflow test", overflow());
	//TEST_OUTPUT("bound_range_exceeded test", bound_range_exceeded());
	//TEST_OUTPUT("invalid_opcode test", invalid_opcode());
	//TEST_OUTPUT("page_fault test", page_fault());
	//TEST_OUTPUT("System call test", system_call());

	//rtc_test();
	vidmap_test();
	//terminal_test();
	//dir_read_test();
	//uint8_t filename[] = "frame1.txt";
	//uint8_t filename[] = "verylargetextwithverylongname.txt";
	//file_read_test(filename);

}
