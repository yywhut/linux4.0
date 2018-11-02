#ifndef _LINUX_MEMBLOCK_H
#define _LINUX_MEMBLOCK_H
#ifdef __KERNEL__

#ifdef CONFIG_HAVE_MEMBLOCK
/*
 * Logical memory blocks.
 *
 * Copyright (C) 2001 Peter Bergner, IBM Corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/init.h>
#include <linux/mm.h>

#define INIT_MEMBLOCK_REGIONS	128
#define INIT_PHYSMEM_REGIONS	4

/* Definition of memblock flags. */
#define MEMBLOCK_HOTPLUG	0x1	/* hotpluggable region */

struct memblock_region {
	phys_addr_t base; //内存区域起始地址，是物理地址
	phys_addr_t size;//内存区域大小，单位是字节
	unsigned long flags;  //该内存区域的标识，例如MEMBLOCK_NOMAP，在做映射的时候不要映射到内核中
#ifdef CONFIG_HAVE_MEMBLOCK_NODE_MAP
	int nid;      //CPU被划分为多个节点(node)，每个node 有对应的内存簇bank，一个标识
#endif
};

struct memblock_type {
	unsigned long cnt;	/* number of regions */ //当前集合(memory或者reserved)中记录的内存区域个数
	unsigned long max;	/* size of the allocated array */ //当前集合(memory或者reserved)中可记录的内存区域的最大个数
	phys_addr_t total_size;	/* size of all regions */   //集合记录的内存总和
	struct memblock_region *regions;   //执行内存区域结构（memblock_region）的指针
};

struct memblock {
	bool bottom_up;  /* is bottom up direction? */  //表示分配器分配内存的方式 true:从低地址向高地址分配  false:相反就是从高地址向地址分配内存.
	phys_addr_t current_limit;     //指出了内存块的大小限制
	struct memblock_type memory;   //可分配内存的集合，申请内存时，会从这些集合中分配内存
	struct memblock_type reserved;    //已分配内存的集合，分配出去的内存会放在这个集合里面管理
#ifdef CONFIG_HAVE_MEMBLOCK_PHYS_MAP  
	struct memblock_type physmem;  //物理内存的集合
#endif
};

extern struct memblock memblock;
extern int memblock_debug;
#ifdef CONFIG_MOVABLE_NODE
/* If movable_node boot option specified */
extern bool movable_node_enabled;
#endif /* CONFIG_MOVABLE_NODE */

#define memblock_dbg(fmt, ...) \
	if (memblock_debug) printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)

phys_addr_t memblock_find_in_range_node(phys_addr_t size, phys_addr_t align,
					    phys_addr_t start, phys_addr_t end,
					    int nid);
phys_addr_t memblock_find_in_range(phys_addr_t start, phys_addr_t end,
				   phys_addr_t size, phys_addr_t align);
phys_addr_t get_allocated_memblock_reserved_regions_info(phys_addr_t *addr);
phys_addr_t get_allocated_memblock_memory_regions_info(phys_addr_t *addr);
void memblock_allow_resize(void);
int memblock_add_node(phys_addr_t base, phys_addr_t size, int nid);
int memblock_add(phys_addr_t base, phys_addr_t size);
int memblock_remove(phys_addr_t base, phys_addr_t size);
int memblock_free(phys_addr_t base, phys_addr_t size);
int memblock_reserve(phys_addr_t base, phys_addr_t size);
void memblock_trim_memory(phys_addr_t align);
int memblock_mark_hotplug(phys_addr_t base, phys_addr_t size);
int memblock_clear_hotplug(phys_addr_t base, phys_addr_t size);

/* Low level functions */
int memblock_add_range(struct memblock_type *type,
		       phys_addr_t base, phys_addr_t size,
		       int nid, unsigned long flags);

int memblock_remove_range(struct memblock_type *type,
			  phys_addr_t base,
			  phys_addr_t size);

void __next_mem_range(u64 *idx, int nid, struct memblock_type *type_a,
		      struct memblock_type *type_b, phys_addr_t *out_start,
		      phys_addr_t *out_end, int *out_nid);

void __next_mem_range_rev(u64 *idx, int nid, struct memblock_type *type_a,
			  struct memblock_type *type_b, phys_addr_t *out_start,
			  phys_addr_t *out_end, int *out_nid);

/**
 * for_each_mem_range - iterate through memblock areas from type_a and not
 * included in type_b. Or just type_a if type_b is NULL.
 * @i: u64 used as loop variable
 * @type_a: ptr to memblock_type to iterate
 * @type_b: ptr to memblock_type which excludes from the iteration
 * @nid: node selector, %NUMA_NO_NODE for all nodes
 * @p_start: ptr to phys_addr_t for start address of the range, can be %NULL
 * @p_end: ptr to phys_addr_t for end address of the range, can be %NULL
 * @p_nid: ptr to int for nid of the range, can be %NULL
 */
#define for_each_mem_range(i, type_a, type_b, nid,			\
			   p_start, p_end, p_nid)			\
	for (i = 0, __next_mem_range(&i, nid, type_a, type_b,		\
				     p_start, p_end, p_nid);		\
	     i != (u64)ULLONG_MAX;					\
	     __next_mem_range(&i, nid, type_a, type_b,			\
			      p_start, p_end, p_nid))

