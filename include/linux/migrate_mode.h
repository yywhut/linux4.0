#ifndef MIGRATE_MODE_H_INCLUDED
#define MIGRATE_MODE_H_INCLUDED
/*
 * MIGRATE_ASYNC means never block
 * MIGRATE_SYNC_LIGHT in the current implementation means to allow blocking
 *	on most operations but not ->writepage as the potential stall time
 *	is too significant
 * MIGRATE_SYNC will block when migrating pages
 */
enum migrate_mode {
	MIGRATE_ASYNC,    // 异步模式，不会阻塞
	MIGRATE_SYNC_LIGHT,
	MIGRATE_SYNC,// 同步模式，需要睡眠等待
};

#endif		/* MIGRATE_MODE_H_INCLUDED */
