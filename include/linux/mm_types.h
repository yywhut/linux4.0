#ifndef _LINUX_MM_TYPES_H
#define _LINUX_MM_TYPES_H

#include <linux/auxvec.h>
#include <linux/types.h>
#include <linux/threads.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/rbtree.h>
#include <linux/rwsem.h>
#include <linux/completion.h>
#include <linux/cpumask.h>
#include <linux/uprobes.h>
#include <linux/page-flags-layout.h>
#include <asm/page.h>
#include <asm/mmu.h>
find_vma
	kmem_cache_alloc_trace
	zone


#ifndef AT_VECTOR_SIZE_ARCH
#define AT_VECTOR_SIZE_ARCH 0
#endif
#define AT_VECTOR_SIZE (2*(AT_VECTOR_SIZE_ARCH + AT_VECTOR_SIZE_BASE + 1))

struct address_space;
struct mem_cgroup;

#define USE_SPLIT_PTE_PTLOCKS	(NR_CPUS >= CONFIG_SPLIT_PTLOCK_CPUS)
#define USE_SPLIT_PMD_PTLOCKS	(USE_SPLIT_PTE_PTLOCKS && \
		IS_ENABLED(CONFIG_ARCH_ENABLE_SPLIT_PMD_PTLOCK))
#define ALLOC_SPLIT_PTLOCKS	(SPINLOCK_SIZE > BITS_PER_LONG/8)

typedef void compound_page_dtor(struct page *);

/*
 * Each physical page in the system has a struct page associated with
 * it to keep track of whatever it is we are using the page for at the
 * moment. Note that we have no way to track which tasks are using
 * a page, though if it is a pagecache page, rmap structures can tell us
 * who is mapping it.
 *
 * The objects in struct page are organized in double word blocks in
 * order to allows us to use atomic double word operations on portions
 * of struct page. That is currently only used by slub but the arrangement
 * allows the use of atomic double word operations on the flags/mapping
 * and lru list pointers also.
 */


v

//atomic_t _count;          //页引用计数器
//atomic_t _mapcount;    //页映射计数器
//paging_init()可以将它们初始化为-1
//mem_init()可以将所有位码为0的页的_count设置为0
//page_count和page_mapcount可以统计其实用者个数
//_count+1为页使用者个数，_mapcount+1为页共享者个数
// _count为-1时不可被__free_page()释放
//_mapcount为0表示该页未被共享
PG_reserved

struct page {

	/***************************************************************************************/

	/* First double word block */
	//flags在page-flags.h文件夹中枚举
	//paging_init()可设置PG_reserved做flags
	//mem_init()可设置含PG_reserved的flags清除
	//#define PG_reserved 11;表示页保留，无法被__free_page()回收

	/* 在lru算法中主要用到两个标志
     * PG_active: 表示此页当前是否活跃，当放到active_lru链表时，被置位
     * PG_referenced: 表示此页最近是否被访问，每次页面访问都会被置位
     */

	unsigned long flags;		/* Atomic flags, some possibly
					 * updated asynchronously */



/*page->mapping == 0 属于交换高速缓存页
*page->mapping != 0 且第0位为1，为匿名页，指向struct anon_vma
*page->mapping != 0 且第0位为0，指向struct address_space地址空间结构变量//该页所在地址空间描述结构指针
*/
	union {
		struct address_space *mapping;	/* If low bit clear, points to 
						 * inode address_space, or NULL.
						 * If page mapped as anonymous
						 * memory, low bit is set, and
						 * it points to anon_vma object:
						 * see PAGE_MAPPING_ANON below.
						 */
		void *s_mem;			/* slab first object 指向第一个obj*/  
	};
/***************************************************************************************/
	/* Second double word */
	struct {
		union {
			// 如果是文件映射，则是文件的偏移，如果是匿名页面
			// 如果匿名页面是私有映射，就是相对于VMA的offset，如果是share的话就是把整个进程地址空间作为一个offset
			//如果此页被分配作为一个匿名页，那么它的mapping会指向一个anon_vma，而index保存此匿名页在vma中以页的偏移量(比如vma的线性地址区间是12个页的大小，此页映射到了第8页包含的线性地址上)。需
			// 注意这个index 是以byte 为单位的
			pgoff_t index;		/* Our offset within mapping. */
			void *freelist;		/* sl[aou]b first free object */ // 指向第一个freelist的地址
			bool pfmemalloc;	/* If set by the page allocator,
						 * ALLOC_NO_WATERMARKS was set
						 * and the low watermark was not
						 * met implying that the system
						 * is under some pressure. The
						 * caller should try ensure
						 * this page is only used to
						 * free other pages.
						 */
		};

