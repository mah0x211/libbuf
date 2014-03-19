/*
 *  libbuf.h
 *
 *  Created by Masatoshi Teruya on 13/01/30.
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
} buf_t;

#define buf_unit(b)     ((b)->unit)
#define buf_used(b)     ((b)->used)
#define buf_total(b)    ((b)->total)
#define buf_mem(b)      ((b)->mem)

int buf_init( buf_t *b, size_t unit );
void buf_dispose( buf_t *b );

int buf_realloc( buf_t *b, size_t bytes );
#define buf_increase(b,B)({ \
    int _rc = BUF_OK; \
    size_t _bytes = (B); \
    if( _bytes > (b)->total ){ \
        size_t _mod = _bytes % (b)->unit; \
        _rc = buf_realloc( b, ( _mod ) ? _bytes - _mod + (b)->unit : _bytes ); \
    } \
    _rc; \
})
int buf_shift( buf_t *b, size_t from, size_t idx );


/* string manipulation API */
int buf_strnset( buf_t *b, const char *str, size_t len );
#define buf_strset(b,s)     buf_strnset(b,s,strlen(s))
int buf_strncat( buf_t *b, const char *str, size_t len );
#define buf_strcat(b,s)     buf_strncat(b,s,strlen(s))
int buf_strccat( buf_t *b, const unsigned char c );
int buf_strnins( buf_t *b, size_t cur, const char *str, size_t len );
#define buf_strins(b,c,s)   buf_strnins(b,c,s,strlen(s))
int buf_strnsub( buf_t *b, const char *str, size_t len, const char *rep,
                 size_t rlen );
#define buf_strsub(b,s,r)   buf_strnsub(b,s,strlen(s),r,strlen(r))
int buf_strnsub_n( buf_t *b, const char *str, size_t len, const char *rep, 
                  size_t rlen, size_t num );
#define buf_strsub_n(b,s,r,n)   buf_strnsub_n(b,s,strlen(s),r,strlen(r),n)
int buf_strnsub_range( buf_t *b, size_t from, size_t to, const char *rep,
                       size_t len );
#define buf_strsub_range(b,f,t,r)   buf_strnsub_range(b,f,t,r,strlen(r))


#endif

