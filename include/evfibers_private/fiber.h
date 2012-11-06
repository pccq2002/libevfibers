/********************************************************************

  Copyright 2012 Konstantin Olkhovskiy <lupus@oxnull.net>

  This file is part of libevfibers.

  libevfibers is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or any later version.

  libevfibers is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with libevfibers.  If not, see
  <http://www.gnu.org/licenses/>.

 ********************************************************************/

#ifndef _FBR_FIBER_PRIVATE_H_
#define _FBR_FIBER_PRIVATE_H_

#include <sys/queue.h>
#include <evfibers/fiber.h>
#include <evfibers_private/trace.h>
#include <coro.h>
#include <uthash.h>

#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

#define FBR_CALL_LIST_WARN 1000

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type, member) );})

#define _unused_ __attribute__((unused))

struct fbr_mem_pool {
	struct fbr_mem_pool *next, *prev;
	void *ptr;
	fbr_alloc_destructor_func destructor;
	void *destructor_context;
};

LIST_HEAD(fiber_list, fbr_fiber);
TAILQ_HEAD(fiber_tailq, fbr_fiber);

struct fbr_fiber {
	const char *name;
	fbr_fiber_func_t func;
	coro_context ctx;
	char *stack;
	size_t stack_size;
	struct fbr_call_info *call_list;
	size_t call_list_size;
	ev_io w_io;
	int w_io_expected;
	struct trace_info w_io_tinfo;
	ev_timer w_timer;
	struct trace_info w_timer_tinfo;
	int w_timer_expected;
	int reclaimed;
	struct trace_info reclaim_tinfo;
	struct fiber_list children;
	struct fbr_fiber *parent;
	struct fbr_mem_pool *pool;

	LIST_ENTRY(fbr_fiber) entries_children;
	LIST_ENTRY(fbr_fiber) entries_reclaimed;
	TAILQ_ENTRY(fbr_fiber) entries_mutex;
};

struct fbr_mutex {
	struct fbr_fiber *locked_by;
	struct fiber_tailq pending;
	TAILQ_ENTRY(fbr_mutex) entries;
};

TAILQ_HEAD(mutex_tailq, fbr_mutex);

struct fbr_stack_item {
	struct fbr_fiber *fiber;
	struct trace_info tinfo;
};

struct fbr_context_private {
	struct fbr_stack_item stack[FBR_CALL_STACK_SIZE];
	struct fbr_stack_item *sp;
	struct fbr_fiber root;
	struct fiber_list reclaimed;
	struct ev_async mutex_async;
	struct mutex_tailq mutexes;
	int backtraces_enabled;

	struct ev_loop *loop;
};

#endif