		union {
#if defined(CONFIG_HAVE_CMPXCHG_DOUBLE) && \
	defined(CONFIG_HAVE_ALIGNED_STRUCT_PAGE)
			/* Used for cmpxchg_double in slub */
			unsigned long counters;
#else
			/*
			 * Keep _count separate from slub cmpxchg_double data.
			 * As the rest of the double word is protected by
			 * slab_lock but _count is not.
			 */
			unsigned counters;
#endif

			struct {

				union {
					/*
					 * Count of ptes mapped in
					 * mms, to show when page is
					 * mapped & limit reverse map
					 * searches.
					 *
					 * Used also for tail pages
					 * refcounting instead of
					 * _count. Tail pages cannot
					 * be mapped and keeping the
					 * tail page _count zero at
					 * all times guarantees
					 * get_page_unless_zero() will
					 * never succeed on tail
					 * pages.
					 */
					 //进程相关，=0表示只有父进程映射了页面
					atomic_t _mapcount;  //页映射计数器， 初始化的时候被设定成-1，每当页被映射的时候都会加加
										//page->_mapcount >=0表示 page 有多个用户映射

					struct { /* SLUB */
						unsigned inuse:16;
						unsigned objects:15;
						unsigned frozen:1;
					};
					int units;	/* SLOB */
				};
				// 很多地方会增加这个值，看书上描述
				atomic_t _count;		/* Usage count, see below. */ //页引用计数器,在初始化的时候就被置1了
			};
			unsigned int active;	/* SLAB */  // 作为下标，来指向可用的obj， active 来表示数组的下标,同时也表示活跃对象的计数，当这个计数为0的时候可以销毁这个slab
										// 摘走一个obj，active就变成1，再拿走一个就变成2
		};
	};
/***************************************************************************************/

	/* Third double word block */
	//设置PG_slab，则该页由slab分配器来管理，lru.next指向kmem_cache_t结构变量，lru.prev则指向struct slab结构
	union {
		struct list_head lru;	/* Pageout list, eg. active_list //最近、最久未使用struct slab结构指针变量
					 * protected by zone->lru_lock !
					 * Can be used as a generic list
					 * by the page owner.
					 */
		struct {		/* slub per cpu partial pages */
			struct page *next;	/* Next partial slab */
#ifdef CONFIG_64BIT
			int pages;	/* Nr of partial slabs left */
			int pobjects;	/* Approximate # of objects */
#else
			short int pages;
			short int pobjects;
#endif
		};

		struct slab *slab_page; /* slab fields */
		struct rcu_head rcu_head;	/* Used by SLAB
						 * when destroying via RCU
						 */
		/* First tail page of compound page */
		struct {
			compound_page_dtor *compound_dtor;
			unsigned long compound_order;
		};

#if defined(CONFIG_TRANSPARENT_HUGEPAGE) && USE_SPLIT_PMD_PTLOCKS
		pgtable_t pmd_huge_pte; /* protected by page->ptl */
#endif
	};

	/* Remainder is not double word aligned */

