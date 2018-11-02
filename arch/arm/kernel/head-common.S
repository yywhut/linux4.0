/*
 *  linux/arch/arm/kernel/head-common.S
 *
 *  Copyright (C) 1994-2002 Russell King
 *  Copyright (c) 2003 ARM Limited
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <asm/assembler.h>

#define ATAG_CORE 0x54410001
#define ATAG_CORE_SIZE ((2*4 + 3*4) >> 2)
#define ATAG_CORE_SIZE_EMPTY ((2*4) >> 2)

#ifdef CONFIG_CPU_BIG_ENDIAN
#define OF_DT_MAGIC 0xd00dfeed
#else
#define OF_DT_MAGIC 0xedfe0dd0 /* 0xd00dfeed in big-endian */
#endif

/*
 * Exception handling.  Something went wrong and we can't proceed.  We
 * ought to tell the user, but since we don't have any guarantee that
 * we're even running on the right architecture, we do virtually nothing.
 *
 * If CONFIG_DEBUG_LL is set we try to print out something about the error
 * and hope for the best (useful if bootloader fails to pass a proper
 * machine ID for example).
 */
	__HEAD

/* Determine validity of the r2 atags pointer.  The heuristic requires
 * that the pointer be aligned, in the first 16k of physical RAM and
 * that the ATAG_CORE marker is first and present.  If CONFIG_OF_FLATTREE
 * is selected, then it will also accept a dtb pointer.  Future revisions
 * of this function may be more lenient with the physical address and
 * may also be able to move the ATAGS block if necessary.
 *
 * Returns:
 *  r2 either valid atags pointer, valid dtb pointer, or zero
 *  r5, r6 corrupted
 */
