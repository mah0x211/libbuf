/*
 *  libbuf.h
 *  Created by Masatoshi Teruya on 13/01/30.
 */
/*
 *  Copyright (C) 2013 Masatoshi Teruya
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

#ifndef ___LIBBUF___
#define ___LIBBUF___

#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>

// return code
#define BUF_OK           0
#define BUF_AGAIN        -1
#define BUF_TOO_LARGE    -5

// do not touch directly
typedef struct {
    // buffer
    size_t unit;
    size_t used;
    size_t total;
    void *mem;
} Buf_t;

#define buf_unit(b)     ((b)->unit)
#define buf_used(b)     ((b)->used)
#define buf_total(b)    ((b)->total)
#define buf_mem(b)      ((b)->mem)

int buf_init( Buf_t *b, size_t unit );
void buf_dispose( Buf_t *b );
void buf_reset( Buf_t *b );

int buf_realloc( Buf_t *b, size_t bytes );
#define buf_increase(b,B)({ \
    int _rc = BUF_OK; \
    size_t _bytes = (B); \
    if( _bytes > (b)->total ){ \
        size_t _mod = _bytes % (b)->unit; \
        _rc = buf_realloc( b, ( _mod ) ? _bytes - _mod + (b)->unit : _bytes ); \
    } \
    _rc; \
})
int buf_shift( Buf_t *b, size_t from, size_t idx );

int buf_strnset( Buf_t *b, const char *str, size_t len );
#define buf_strset(b,s)     buf_strnset(b,s,strlen(s))
int buf_strncat( Buf_t *b, const char *str, size_t len );
#define buf_strcat(b,s)     buf_strncat(b,s,strlen(s))
int buf_strccat( Buf_t *b, const unsigned char c );
int buf_strnins( Buf_t *b, size_t cur, const char *str, size_t len );
#define buf_strins(b,c,s)   buf_strnins(b,c,s,strlen(s))
int buf_strsub( Buf_t *b, const char *str, const char *rep );
int buf_strnsub( Buf_t *b, const char *str, const char *rep, size_t num );
int buf_strsub_range( Buf_t *b, size_t from, size_t to, const char *rep );

#define buf_strisdegit(c)   ((c) >= '0' && (c) <= '9')
// unsigned decimal string to unsigned integer
#define _buf_strudec2uint(s,e,m)({ \
    uint64_t _v = 0; \
    uint64_t _c = *(s); \
    e = (s); \
    while( buf_strisdegit(_c) ){ \
        if( _v > m || _v == m && *e > '5' ){ \
            errno = ERANGE; \
            break; \
        } \
        _v = _c - '0' + _v * 10; \
        _c = *(++e); \
    } \
    _v; \
})
#define buf_strudec2u8(s,e)   _buf_strudec2uint(s,e,25)
#define buf_strudec2u16(s,e)  _buf_strudec2uint(s,e,6553)
#define buf_strudec2u32(s,e)  _buf_strudec2uint(s,e,429496729)
#define buf_strudec2u64(s,e)  _buf_strudec2uint(s,e,1844674407370955161)

#endif