	//设置为PG_private，则private字段指向struct buffer_head
//设置为PG_compound，则指向struct page
//设置为PG_swapcache，则private为swp_entry_t的成员变量val
//如果PG_buddy 设定了，则这个值就表示伙伴系统中的order，也就是这个page的order是几
	union {
		unsigned long private;		/* Mapping-private opaque data:   //私有数据指针
					 	 * usually used for buffer_heads
						 * if PagePrivate set; used for
						 * swp_entry_t if PageSwapCache;
						 * indicates order in the buddy
						 * system if PG_buddy is set.
						 */
#if USE_SPLIT_PTE_PTLOCKS
#if ALLOC_SPLIT_PTLOCKS
		spinlock_t *ptl;
#else
		spinlock_t ptl;
#endif
#endif
		// 针对slab或者slub分配的时候，在一个slab中 如果这个page是第一个page，则用slab_cache 指向slab_cache结构
		//如果这个page不是第一个page，则用first_page 来指向第一个page结构，这里挺有意思
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
/*
 * The struct page can be forced to be double word aligned so that atomic ops
 * on double words work. The SLUB allocator can make use of such a feature.
 */
#ifdef CONFIG_HAVE_ALIGNED_STRUCT_PAGE
	__aligned(2 * sizeof(unsigned long))
#endif
;

struct page_frag {
	struct page *page;
#if (BITS_PER_LONG > 32) || (PAGE_SIZE >= 65536)
	__u32 offset;
	__u32 size;
#else
	__u16 offset;
	__u16 size;
#endif
};

typedef unsigned long __nocast vm_flags_t;

/*
 * A region containing a mapping of a non-memory backed file under NOMMU
 * conditions.  These are held in a global tree and are pinned by the VMAs that
 * map parts of them.
 */
struct vm_region {
	struct rb_node	vm_rb;		/* link in global region tree */
	vm_flags_t	vm_flags;	/* VMA vm_flags */
	unsigned long	vm_start;	/* start address of region */
	unsigned long	vm_end;		/* region initialised to here */
	unsigned long	vm_top;		/* region allocated to here */
	unsigned long	vm_pgoff;	/* the offset in vm_file corresponding to vm_start */
	struct file	*vm_file;	/* the backing file or NULL */

	int		vm_usage;	/* region usage count (access under nommu_region_sem) */
	bool		vm_icache_flushed : 1; /* true if the icache has been flushed for
						* this region */
};

/*
 * This struct defines a memory VMM memory area. There is one of these
 * per VM-area/task.  A VM area is any part of the process virtual memory
 * space that has a special rule for the page-fault handlers (ie a shared
 * library, the executable area etc).
 */


/* 描述线性区结构 
 * 内核尽力把新分配的线性区与紧邻的现有线性区进程合并。如果两个相邻的线性区访问权限相匹配，就能把它们合并在一起。
 * 每个线性区都有一组连续号码的页(非页框)所组成，而页只有在被访问的时候系统会产生缺页异常，在异常中分配页框
 */

anon_vma
// 进程相关的，与内核的有区别
struct vm_area_struct {
	/* The first cache line has the info for VMA tree walking. */

	/* 线性区内的第一个线性地址 */
	unsigned long vm_start;		/* Our start address within vm_mm. */

	/* 线性区之外的第一个线性地址 */
	unsigned long vm_end;		/* The first byte after our end address
					   within vm_mm. */

	  /* 整个链表会按地址大小递增排序 */
	/* linked list of VM areas per task, sorted by address */
	struct vm_area_struct *vm_next, *vm_prev;

	struct rb_node vm_rb;  // 挂入红黑树

	/*
	 * Largest free memory gap in bytes to the left of this VMA.
	 * Either between this VMA and vma->vm_prev, or between one of the
	 * VMAs below us in the VMA rbtree and its ->vm_prev. This helps
	 * get_unmapped_area find a free area of the right size.
	 */

	/* 此vma的子树中最大的空闲内存块大小(bytes) */
	unsigned long rb_subtree_gap;

	/* Second cache line starts here. */

	/* 指向所属的内存描述符 */
	struct mm_struct *vm_mm;	/* The address space we belong to. */

	/* 页表项标志的初值，当增加一个页时，内核根据这个字段的值设置相应页表项中的标志 */
    /* 页表中的User/Supervisor标志应当总被置1 */
	pgprot_t vm_page_prot;		/* Access permissions of this VMA. */// 访问权限

	/* 线性区标志
     * 读写可执行权限会复制到页表项中，由分页单元去检查这几个权限
     */
	unsigned long vm_flags;		/* Flags, see mm.h. */

	/*
	 * For areas with an address space and backing store,
	 * linkage into the address_space->i_mmap interval tree.
	 */

	/* 链接到反向映射所使用的数据结构，用于文件映射的线性区，主要用于文件页的反向映射 */
	struct {
		struct rb_node rb;
		unsigned long rb_subtree_last;
	} shared;

	/*
	 * A file's MAP_PRIVATE vma can be in both i_mmap tree and anon_vma
	 * list, after a COW of one of the file pages.	A MAP_SHARED vma
	 * can only be in the i_mmap tree.  An anonymous MAP_PRIVATE, stack
	 * or brk vma (with NULL file) can only be in an anon_vma list.
	 */