__vet_atags:
	tst	r2, #0x3			@ aligned?
	bne	1f

	ldr	r5, [r2, #0]
#ifdef CONFIG_OF_FLATTREE
	ldr	r6, =OF_DT_MAGIC		@ is it a DTB?
	cmp	r5, r6
	beq	2f
#endif
	cmp	r5, #ATAG_CORE_SIZE		@ is first tag ATAG_CORE?
	cmpne	r5, #ATAG_CORE_SIZE_EMPTY
	bne	1f
	ldr	r5, [r2, #4]
	ldr	r6, =ATAG_CORE
	cmp	r5, r6
	bne	1f

2:	ret	lr				@ atag/dtb pointer is ok

1:	mov	r2, #0
	ret	lr
ENDPROC(__vet_atags)

/*
 * The following fragment of code is executed with the MMU on in MMU mode,
 * and uses absolute addresses; this is not position independent.
 *
 *  r0  = cp#15 control register
 *  r1  = machine ID
 *  r2  = atags/dtb pointer
 *  r9  = processor ID
 */
	__INIT
__mmap_switched:
	adr	r3, __mmap_switched_data  取__mmap_switched_data的地址到r3. 这个地址就是第109行的地址.

	ldmia	r3!, {r4, r5, r6, r7} @并且累加r3的值

	@ 将__mmap_switched_data(r3)上的值分别加载到r4、r5、r6、r7寄存器中，__mmap_switched_data前面说明了
	@ 经过上述动作，r4、r5、r6、r7寄存器分别存放了如下值
	@ r4 -> __data_loc：数据段存储地址
	@ r5 -> _sdata：数据段起始地址
	@ r6 -> __bss_start：堆栈段起始地址
	@ r7 -> _end：堆栈段结束地址
									
	cmp	r4, r5				@ Copy data segment if needed  比较 __data_loc 和 _sdata
1:	cmpne	r5, r6
	ldrne	fp, [r4], #4
	strne	fp, [r5], #4
	bne	1b
	@ 判断数据段存储地址(r4)和数据段起始地址(r5)
@ 如果不一样的话需要搬移到数据段起始地址(r5)上。

	mov	fp, #0				@ Clear BSS (and zero fp)
1:	cmp	r6, r7
	strcc	fp, [r6],#4
	bcc	1b

	@ 清空堆栈段。
	@ 从堆栈段起始地址(r6)开始写入0，一直写到地址为堆栈段结束地址(r7)

 ARM(	ldmia	r3, {r4, r5, r6, r7, sp})    这里又是把地址放到后面的寄存器中
 THUMB(	ldmia	r3, {r4, r5, r6, r7}	)
 THUMB(	ldr	sp, [r3, #16]		)
 
@ 继续将__mmap_switched_data(r3)上的值分别加载到r4、r5、r6、r7、sp寄存器中，注意是前面r3已经加载过一部分了，地址和__mmap_switched_data已经不一样了。
@ 经过上述动作，r4、r5、r6、r7寄存器分别存放了如下值
@ r4 -> processor_id变量地址：其内容是cpu处理器ID
@ r5 -> __machine_arch_type变量地址：其内容是machine id
@ r6 -> __atags_pointer变量地址：其内容是dtb的地址
@ r7 -> cr_alignment变量地址：其内容是cp15的c1的寄存器的值
@ sp->  init_thread_union + THREAD_START_SP，设置了当前进程的堆栈


 
	str	r9, [r4]			@ Save processor ID
	@ 把cpu处理器id(r9)放到processor_id变量中([r4])

	str	r1, [r5]			@ Save machine type
	@ 把mechine id(r1)存放到__machine_arch_type变量中([r5])
	
	str	r2, [r6]			@ Save atags pointer   把r2中的内容保存到r6所指向的存储单元中
	@ 把dtb的地址指针(r2)存放到__atags_pointer变量中([r6])
	
	cmp	r7, #0
	strne	r0, [r7]			@ Save control register values
	b	start_kernel
ENDPROC(__mmap_switched)

	.align	2
	.type	__mmap_switched_data, %object

	@留意这里放的都是地址,韦东山也说了这里存放的是变量的地址
__mmap_switched_data:
	.long	__data_loc			@ r4     // 数据段的起始地址
	.long	_sdata				@ r5			
	.long	__bss_start			@ r6		//bss段的起始地址
	.long	_end				@ r7
	.long	processor_id			@ r4     //cpu处理器ID地址，其变量定义在arch/arm/kernel/setup.c中
	.long	__machine_arch_type		@ r5    //machine id地址，其变量定义在arch/arm/kernel/setup.c中
	.long	__atags_pointer			@ r6    // device tree 地址 其变量定义在arch/arm/kernel/setup.c中
#ifdef CONFIG_CPU_CP15
	.long	cr_alignment			@ r7  //cp15的c1寄存器的值的地址，也就是mmu控制寄存器的值，其变量定义在arch/arm/kernel/entry-armv.S中
#else 
	.long	0				@ r7
#endif
	.long	init_thread_union + THREAD_START_SP @ sp
	.size	__mmap_switched_data, . - __mmap_switched_data

/*
 * This provides a C-API version of __lookup_processor_type
 */
ENTRY(lookup_processor_type)
	stmfd	sp!, {r4 - r6, r9, lr}
	mov	r9, r0
	bl	__lookup_processor_type
	mov	r0, r5
	ldmfd	sp!, {r4 - r6, r9, pc}
ENDPROC(lookup_processor_type)

	__FINIT
	.text

/*
 * Read processor ID register (CP#15, CR0), and look up in the linker-built
 * supported processor list.  Note that we can't use the absolute addresses
 * for the __proc_info lists since we aren't running with the MMU on
 * (and therefore, we are not in the correct address space).  We have to
 * calculate the offset.
 *
 *	r9 = cpuid
 * Returns:
 *	r3, r4, r6 corrupted
 *	r5 = proc_info pointer in physical address space
 *	r9 = cpuid (preserved)
 */
__lookup_processor_type:
	adr	r3, __lookup_processor_type_data
	ldmia	r3, {r4 - r6}
	sub	r3, r3, r4			@ get offset between virt&phys
	add	r5, r5, r3			@ convert virt addresses to
	add	r6, r6, r3			@ physical address space
1:	ldmia	r5, {r3, r4}			@ value, mask
	and	r4, r4, r9			@ mask wanted bits
	teq	r3, r4
	beq	2f
	add	r5, r5, #PROC_INFO_SZ		@ sizeof(proc_info_list)
	cmp	r5, r6
	blo	1b
	mov	r5, #0				@ unknown processor
2:	ret	lr
ENDPROC(__lookup_processor_type)

/*
 * Look in <asm/procinfo.h> for information about the __proc_info structure.
 */
	.align	2
	.type	__lookup_processor_type_data, %object
__lookup_processor_type_data:
	.long	.
	.long	__proc_info_begin
	.long	__proc_info_end
	.size	__lookup_processor_type_data, . - __lookup_processor_type_data

__error_lpae:
#ifdef CONFIG_DEBUG_LL
	adr	r0, str_lpae
	bl 	printascii
	b	__error
str_lpae: .asciz "\nError: Kernel with LPAE support, but CPU does not support LPAE.\n"
#else
	b	__error
#endif
	.align
ENDPROC(__error_lpae)

__error_p:
#ifdef CONFIG_DEBUG_LL
	adr	r0, str_p1
	bl	printascii
	mov	r0, r9
	bl	printhex8
	adr	r0, str_p2
	bl	printascii
	b	__error
str_p1:	.asciz	"\nError: unrecognized/unsupported processor variant (0x"
str_p2:	.asciz	").\n"
	.align
#endif
ENDPROC(__error_p)

__error:
#ifdef CONFIG_ARCH_RPC
/*
 * Turn the screen red on a error - RiscPC only.
 */
	mov	r0, #0x02000000
	mov	r3, #0x11
	orr	r3, r3, r3, lsl #8
	orr	r3, r3, r3, lsl #16
	str	r3, [r0], #4
	str	r3, [r0], #4
	str	r3, [r0], #4
	str	r3, [r0], #4
#endif
1:	mov	r0, r0
	b	1b
ENDPROC(__error)
