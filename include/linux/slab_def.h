#ifndef _LINUX_SLAB_DEF_H
#define	_LINUX_SLAB_DEF_H

#include <linux/reciprocal_div.h>

/*
 * Definitions unique to the original Linux SLAB allocator.
 */

struct kmem_cache {
	struct array_cache __percpu *cpu_cache;// 一个cpu一个，表示本地cpu对象的缓冲池

/* 1) Cache tunables. Protected by slab_mutex */
	// 表示当前cpu的本地对象缓冲池array_cache为空时，从共享的缓冲池或者slabs_partial/slabs_free列表中获取
	// 对象的数目
	unsigned int batchcount;
	unsigned int limit; // 当本地对象的缓冲池空闲对象数据大于limit时就会主动释放batchcount个对象，便于内核回收
						//和销毁slab
	unsigned int shared;// 用于多核系统

	unsigned int size;// 对象的长度，这个长度要加上align对齐字节
	struct reciprocal_value reciprocal_buffer_size;
/* 2) touched by every alloc & free from the backend */

	unsigned int flags;		/* constant flags */  //  对象的分配掩码
	unsigned int num;		/* # of objs per slab */// 一个slab中最多可以有多少个对象

/* 3) cache_grow/shrink */
	/* order of pgs per slab (2^n) */
	unsigned int gfporder;   // 一个slab中占用2^gfporder 个页面

	/* force GFP flags, e.g. GFP_DMA */
	gfp_t allocflags;

	size_t colour;			/* cache colouring range */// 一个slab中有几个不同的cache line
	unsigned int colour_off;	/* colour offset */// 一个cache colour的长度，和L1 cache line 大小相同
	struct kmem_cache *freelist_cache; 
	unsigned int freelist_size; //每个对象要占用1Byte来存放freelist

	/* constructor func */
	void (*ctor)(void *obj);

/* 4) cache creation/removal */
	const char *name;  // slab 描述符的名称
	struct list_head list;
	int refcount;
	int object_size;  // 对象的实际大小
	int align;  // 对齐的长度

/* 5) statistics */
#ifdef CONFIG_DEBUG_SLAB
	unsigned long num_active;
	unsigned long num_allocations;
	unsigned long high_mark;
	unsigned long grown;
	unsigned long reaped;
	unsigned long errors;
	unsigned long max_freeable;
	unsigned long node_allocs;
	unsigned long node_frees;
	unsigned long node_overflow;
	atomic_t allochit;
	atomic_t allocmiss;
	atomic_t freehit;
	atomic_t freemiss;

	/*
	 * If debugging is enabled, then the allocator can add additional
	 * fields and/or padding to every object. size contains the total
	 * object size including these internal fields, the following two
	 * variables contain the offset to the user object and its size.
	 */
	int obj_offset;
#endif /* CONFIG_DEBUG_SLAB */
#ifdef CONFIG_MEMCG_KMEM
	struct memcg_cache_params memcg_params;
#endif

	struct kmem_cache_node *node[MAX_NUMNODES];  // slab节点，在NUMA中，每个节点有一个这个数据结构，我们这里只有一个
};

#endif	/* _LINUX_SLAB_DEF_H */