	/* 
     * 指向匿名线性区链表头的指针，这个链表会将此mm_struct中的所有匿名线性区链接起来
     * 匿名的MAP_PRIVATE、堆和栈的vma都会存在于这个anon_vma_chain链表中
     * 如果mm_struct的anon_vma为空，那么其anon_vma_chain也一定为空
     */

	// 下面两个管理反向映射
	struct list_head anon_vma_chain; /* Serialized by mmap_sem &
					  * page_table_lock */
		
		/* 指向anon_vma数据结构的指针，对于匿名线性区，此为重要结构 */
	struct anon_vma *anon_vma;	/* Serialized by page_table_lock */

	

	/* Function pointers to deal with this struct. */
	//用于当虚存页面不在物理内存而引起的“缺页异常”时所应该调用的函数
	// 经常用于文件映射，里面有响应的操作函数
	const struct vm_operations_struct *vm_ops;

	/* Information about our backing store: */
	//这里的单位要注意，是page为单位
	// 分两种情况，用于文件映射，是相对于文件起始的地方的偏移，对于匿名页面，通常也是有含义的，一般是0或者是vm_addr/page_size
	//对于匿名页面一种是share anonymous mapping，起点位置是0。另外一种是private anonymous mapping，
	//起点位置是mapping的虚拟地址（除以page size）
	unsigned long vm_pgoff;		/* Offset (within vm_file) in PAGE_SIZE
					   units, *not* PAGE_CACHE_SIZE */

	 /* 指向映射文件的文件对象，也可能指向建立shmem共享内存中返回的struct file，如果是匿名线性区，此值为NULL或者一个匿名文件(这个匿名文件跟swap有关?待看) */				   
	struct file * vm_file;		/* File we map to (can be NULL). */
	void * vm_private_data;		/* was vm_pte (shared mem) */

#ifndef CONFIG_MMU
	struct vm_region *vm_region;	/* NOMMU mapping region */
#endif
#ifdef CONFIG_NUMA
	struct mempolicy *vm_policy;	/* NUMA policy for the VMA */
#endif
};

struct core_thread {
	struct task_struct *task;
	struct core_thread *next;
};

struct core_state {
	atomic_t nr_threads;
	struct core_thread dumper;
	struct completion startup;
};

enum {
	MM_FILEPAGES,
	MM_ANONPAGES,
	MM_SWAPENTS,
	NR_MM_COUNTERS
};

#if USE_SPLIT_PTE_PTLOCKS && defined(CONFIG_MMU)
#define SPLIT_RSS_COUNTING
/* per-thread cached information, */
struct task_rss_stat {
	int events;	/* for synchronization threshold */
	int count[NR_MM_COUNTERS];
};
#endif /* USE_SPLIT_PTE_PTLOCKS */

struct mm_rss_stat {
	atomic_long_t count[NR_MM_COUNTERS];
};

struct kioctx_table;



/*
该结构中get_unmapped_area函数用于在虚拟空间中获得未被映射的空间，
mmap_base是上文中MMAP区域的基地址，task_size是进程地址空间的大小，
start_code和end_code是进程代码段的起止地址，start_data和end_data是进程数据段
的起止地址，start_brk和堆空间的起始地址，start_stack是栈空间的起始地址，
brk表示堆区域当前的结束地址（为什么栈空间没有当前的结束地址呢？想想esp寄存器...），
arg_start和arg_end表示进程参数列表，env_start和env_end表示环境变量，
这两个区域都位于栈中最高的区域。
*/
/* 所有的内存描述符存放在一个双向链表中，链表中第一个元素是init_mm，它是初始化阶段进程0的内存描述符 */

//描述进程的
struct mm_struct {
	/* 指向线性区对象的链表头，链表是经过排序的，按线性地址升序排列，里面包括了匿名映射线性区和文件映射线性区 */
	struct vm_area_struct *mmap;		/* list of VMAs */  //单链表，进程中所有的 都链接进来，注意是以vma的起始地址，按照递增的方式插入进来的

	/* 指向线性区对象的红黑树的根，一个内存描述符的线性区会用两种方法组织，链表和红黑树，红黑树适合内存描述符有非常多线性区的情况 */
	struct rb_root mm_rb;

