# Interrupt Service Routine; Assembly wrapper that calls handlers in c
# for reasons why we need Assembly wrapper functions, see the following
# reference:
# 	https://wiki.osdev.org/Interrupt_Service_Routines

# allocate labels(pointers) to specific handlers
# exceptions
.globl PAGE_FAULT
.globl DIV_BY_ZERO
.globl SINGLE_STEP_INT
.globl NMI
.globl BREAKPOINT
.globl OVERFLOW
.globl BOUNDS
.globl INVALID_OPCODE
.globl COPROCESSOR_NA
.globl DOUBLE_FAULT
.globl COPROCESSOR_SO
.globl INVALID_TSS
.globl SEG_NOT_PRESENT
.globl STACK_FAULT
.globl GENERAL_PFAULT
.globl RESERVED
.globl MATH_FAULT
.globl ALIGNMENT_CHECK
.globl MACHINE_CHECK
.globl SIMD_FP_EXCEP
.globl VIRTUAL_EXCEP
.globl CTRL_PROT_EXCEP

# interrupts
.globl IRQ_KEYBOARD
.globl IRQ_RTC
.globl IRQ_PIT
# system calls
.globl SYSTEM_CALL

# For all Exceptions Below:
#   DESCRIPTION: wrapper function for interrupt handlers in c. save flags and regs
#   INPUT: none
#   OUTPUT: none
#   SIDE_EFFECT: save registers. call int handlers, restore registers.

PAGE_FAULT:
	pushal   # save all regs
	pushfl   # push all flags
	call PAGE_FAULT_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

DIV_BY_ZERO:
	pushal   # save all regs
	pushfl   # push all flags
	call DIV_BY_ZERO_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

SINGLE_STEP_INT:
	pushal   # save all regs
	pushfl   # push all flags
	call SINGLE_STEP_INT_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

NMI:
	pushal   # save all regs
	pushfl   # push all flags
	call NMI_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

BREAKPOINT:
	pushal   # save all regs
	pushfl   # push all flags
	call BREAKPOINT_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

OVERFLOW:
	pushal   # save all regs
	pushfl   # push all flags
	call OVERFLOW_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

BOUNDS:
	pushal   # save all regs
	pushfl   # push all flags
	call BOUNDS_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

INVALID_OPCODE:
	pushal   # save all regs
	pushfl   # push all flags
	call INVALID_OPCODE_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

COPROCESSOR_NA:
	pushal   # save all regs
	pushfl   # push all flags
	call COPROCESSOR_NA_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

DOUBLE_FAULT:
	pushal   # save all regs
	pushfl   # push all flags
	call DOUBLE_FAULT_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

COPROCESSOR_SO:
	pushal   # save all regs
	pushfl   # push all flags
	call COPROCESSOR_SO_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

INVALID_TSS:
	pushal   # save all regs
	pushfl   # push all flags
	call INVALID_TSS_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

SEG_NOT_PRESENT:
	pushal   # save all regs
	pushfl   # push all flags
	call SEG_NOT_PRESENT_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

STACK_FAULT:
	pushal   # save all regs
	pushfl   # push all flags
	call STACK_FAULT_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

GENERAL_PFAULT:
	pushal   # save all regs
	pushfl   # push all flags
	call GENERAL_PFAULT_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

RESERVED:
	pushal   # save all regs
	pushfl   # push all flags
	call RESERVED_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

MATH_FAULT:
	pushal   # save all regs
	pushfl   # push all flags
	call MATH_FAULT_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

ALIGNMENT_CHECK:
	pushal   # save all regs
	pushfl   # push all flags
	call ALIGNMENT_CHECK_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

MACHINE_CHECK:
	pushal   # save all regs
	pushfl   # push all flags
	call MACHINE_CHECK_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

SIMD_FP_EXCEP:
	pushal   # save all regs
	pushfl   # push all flags
	call SIMD_FP_EXCEP_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

VIRTUAL_EXCEP:
	pushal   # save all regs
	pushfl   # push all flags
	call VIRTUAL_EXCEP_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

CTRL_PROT_EXCEP:
	pushal   # save all regs
	pushfl   # push all flags
	call CTRL_PROT_EXCEP_handler
	addl $4, %esp
	popfl    # pop all flags
	popal    # restore all regs
	iret

IRQ_KEYBOARD:
	pushal	# save all regs
	pushfl	# push all flags
	call IRQ_KEYBOARD_handler
	popfl		# pop all flags
	popal		# restore all regs
	iret

IRQ_RTC:
	pushal	# save all regs
	pushfl	# push all flags
	call IRQ_RTC_handler
	popfl		# pop all flags
	popal		# restore all regs
	iret

IRQ_PIT:
	pushal	# save all regs
	pushfl	# push all flags
	call IRQ_PIT_handler
	popfl		# pop all flags
	popal		# restore all regs
	iret


#notice, cli is called immediately but sti is never called after jumping to the syscall functions
#or if syscall is immediately identified as invalid within it's sycall function,
#this is because our syscall implementation always does an effective sti
SYSTEM_CALL: #securely setup full stack
	cli
	#save user's normal registers
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi
	pushl %ebp
	#save user's segment registers
	pushl %ds
	pushl %es
	pushl %fs
	#save user's flag registers
	pushfl
	sti
	#incoming system call number error checking
	decl %eax #0 index the call number
	cmpl $0, %eax # if call number (eax) < 0
	jl INVALID_CALL
	cmpl $9, %eax # if call number (eax) > 9
	jg INVALID_CALL

	#call systemcall function
	pushl %edx #pass third argument
	pushl %ecx #pass second argument
	pushl %ebx #pass first argument
	call *syscalls_fxns_jmp(,%eax,4)
	addl $12, %esp #pop the 3 arguments, each 3 bytes

	#syscall function return handling and errorc hecking
	cmpl $0, %eax #if the system call returns something less then 0,
	jl SYSCALL_FAIL #then the args were invalid, error accordingly
	jmp DONE_ #and leave normally

#somehow notify user and kwernel of either an invalid call number
#or that the system call has failed
INVALID_CALL:
SYSCALL_FAIL:
	movl $-1, %eax
	sti

#teardown full stack frame
DONE_:
	#restore user's flag registers
	popfl
	#restore user's segment registers
	popl %fs
	popl %es
	popl %ds
	#save user's normal registers
	popl %ebp
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	#return
	iret

#systemcall functions name list to jump to in the .c
syscalls_fxns_jmp:
	.long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
