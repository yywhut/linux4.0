#ifndef __ASM_MEMORY_MODEL_H
#define __ASM_MEMORY_MODEL_H

#ifndef __ASSEMBLY__

#if defined(CONFIG_FLATMEM)

#ifndef ARCH_PFN_OFFSET
#define ARCH_PFN_OFFSET		(0UL)
#endif

#elif defined(CONFIG_DISCONTIGMEM)

#ifndef arch_pfn_to_nid
#define arch_pfn_to_nid(pfn)	pfn_to_nid(pfn)
#endif

#ifndef arch_local_page_offset
#define arch_local_page_offset(pfn, nid)	\
	((pfn) - NODE_DATA(nid)->node_start_pfn)
#endif

#endif /* CONFIG_DISCONTIGMEM */

/*
 * supports 3 memory models.
 */
#if defined(CONFIG_FLATMEM)

#define __pfn_to_page(pfn)	(mem_map + ((pfn) - ARCH_PFN_OFFSET))

//把page转化成pfn   地址相减，mem_map 肯定是 0x1000 类似的，然后第二个肯定是 0x1004类似的。
/*
指针之间可以相减，但不可以相加：两个同一类型的指针变量是可以相减的，他们的意义表示两个指针指
向的内存位置之间相隔多少个元素(注意是元素，并不是字节数)
*/
//page 类型占用0x20多个字节

//(unsigned long)((page) - mem_map) 是看的指针计算，所以结果要除以0x20
// 但是这里算完了之后，被大括号强制转化为(unsigned long)， 再加ARCH_PFN_OFFSET，就是数字加数字了，
#define __page_to_pfn(page)	((unsigned long)((page) - mem_map) + \
				 ARCH_PFN_OFFSET)       //ARCH_PFN_OFFSET = 0x60000
#elif defined(CONFIG_DISCONTIGMEM)

#define __pfn_to_page(pfn)			\
({	unsigned long __pfn = (pfn);		\
	unsigned long __nid = arch_pfn_to_nid(__pfn);  \
	NODE_DATA(__nid)->node_mem_map + arch_local_page_offset(__pfn, __nid);\
})

#define __page_to_pfn(pg)						\
({	const struct page *__pg = (pg);					\
	struct pglist_data *__pgdat = NODE_DATA(page_to_nid(__pg));	\
	(unsigned long)(__pg - __pgdat->node_mem_map) +			\
	 __pgdat->node_start_pfn;					\
})

#elif defined(CONFIG_SPARSEMEM_VMEMMAP)

/* memmap is virtually contiguous.  */
#define __pfn_to_page(pfn)	(vmemmap + (pfn))
#define __page_to_pfn(page)	(unsigned long)((page) - vmemmap)

#elif defined(CONFIG_SPARSEMEM)
/*
 * Note: section's mem_map is encoded to reflect its start_pfn.
 * section[i].section_mem_map == mem_map's address - start_pfn;
 */
#define __page_to_pfn(pg)					\
({	const struct page *__pg = (pg);				\
	int __sec = page_to_section(__pg);			\
	(unsigned long)(__pg - __section_mem_map_addr(__nr_to_section(__sec)));	\
})

#define __pfn_to_page(pfn)				\
({	unsigned long __pfn = (pfn);			\
	struct mem_section *__sec = __pfn_to_section(__pfn);	\
	__section_mem_map_addr(__sec) + __pfn;		\
})
#endif /* CONFIG_FLATMEM/DISCONTIGMEM/SPARSEMEM */

#define page_to_pfn __page_to_pfn
#define pfn_to_page __pfn_to_page

#endif /* __ASSEMBLY__ */

#endif