	u32 vmacache_seqnum;                   /* per-thread vmacache */


/* 在进程地址空间中找一个可以使用的线性地址空间，查找一个空闲的地址区间
     * len: 指定区间的长度
     * 返回新区间的起始地址
     */

#ifdef CONFIG_MMU
	unsigned long (*get_unmapped_area) (struct file *filp,
				unsigned long addr, unsigned long len,
				unsigned long pgoff, unsigned long flags);
#endif

	/* 标识第一个分配的匿名线性区或文件内存映射的线性地址 */
	unsigned long mmap_base;		/* base of mmap area */
	unsigned long mmap_legacy_base;         /* base of mmap area in bottom-up allocations */
	unsigned long task_size;		/* size of task vm space */

	/* 所有vma中最大的结束地址 */
	unsigned long highest_vm_end;		/* highest vma end address */

	/* 指向页全局目录 */
	pgd_t * pgd;

	/* 次使用计数器，存放了共享此mm_struct的轻量级进程的个数，但所有的mm_users在mm_count的计算中只算作1 */
	atomic_t mm_users;			/* How many users with user space? */ /* 初始为1 */ 


	/* 主使用计数器，当mm_count递减时，系统会检查是否为0，为0则解除这个mm_struct *//* 初始为1 */ 
	atomic_t mm_count;			/* How many references to "struct mm_struct" (users count as 1) */
	atomic_long_t nr_ptes;			/* PTE page table pages */
	atomic_long_t nr_pmds;			/* PMD page table pages */

	/* 线性区的个数，默认最多是65535个，系统管理员可以通过写/proc/sys/vm/max_map_count文件修改这个值 */
	int map_count;				/* number of VMAs */

	/* 线性区的自旋锁和页表的自旋锁 */
	spinlock_t page_table_lock;		/* Protects page tables and some counters */

	/* 线性区的自旋锁和页表的自旋锁 */
	struct rw_semaphore mmap_sem;

	struct list_head mmlist;		/* List of maybe swapped mm's.	These are globally strung
						 * together off init_mm.mmlist, and are protected
						 * by mmlist_lock
						 */

	 /* 进程所拥有的最大页框数 */
	unsigned long hiwater_rss;	/* High-watermark of RSS usage */
	/* 进程线性区中的最大页数 */
	unsigned long hiwater_vm;	/* High-water virtual memory usage */

	/* 进程地址空间的大小(页框数) */
	unsigned long total_vm;		/* Total pages mapped */

	 /* 锁住而不能换出的页的数量 */
	unsigned long locked_vm;	/* Pages that have PG_mlocked set */
	unsigned long pinned_vm;	/* Refcount permanently increased */

	/* 共享文件内存映射中的页数量 */
	unsigned long shared_vm;	/* Shared pages (files) */

	/* 可执行内存映射中的页数量 */
	unsigned long exec_vm;		/* VM_EXEC & ~VM_WRITE */


	/* 用户态堆栈的页数量 */
	unsigned long stack_vm;		/* VM_GROWSUP/DOWN */
	unsigned long def_flags;
	unsigned long start_code, end_code, start_data, end_data;//代码段从start_code到end_code；数据段从start_data到end_data。
	unsigned long start_brk, brk, start_stack;//堆从start_brk开始，brk表示堆的结束地址；栈从start_stack开始。
	unsigned long arg_start, arg_end, env_start, env_end;//表示参数列表和环境变量的起始和结束地址，这两个区域都位于栈的最高区域。

	unsigned long saved_auxv[AT_VECTOR_SIZE]; /* for /proc/PID/auxv */

	/*
	 * Special counters, in some configurations protected by the
	 * page_table_lock, in other configurations by being atomic.
	 */
	struct mm_rss_stat rss_stat;

	struct linux_binfmt *binfmt;

	cpumask_var_t cpu_vm_mask_var;

	/* Architecture-specific MM context */
	mm_context_t context;

	unsigned long flags; /* Must use atomic bitops to access the bits */

	struct core_state *core_state; /* coredumping support */
#ifdef CONFIG_AIO
	spinlock_t			ioctx_lock;
	struct kioctx_table __rcu	*ioctx_table;
#endif
#ifdef CONFIG_MEMCG
	/*
	 * "owner" points to a task that is regarded as the canonical
	 * user/owner of this mm. All of the following must be true in
	 * order for it to be changed:
	 *
	 * current == mm->owner
	 * current->mm != mm
	 * new_owner->mm == mm
	 * new_owner->alloc_lock is held
	 */
	  /* 所属进程 */
	struct task_struct __rcu *owner;
#endif