/**
 * for_each_mem_range_rev - reverse iterate through memblock areas from
 * type_a and not included in type_b. Or just type_a if type_b is NULL.
 * @i: u64 used as loop variable
 * @type_a: ptr to memblock_type to iterate
 * @type_b: ptr to memblock_type which excludes from the iteration
 * @nid: node selector, %NUMA_NO_NODE for all nodes
 * @p_start: ptr to phys_addr_t for start address of the range, can be %NULL
 * @p_end: ptr to phys_addr_t for end address of the range, can be %NULL
 * @p_nid: ptr to int for nid of the range, can be %NULL
 */
#define for_each_mem_range_rev(i, type_a, type_b, nid,			\
			       p_start, p_end, p_nid)			\
	for (i = (u64)ULLONG_MAX,__next_mem_range_rev(&i, nid, type_a, type_b,p_start, p_end, p_nid);	\
	     i != (u64)ULLONG_MAX;					\
	     __next_mem_range_rev(&i, nid, type_a, type_b,p_start, p_end, p_nid))

#ifdef CONFIG_MOVABLE_NODE
static inline bool memblock_is_hotpluggable(struct memblock_region *m)
{
	return m->flags & MEMBLOCK_HOTPLUG;
}

static inline bool movable_node_is_enabled(void)
{
	return movable_node_enabled;
}
#else
static inline bool memblock_is_hotpluggable(struct memblock_region *m)
{
	return false;
}
static inline bool movable_node_is_enabled(void)
{
	return false;
}
#endif

#ifdef CONFIG_HAVE_MEMBLOCK_NODE_MAP
int memblock_search_pfn_nid(unsigned long pfn, unsigned long *start_pfn,
			    unsigned long  *end_pfn);
void __next_mem_pfn_range(int *idx, int nid, unsigned long *out_start_pfn,
			  unsigned long *out_end_pfn, int *out_nid);

/**
 * for_each_mem_pfn_range - early memory pfn range iterator
 * @i: an integer used as loop variable
 * @nid: node selector, %MAX_NUMNODES for all nodes
 * @p_start: ptr to ulong for start pfn of the range, can be %NULL
 * @p_end: ptr to ulong for end pfn of the range, can be %NULL
 * @p_nid: ptr to int for nid of the range, can be %NULL
 *
 * Walks over configured memory ranges.
 */
#define for_each_mem_pfn_range(i, nid, p_start, p_end, p_nid)		\
	for (i = -1, __next_mem_pfn_range(&i, nid, p_start, p_end, p_nid); \
	     i >= 0; __next_mem_pfn_range(&i, nid, p_start, p_end, p_nid))
#endif /* CONFIG_HAVE_MEMBLOCK_NODE_MAP */

/**
 * for_each_free_mem_range - iterate through free memblock areas
 * @i: u64 used as loop variable
 * @nid: node selector, %NUMA_NO_NODE for all nodes
 * @p_start: ptr to phys_addr_t for start address of the range, can be %NULL
 * @p_end: ptr to phys_addr_t for end address of the range, can be %NULL
 * @p_nid: ptr to int for nid of the range, can be %NULL
 *
 * Walks over free (memory && !reserved) areas of memblock.  Available as
 * soon as memblock is initialized.
 */
 //遍历memblock算法中的空闲内存空间
#define for_each_free_mem_range(i, nid, p_start, p_end, p_nid)		\
	for_each_mem_range(i, &memblock.memory, &memblock.reserved,	\
			   nid, p_start, p_end, p_nid)

/**
 * for_each_free_mem_range_reverse - rev-iterate through free memblock areas
 * @i: u64 used as loop variable
 * @nid: node selector, %NUMA_NO_NODE for all nodes
 * @p_start: ptr to phys_addr_t for start address of the range, can be %NULL
 * @p_end: ptr to phys_addr_t for end address of the range, can be %NULL
 * @p_nid: ptr to int for nid of the range, can be %NULL
 *
 * Walks over free (memory && !reserved) areas of memblock in reverse
 * order.  Available as soon as memblock is initialized.
 */
#define for_each_free_mem_range_reverse(i, nid, p_start, p_end, p_nid)	\
	for_each_mem_range_rev(i, &memblock.memory, &memblock.reserved,	\
			       nid, p_start, p_end, p_nid)

static inline void memblock_set_region_flags(struct memblock_region *r,
					     unsigned long flags)
{
	r->flags |= flags;
}

static inline void memblock_clear_region_flags(struct memblock_region *r,
					       unsigned long flags)
{
	r->flags &= ~flags;
}

#ifdef CONFIG_HAVE_MEMBLOCK_NODE_MAP
int memblock_set_node(phys_addr_t base, phys_addr_t size,
		      struct memblock_type *type, int nid);

static inline void memblock_set_region_node(struct memblock_region *r, int nid)
{
	r->nid = nid;
}

static inline int memblock_get_region_node(const struct memblock_region *r)
{
	return r->nid;
}
#else
static inline void memblock_set_region_node(struct memblock_region *r, int nid)
{
}

