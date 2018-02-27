#ifndef _UAPI_LINUX_KERNEL_H
#define _UAPI_LINUX_KERNEL_H

#include <linux/sysinfo.h>

/*
 * 'kernel.h' contains some often-used function prototypes etc
 */

/*
// 
 // 选自 linux-2.6.7 内核源码 
 // filename: linux-2.6.7/include/linux/kernel.h 
//  
#define min(x,y) ({ \  
    typeof(x) _x = (x); \  
    typeof(y) _y = (y); \  
    (void) (&_x == &_y);        \  
    _x < _y ? _x : _y; })  
    

上面这个例子是选自 linux 2.6.7 内核中 include/linux/kernel.h 这个头文件，
宏定义 min 的作用是从两个相同类型的对象中选取一个最小的，它接受两个参数 x 和 y，
后面的宏替换部分就用 typeof 定义两个变量 _x 和 _y，并分别赋值为 x y，这里用 typeof
的作用就是可以让 min 接受任何类型的参数而不必局限于某一个单一类型，这有点泛型编程
的味道了，最后一个语句 _x < _y ? _x : _y; 用了一个条件运算符来返回二者之中最小的，
中间还有一句 (void) (&_x == &_y); 看起来好像是废话，其实这句话是有特殊用意的，因为
我们不能保证你在使用 min 的时候传入的两个参数都是相同的类型，这时候就需要做一个检测，
而 C 语言不支持直接 typeof(_x) == typeof(_y) 这样的操作，所以就取其地址，用指针类型
来比较，如果两个指针的类型不一致，编译器就会产生警告以达到检测的效果，至于前面的 
(void)，是因为仅表达式 &_x == &_y 本身是没有意义的，如果没有这个 (void) 编译器同
样会警告：statement with no effect [-Wunused-value]，无效的语句，如果不想看到这个
警告，那就在前面加个 (void) 忽略掉。

*/








//查下typeof的含义
// 这的目的是可以接受任意的表达式
#define __ALIGN_KERNEL(x, a)		__ALIGN_KERNEL_MASK(x, (typeof(x))(a) - 1)
#define __ALIGN_KERNEL_MASK(x, mask)	(((x) + (mask)) & ~(mask))


#endif /* _UAPI_LINUX_KERNEL_H */