	/* store ref to file /proc/<pid>/exe symlink points to */

	/* 代码段中映射的可执行文件的file */

	struct file *exe_file;
#ifdef CONFIG_MMU_NOTIFIER
	struct mmu_notifier_mm *mmu_notifier_mm;
#endif
#if defined(CONFIG_TRANSPARENT_HUGEPAGE) && !USE_SPLIT_PMD_PTLOCKS
	pgtable_t pmd_huge_pte; /* protected by page_table_lock */
#endif
#ifdef CONFIG_CPUMASK_OFFSTACK
	struct cpumask cpumask_allocation;
#endif
#ifdef CONFIG_NUMA_BALANCING
	/*
	 * numa_next_scan is the next time that the PTEs will be marked
	 * pte_numa. NUMA hinting faults will gather statistics and migrate
	 * pages to new nodes if necessary.
	 */
	unsigned long numa_next_scan;

	/* Restart point for scanning and setting pte_numa */
	unsigned long numa_scan_offset;

	/* numa_scan_seq prevents two threads setting pte_numa */
	int numa_scan_seq;
#endif
#if defined(CONFIG_NUMA_BALANCING) || defined(CONFIG_COMPACTION)
	/*
	 * An operation with batched TLB flushing is going on. Anything that
	 * can move process memory needs to flush the TLB when moving a
	 * PROT_NONE or PROT_NUMA mapped page.
	 */
	bool tlb_flush_pending;
#endif
	struct uprobes_state uprobes_state;
#ifdef CONFIG_X86_INTEL_MPX
	/* address of the bounds directory */
	void __user *bd_addr;
#endif
};

static inline void mm_init_cpumask(struct mm_struct *mm)
{
#ifdef CONFIG_CPUMASK_OFFSTACK
	mm->cpu_vm_mask_var = &mm->cpumask_allocation;
#endif
	cpumask_clear(mm->cpu_vm_mask_var);
}

/* Future-safe accessor for struct mm_struct's cpu_vm_mask. */
static inline cpumask_t *mm_cpumask(struct mm_struct *mm)
{
	return mm->cpu_vm_mask_var;
}

#if defined(CONFIG_NUMA_BALANCING) || defined(CONFIG_COMPACTION)
/*
 * Memory barriers to keep this state in sync are graciously provided by
 * the page table locks, outside of which no page table modifications happen.
 * The barriers below prevent the compiler from re-ordering the instructions
 * around the memory barriers that are already present in the code.
 */
static inline bool mm_tlb_flush_pending(struct mm_struct *mm)
{
	barrier();
	return mm->tlb_flush_pending;
}
static inline void set_tlb_flush_pending(struct mm_struct *mm)
{
	mm->tlb_flush_pending = true;

	/*
	 * Guarantee that the tlb_flush_pending store does not leak into the
	 * critical section updating the page tables
	 */
	smp_mb__before_spinlock();
}
/* Clearing is done after a TLB flush, which also provides a barrier. */
static inline void clear_tlb_flush_pending(struct mm_struct *mm)
{
	barrier();
	mm->tlb_flush_pending = false;
}
#else
static inline bool mm_tlb_flush_pending(struct mm_struct *mm)
{
	return false;
}
static inline void set_tlb_flush_pending(struct mm_struct *mm)
{
}
static inline void clear_tlb_flush_pending(struct mm_struct *mm)
{
}
#endif

struct vm_special_mapping
{
	const char *name;
	struct page **pages;
};

enum tlb_flush_reason {
	TLB_FLUSH_ON_TASK_SWITCH,
	TLB_REMOTE_SHOOTDOWN,
	TLB_LOCAL_SHOOTDOWN,
	TLB_LOCAL_MM_SHOOTDOWN,
	NR_TLB_FLUSH_REASONS,
};

 /*
  * A swap entry has to fit into a "unsigned long", as the entry is hidden
  * in the "index" field of the swapper address space.
  */
typedef struct {
	unsigned long val;
} swp_entry_t;

#endif /* _LINUX_MM_TYPES_H */
