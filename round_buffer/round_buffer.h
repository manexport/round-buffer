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

#ifndef __ROUND_BUFFER_H__
#define __ROUND_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ROUND_BUFFER_STATICLIB
#  define ROUND_BUFFER_EXTERN
#elif defined(_WIN32)					 
#  ifdef ROUND_BUFFER_EXPORTS
#    define ROUND_BUFFER_EXTERN __declspec(dllexport)
#  else /* !ROUND_BUFFER_EXPORTS */
#    define ROUND_BUFFER_EXTERN __declspec(dllimport)
#  endif /* !ROUND_BUFFER_EXPORTS */
#else    /* !defined(_WIN32) */
#  ifdef ROUND_BUFFER_EXPORTS
#    define ROUND_BUFFER_EXTERN __attribute__((visibility("default")))
#  else /* !ROUND_BUFFER_EXPORTS */
#    define ROUND_BUFFER_EXTERN
#  endif /* !ROUND_BUFFER_EXPORTS */
#endif   /* !defined(_WIN32) */

	typedef void *rb_t;
#ifdef _WIN64
	typedef unsigned long long ullt;
#else 
	typedef unsigned long ullt;
	
#endif
	ROUND_BUFFER_EXTERN void round_buffer_reset(rb_t hd);
	ROUND_BUFFER_EXTERN rb_t round_buffer_init(const ullt size);
	ROUND_BUFFER_EXTERN void round_buffer_destroy(rb_t hd);
	ROUND_BUFFER_EXTERN ullt round_buffer_read(rb_t hd, char* buf, long size);
	ROUND_BUFFER_EXTERN ullt round_buffer_write(rb_t hd, const char* buf, long size);
	ROUND_BUFFER_EXTERN int  round_buffer_readblock(rb_t hd, char* buf, long size);
	ROUND_BUFFER_EXTERN int  round_buffer_writeblock(rb_t hd, const char* buf, long size);
	ROUND_BUFFER_EXTERN int  round_buffer_status(rb_t hd);
	ROUND_BUFFER_EXTERN int  round_buffer_shutdown(rb_t hd);


#ifdef __cplusplus
}
#endif
#endif // __ROUND_BUFFER_H__