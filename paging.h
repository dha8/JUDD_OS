#ifndef PAGING_H
#define PAGING_H

#include "types.h"

#define NUM_PDE 								1024
#define NUM_PTE 								1024
#define SIZE_OF_ENTRY 					4096
#define ALIGN 										12
#define KERNEL_SPACE_OFFSET 0x400000
#define USER_SPACE_OFFSET   0x800000
#define USER_SPACE_SIZE     0x400000
#define READ_WRITE_BIT 		0x00000002
#define PRESENT_BIT 			0x00000001
#define ADDR_START_OFFSET		  0x1000
#define USER_VIRTUAL_ADDR 				32 // 4MB * 32 = 128 MB
#define DENTRY_SHIFT_OFFSET				22

#define MASK_D_P 					0x000003FF

/* memory location for video buf, in phys addr space(cp5) */
#define VID_BUF_ADDR_BASE 				 (0x800000 + 0x400000*6)

typedef struct PDE { //a table
    union {
        uint32_t val;
        struct {
            uint32_t present         : 1;
            uint32_t read_write      : 1;
            uint32_t user_supervisor : 1;
            uint32_t write_through   : 1;
            uint32_t cache_disabled  : 1;
            uint32_t accessed        : 1;
            uint32_t reserved        : 1;
            uint32_t page_size       : 1;
            uint32_t ignored         : 1;
            uint32_t avail           : 3;
            uint32_t page_table_addr : 20;
        } __attribute__ ((packed));
    };
} PDE_t;

typedef struct PTE { //a page
    union {
        uint32_t val;
        struct {
            uint32_t present         : 1;
            uint32_t read_write      : 1;
            uint32_t user_supervisor : 1;
            uint32_t write_through   : 1;
            uint32_t cache_disabled  : 1;
            uint32_t accessed        : 1;
            uint32_t dirty           : 1;
            uint32_t reserved        : 1;
            uint32_t global          : 1;
            uint32_t avail           : 3;
            uint32_t page_addr       : 20;
        } __attribute__ ((packed));
    };
} PTE_t;

// aligned pages on 4 kB boundaries
PDE_t Page_Directory_Entry[NUM_PDE] __attribute__ ((aligned(SIZE_OF_ENTRY)));
PTE_t Page_Table_Entry[NUM_PTE] __attribute__ ((aligned(SIZE_OF_ENTRY)));

PTE_t Page_Table_Entry_For_Video[NUM_PTE] __attribute__ ((aligned(SIZE_OF_ENTRY)));

extern void init_paging();
void set_process_memory(uint32_t PID);
void free_process_memory(uint32_t PID);
void set_up_virtual_to_video(uint8_t tid);
// set new video memory for terminal swapping

#endif
