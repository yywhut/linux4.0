#ifndef _UAPI_LINUX_SCHED_H
#define _UAPI_LINUX_SCHED_H

/*
 * cloning flags:
 */
#define CSIGNAL		0x000000ff	/* signal mask to be sent at exit */
#define CLONE_VM	0x00000100	/* set if VM shared between processes 父子进程共享内存空间*/
#define CLONE_FS	0x00000200	/* set if fs info shared between processes 共享相同的文件系统*/
#define CLONE_FILES	0x00000400	/* set if open files shared between processes 共享文件描述符*/
#define CLONE_SIGHAND	0x00000800	/* set if signal handlers and blocked signals shared 共享相同的信号处理*/
#define CLONE_PTRACE	0x00002000	/* set if we want to let tracing continue on the child too 父进程trace，子进程也trace*/
#define CLONE_VFORK	0x00004000	/* set if the parent wants the child to wake it up on mm_release 父进程被挂起，直到子进程释放了虚拟内存资源 */
#define CLONE_PARENT	0x00008000	/* set if we want to have the same parent as the cloner 新进程和创建他的进程是兄弟关系，而不是父子关系 */
#define CLONE_THREAD	0x00010000	/* Same thread group? 父子进程共享相同的线程群*/
#define CLONE_NEWNS	0x00020000	/* New mount namespace group */
#define CLONE_SYSVSEM	0x00040000	/* share system V SEM_UNDO semantics */
#define CLONE_SETTLS	0x00080000	/* create a new TLS for the child */
#define CLONE_PARENT_SETTID	0x00100000	/* set the TID in the parent */
#define CLONE_CHILD_CLEARTID	0x00200000	/* clear the TID in the child */
#define CLONE_DETACHED		0x00400000	/* Unused, ignored */
#define CLONE_UNTRACED		0x00800000	/* set if the tracing process can't force CLONE_PTRACE on this clone */
#define CLONE_CHILD_SETTID	0x01000000	/* set the TID in the child */
/* 0x02000000 was previously the unused CLONE_STOPPED (Start in stopped state)
   and is now available for re-use. */
#define CLONE_NEWUTS		0x04000000	/* New utsname namespace */
#define CLONE_NEWIPC		0x08000000	/* New ipc namespace */
#define CLONE_NEWUSER		0x10000000	/* New user namespace */
#define CLONE_NEWPID		0x20000000	/* New pid namespace */
#define CLONE_NEWNET		0x40000000	/* New network namespace */
#define CLONE_IO		0x80000000	/* Clone io context */

/*
 * Scheduling policies
 */
#define SCHED_NORMAL		0
#define SCHED_FIFO		1
#define SCHED_RR		2
#define SCHED_BATCH		3
/* SCHED_ISO: reserved but not implemented yet */
#define SCHED_IDLE		5
#define SCHED_DEADLINE		6

/* Can be ORed in to make sure the process is reverted back to SCHED_NORMAL on fork */
#define SCHED_RESET_ON_FORK     0x40000000

/*
 * For the sched_{set,get}attr() calls
 */
#define SCHED_FLAG_RESET_ON_FORK	0x01

#endif /* _UAPI_LINUX_SCHED_H */