static inline int memblock_get_region_node(const struct memblock_region *r)
{
	return 0;
}
#endif /* CONFIG_HAVE_MEMBLOCK_NODE_MAP */

phys_addr_t memblock_alloc_nid(phys_addr_t size, phys_addr_t align, int nid);
phys_addr_t memblock_alloc_try_nid(phys_addr_t size, phys_addr_t align, int nid);

phys_addr_t memblock_alloc(phys_addr_t size, phys_addr_t align);

#ifdef CONFIG_MOVABLE_NODE
/*
 * Set the allocation direction to bottom-up or top-down.
 */
static inline void __init memblock_set_bottom_up(bool enable)
{
	memblock.bottom_up = enable;
}

/*
 * Check if the allocation direction is bottom-up or not.
 * if this is true, that said, memblock will allocate memory
 * in bottom-up direction.
 */
static inline bool memblock_bottom_up(void)
{
	return memblock.bottom_up;
}
#else
static inline void __init memblock_set_bottom_up(bool enable) {}
static inline bool memblock_bottom_up(void) { return false; }
#endif

/* Flags for memblock_alloc_base() amd __memblock_alloc_base() */
#define MEMBLOCK_ALLOC_ANYWHERE	(~(phys_addr_t)0)
#define MEMBLOCK_ALLOC_ACCESSIBLE	0

phys_addr_t __init memblock_alloc_range(phys_addr_t size, phys_addr_t align,
					phys_addr_t start, phys_addr_t end);
phys_addr_t memblock_alloc_base(phys_addr_t size, phys_addr_t align,
				phys_addr_t max_addr);
phys_addr_t __memblock_alloc_base(phys_addr_t size, phys_addr_t align,
				  phys_addr_t max_addr);
phys_addr_t memblock_phys_mem_size(void);
phys_addr_t memblock_mem_size(unsigned long limit_pfn);
phys_addr_t memblock_start_of_DRAM(void);
phys_addr_t memblock_end_of_DRAM(void);
void memblock_enforce_memory_limit(phys_addr_t memory_limit);
int memblock_is_memory(phys_addr_t addr);
int memblock_is_region_memory(phys_addr_t base, phys_addr_t size);
int memblock_is_reserved(phys_addr_t addr);
int memblock_is_region_reserved(phys_addr_t base, phys_addr_t size);

extern void __memblock_dump_all(void);

static inline void memblock_dump_all(void)
{
	if (memblock_debug)
		__memblock_dump_all();
}

/**
 * memblock_set_current_limit - Set the current allocation limit to allow
 *                         limiting allocations to what is currently
 *                         accessible during boot
 * @limit: New limit value (physical address)
 */
void memblock_set_current_limit(phys_addr_t limit);


phys_addr_t memblock_get_current_limit(void);

/*
 * pfn conversion functions
 *
 * While the memory MEMBLOCKs should always be page aligned, the reserved
 * MEMBLOCKs may not be. This accessor attempt to provide a very clear
 * idea of what they return for such non aligned MEMBLOCKs.
 */

/**
 * memblock_region_memory_base_pfn - Return the lowest pfn intersecting with the memory region
 * @reg: memblock_region structure
 */
static inline unsigned long memblock_region_memory_base_pfn(const struct memblock_region *reg)
{
	return PFN_UP(reg->base);
}

/**
 * memblock_region_memory_end_pfn - Return the end_pfn this region
 * @reg: memblock_region structure
 */
static inline unsigned long memblock_region_memory_end_pfn(const struct memblock_region *reg)
{
	return PFN_DOWN(reg->base + reg->size);
}

/**
 * memblock_region_reserved_base_pfn - Return the lowest pfn intersecting with the reserved region
 * @reg: memblock_region structure
 */
static inline unsigned long memblock_region_reserved_base_pfn(const struct memblock_region *reg)
{
	return PFN_DOWN(reg->base);
}

/**
 * memblock_region_reserved_end_pfn - Return the end_pfn this region
 * @reg: memblock_region structure
 */
static inline unsigned long memblock_region_reserved_end_pfn(const struct memblock_region *reg)
{
	return PFN_UP(reg->base + reg->size);
}

#define for_each_memblock(memblock_type, region)					\
	for (region = memblock.memblock_type.regions;				\
	     region < (memblock.memblock_type.regions + memblock.memblock_type.cnt);	\
	     region++)


#ifdef CONFIG_ARCH_DISCARD_MEMBLOCK   // 没有定义
#define __init_memblock __meminit
#define __initdata_memblock __meminitdata    // 如果这个编译配置选项开启，内存块的代码会被放置在 .init 段，这样它就会在内核引导完毕后被释放掉。
#else
#define __init_memblock       // 就为空了，
#define __initdata_memblock
#endif

#else
static inline phys_addr_t memblock_alloc(phys_addr_t size, phys_addr_t align)
{
	return 0;
}

#endif /* CONFIG_HAVE_MEMBLOCK */

#endif /* __KERNEL__ */

#endif /* _LINUX_MEMBLOCK_H */
