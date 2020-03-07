/*
BSD 2-Clause License

Copyright (c) 2020, manexport<manexport@yeah.net>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdlib.h>
#include <string.h>
#include "pthread.h"
#include "round_buffer.h"

#define BUFFER_SIZE_LIMIT  (1024*1024*10)
#define RB_TRUE            (1)
#define RB_FALSE           (0)



typedef struct _round_buffer
{
	int bshutdown;
	char* mpb_round_buffer;
	ullt ull_buffer_size;
	ullt buffer_write_pos;
	ullt buffer_read_pos;
	pthread_mutex_t mutex;

}round_buffer;


static int _round_buffer_lock(const rb_t hd)
{
	int ret = 0;
	round_buffer* rb = (round_buffer *)hd;
	pthread_mutex_lock(&rb->mutex);
	return ret;
}

static int _round_buffer_unlock(const rb_t hd)
{
	int ret = 0;
	round_buffer* rb = (round_buffer *)hd;
	pthread_mutex_unlock(&rb->mutex);
	return ret;
}

rb_t round_buffer_init(const ullt size)
{
	if (size >BUFFER_SIZE_LIMIT)
		return RB_FALSE;
	round_buffer* rb = (round_buffer*)malloc(sizeof(round_buffer));
	if (!rb)
		return RB_FALSE;

	rb->mpb_round_buffer = 0;
	rb->ull_buffer_size = 0;
	rb->buffer_write_pos = 0;
	rb->buffer_read_pos = 0;
	rb->bshutdown = 0;
	pthread_mutex_init(&rb->mutex, NULL);
	rb->mpb_round_buffer = (char*)malloc(size);
	if(!rb->mpb_round_buffer)
		return RB_FALSE;
	rb->ull_buffer_size = size;
	return rb;
}

void round_buffer_destroy(rb_t hd)
{
	round_buffer* rb = (round_buffer *)hd;
	pthread_mutex_destroy(&rb->mutex);
	if (rb->mpb_round_buffer)
	{
		free(rb->mpb_round_buffer);
		rb->mpb_round_buffer = 0;
	}
}
void round_buffer_reset(rb_t hd)
{
	round_buffer* rb = (round_buffer *)hd;
	_round_buffer_lock(hd);
	rb->buffer_write_pos = 0;
	rb->buffer_read_pos = 0;
	rb->bshutdown = 0;
	_round_buffer_unlock(hd);
}

static ullt get_read_pos(rb_t hd)
{
	ullt irv;
	round_buffer* rb = (round_buffer *)hd;
	_round_buffer_lock(hd);
	irv = rb->buffer_read_pos;
	_round_buffer_unlock(hd);
	return irv;
}

static ullt get_write_pos(rb_t hd)
{
	ullt irv;
	round_buffer* rb = (round_buffer *)hd;
	_round_buffer_lock(hd);
	irv = rb->buffer_write_pos;
	_round_buffer_unlock(hd);
	return irv;
}

 ullt get_used_size(rb_t hd)
{
	ullt irv;
	round_buffer* rb = (round_buffer *)hd;
	_round_buffer_lock(hd);
	irv = rb->buffer_write_pos - rb->buffer_read_pos;
	_round_buffer_unlock(hd);
	return irv;
}

ROUND_BUFFER_EXTERN int get_shutdown(rb_t hd)
{
	int ir = 0;
	round_buffer* rb = (round_buffer *)hd;
	_round_buffer_lock(hd);
	ir=rb->bshutdown;
	_round_buffer_unlock(hd);
	return ir;
}

ROUND_BUFFER_EXTERN int set_shutdown(rb_t hd)
{
	int ir = 0;
	round_buffer* rb = (round_buffer *)hd;
	_round_buffer_lock(hd);
	rb->bshutdown = 1;
	_round_buffer_unlock(hd);
	return ir;
}

static void round_write_memory(rb_t hd,const char* src_cache, ullt write_pos, const long write_size)
{
	round_buffer* rb = (round_buffer *)hd;
	if (write_pos + write_size <= rb->ull_buffer_size)
	{
		memcpy(rb->mpb_round_buffer + write_pos, src_cache, write_size);
	}
	else
	{
		memcpy(rb->mpb_round_buffer + write_pos, src_cache, (long)(rb->ull_buffer_size - write_pos));
		memcpy(rb->mpb_round_buffer, src_cache + rb->ull_buffer_size - write_pos,
			write_size - (long)(rb->ull_buffer_size - write_pos));
	}
}


static void round_read_memory(rb_t hd,char* dest_cache, ullt read_pos, const long read_size)
{
	round_buffer* rb = (round_buffer *)hd;
	if (read_pos + read_size <= rb->ull_buffer_size)
	{
		memcpy(dest_cache, rb->mpb_round_buffer + read_pos, read_size);
	}
	else
	{
		memcpy(dest_cache, rb->mpb_round_buffer + read_pos, (long)(rb->ull_buffer_size - read_pos));
		memcpy(dest_cache + rb->ull_buffer_size - read_pos,
			rb->mpb_round_buffer, read_size - (long)(rb->ull_buffer_size - read_pos));
	}
}

int round_buffer_readblock(rb_t hd, char* buf, long size )
{
	round_buffer* rb = (round_buffer *)hd;
	_round_buffer_lock(hd);

	if ( size > rb->buffer_write_pos - rb->buffer_read_pos)
	{
		_round_buffer_unlock(hd);
		return RB_FALSE;
	}
	round_read_memory(hd,buf, rb->buffer_read_pos % rb->ull_buffer_size, size );
	rb->buffer_read_pos += size;

	_round_buffer_unlock(hd);

	return RB_TRUE;
}

int round_buffer_writeblock(rb_t hd, const char* buf, long size )
{
	round_buffer* rb = (round_buffer *)hd;
	_round_buffer_lock(hd);

	if ( size > rb->ull_buffer_size - (rb->buffer_write_pos - rb->buffer_read_pos) )
	{
		_round_buffer_unlock(hd);
		return RB_FALSE;
	}
	round_write_memory(hd,buf, rb->buffer_write_pos % rb->ull_buffer_size, size );
	rb->buffer_write_pos += size;

	_round_buffer_unlock(hd);

	return RB_TRUE;
}

ullt round_buffer_read(rb_t hd, char* buf, long size)
{
	round_buffer* rb = (round_buffer *)hd;
	ullt rw_size = size;

	_round_buffer_lock(hd);

	if ( rw_size > rb->buffer_write_pos - rb->buffer_read_pos)
		rw_size = rb->buffer_write_pos - rb->buffer_read_pos;
	round_read_memory(hd,buf, rb->buffer_read_pos % rb->ull_buffer_size, (long)rw_size );
	rb->buffer_read_pos += rw_size;

	_round_buffer_unlock(hd);

	return rw_size;
}

ullt round_buffer_write(rb_t hd, const char* buf, long size )
{
	round_buffer* rb = (round_buffer *)hd;
	ullt rw_size = 0;

	_round_buffer_lock(hd);
	
	rw_size = size;
	if ( rw_size > rb->ull_buffer_size - (rb->buffer_write_pos - rb->buffer_read_pos) )
		rw_size = rb->ull_buffer_size - (rb->buffer_write_pos - rb->buffer_read_pos);
	round_write_memory(hd,buf, rb->buffer_write_pos % rb->ull_buffer_size, (long)rw_size );
	rb->buffer_write_pos += rw_size;

	_round_buffer_unlock(hd);

	return rw_size;
}
