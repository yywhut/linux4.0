
//第一次进来，整个内存添加到 memroy
int __init_memblock memblock_add_range(struct memblock_type *type,
				phys_addr_t base, phys_addr_t size,
				int nid, unsigned long flags)
{

	//nid =1 ,flag =0
	bool insert = false;
	phys_addr_t obase = base;  //60000000

	/*  获取内存区域的结束位置,memblock_cap_size函数会设置size大小确保base + size不会溢出  */
	phys_addr_t end = base + memblock_cap_size(base, &size);  //a0000000
	int i, nr_new;

	if (!size)
		return 0;

	//第一次进来会先进这里,如果内存集合为空，则不需要执行插入或者合并动作
	// 第一次放memroy 跟reserve 都是先进的这里
	/* special case for empty array */
	if (type->regions[0].size == 0) {
		WARN_ON(type->cnt != 1 || type->total_size);
		type->regions[0].base = base;
		type->regions[0].size = size;
		type->regions[0].flags = flags;
		memblock_set_region_node(&type->regions[0], nid);   // 就是1
		type->total_size = size;     //40000000
		return 0;
	}

}




				
				
//第二次进来,这个保存的是kernel的大小
int __init_memblock memblock_add_range(struct memblock_type *type,
				phys_addr_t base, phys_addr_t size,
				int nid, unsigned long flags)
{

	//nid =1 ,flag =0
	bool insert = false;
	phys_addr_t obase = base;  //0x60008280

	//size 0x1125ed8

	/*	获取内存区域的结束位置,memblock_cap_size函数会设置size大小确保base + size不会溢出  */
	phys_addr_t end = base + memblock_cap_size(base, &size);  //0x6112e158

	//第一次进来会先进这里,如果内存集合为空，则不需要执行插入或者合并动作
	// 第一次放memroy 跟reserve 都是先进的这里
	/* special case for empty array */
	if (type->regions[0].size == 0) {
		WARN_ON(type->cnt != 1 || type->total_size);
		type->regions[0].base = base;//0x60008280
		type->regions[0].size = size;//size 0x1125ed8
		type->regions[0].flags = flags;
		memblock_set_region_node(&type->regions[0], nid);	// 就是1
		type->total_size = size;	 //0x1125ed8
		return 0;
	}

}

//第三次进来，保存页表的大小

int __init_memblock memblock_add_range(struct memblock_type *type,
				phys_addr_t base, phys_addr_t size,
				int nid, unsigned long flags)
{

	//nid =1 ,flag =0
	bool insert = false;
	phys_addr_t obase = base;  //0x60004000

	/*  获取内存区域的结束位置,memblock_cap_size函数会设置size大小确保base + size不会溢出  */
	phys_addr_t end = base + memblock_cap_size(base, &size);  // end = 0x60008000
	int i, nr_new;

	// size  0x4000

	if (!size)
		return 0;

repeat:
	/*
	 * The following is executed twice.  Once with %false @insert and
	 * then with %true.  The first counts the number of regions needed
	 * to accomodate the new area.  The second actually inserts them.
	 */
	 //第二次循环，从repeat标签开始经过同样的循环然后用memblock_insert_region函数把当前内存区域插入到内存块：
	base = obase;  //0x60004000
	nr_new = 0;

	// 查找新加的区域在已经存在的空间中的什么位置
	for (i = 0; i < type->cnt; i++) {
		struct memblock_region *rgn = &type->regions[i];
		phys_addr_t rbase = rgn->base;     //0x60008280
		phys_addr_t rend = rbase + rgn->size;  //0x6112e158

		if (rbase >= end)   // end = 60008000
			break;
		if (rend <= base)
			continue;
		/*
		 * @rgn overlaps.  If it separates the lower part of new
		 * area, insert that portion.
		 */
		 // 如果内存区重叠，则先插入低地址部分base-rbase，然后重新计算base地址
		if (rbase > base) {     
			nr_new++;
			if (insert)
				memblock_insert_region(type, i++, base,
						       rbase - base, nid,
						       flags);
		}
		/* area below @rend is dealt with, forget about it */
		base = min(rend, end);
	}

	/* insert the remaining portion */ //  插入内存区 base - end
	if (base < end) {  //60004000   60008000
		nr_new++;
		if (insert)//第二次进来的时候  i = 0
			memblock_insert_region(type, i, base, end - base,
					       nid, flags);   // nid = 1  flags  = 0 把60004000   60008000 这一段加入
	}

	/*
	 * If this was the first round, resize array and repeat for actual
	 * insertions; otherwise, merge and return.
	 */

	//如果出现region[]数组空间不够的情况，则通过memblock_double_array()添加新的
	//region[]空间；最后通过memblock_merge_regions()把紧挨着的内存合并了。

	/*  第一次执行的的时候insert == false  */
	if (!insert) {
		while (type->cnt + nr_new > type->max)
			//当增加的区域与原有的区域之和大于默认的最大的区域，就要把这个默认的区域变大才行
			//memblock_double_array函数加倍给定的内存区域大小，然后把insert设为true再转到repeat标签.
			if (memblock_double_array(type, obase, size) < 0)
				return -ENOMEM;
		insert = true;
		goto repeat;
	} else {
		memblock_merge_regions(type);
		return 0;
	}
}

