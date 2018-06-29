struct page {

	
	unsigned long flags;		/* Atomic flags, some possibly
					 
	union {
		void *s_mem;			/* slab first object 指向第一个obj*/  
	};
/***************************************************************************************/
	struct {
		union {
			pgoff_t index;		
			void *freelist;		
			bool pfmemalloc;				 
		};

		union {
#if defined(CONFIG_HAVE_CMPXCHG_DOUBLE) && \
	defined(CONFIG_HAVE_ALIGNED_STRUCT_PAGE)
			/* Used for cmpxchg_double in slub */
			unsigned long counters;
#else
			unsigned counters;
#endif

			struct {

				union {
					atomic_t _mapcount;  //页映射计数器， 初始化的时候被设定成-1

					struct { /* SLUB */
						unsigned inuse:16;
						unsigned objects:15;
						unsigned frozen:1;
					};
					int units;	/* SLOB */
				};
				atomic_t _count;		/* Usage count, see below. */ //页引用计数器,在初始化的时候就被置1了
			};
			unsigned int active;	/* SLAB */  // 作为下标，来指向可用的obj， active 来表示数组的下标,同时也表示活跃对象的计数，当这个计数为0的时候可以销毁这个slab
										
		};
	};
/***************************************************************************************/

	union {
		struct list_head lru;				 
		struct {		/* slub per cpu partial pages */
			struct page *next;	/* Next partial slab */
			short int pages;
			short int pobjects;
		};

		struct slab *slab_page; /* slab fields */
		struct rcu_head rcu_head;						
		struct {
			compound_page_dtor *compound_dtor;
			unsigned long compound_order;
		};

#if defined(CONFIG_TRANSPARENT_HUGEPAGE) && USE_SPLIT_PMD_PTLOCKS
		pgtable_t pmd_huge_pte; /* protected by page->ptl */
#endif
	};

	union {
		unsigned long private;				 	 
#if USE_SPLIT_PTE_PTLOCKS
#if ALLOC_SPLIT_PTLOCKS
		spinlock_t *ptl;
#else
		spinlock_t ptl;
#endif
#endif
		struct kmem_cache *slab_cache;	/* SL[AU]B: Pointer to slab *///第一个page页面的slab_cache 指向kmem_cache结构
		struct page *first_page;	/* Compound tail pages */
	};
	/***************************************************************************************/

#ifdef CONFIG_MEMCG
	struct mem_cgroup *mem_cgroup;
#endif

	/*
	 * On machines where all RAM is mapped into kernel address space,
	 * we can simply calculate the virtual address. On machines with
	 * highmem some memory is mapped into kernel virtual memory
	 * dynamically, so we need a place to store that address.
	 * Note that this field could be 16 bits on x86 ... ;)
	 *
	 * Architectures with slow multiplication can define
	 * WANT_PAGE_VIRTUAL in asm/page.h
	 */
#if defined(WANT_PAGE_VIRTUAL)
	void *virtual;			/* Kernel virtual address (NULL if
					   not kmapped, ie. highmem) */
#endif /* WANT_PAGE_VIRTUAL */

#ifdef CONFIG_KMEMCHECK
	/*
	 * kmemcheck wants to track the status of each byte in a page; this
	 * is a pointer to such a status block. NULL if not tracked.
	 */
	void *shadow;
#endif

#ifdef LAST_CPUPID_NOT_IN_PAGE_FLAGS
	int _last_cpupid;
#endif
}

