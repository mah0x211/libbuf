/*
 *  buf.c
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

#include "libbuf.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int buf_init( buf_t *b, size_t unit )
{
    memset( (void*)b, 0, sizeof( buf_t ) );
    if( ( b->mem = malloc( unit ) ) ){
        b->unit = b->total = unit;
        b->used = 0;
        return BUF_OK;
    }
    
    return errno;
}

void buf_dispose( buf_t *b )
{
    if( b->mem ){
        free( (void*)b->mem );
        b->mem = NULL;
    }
}

int buf_realloc( buf_t *b, size_t bytes )
{
    void *buf = realloc( b->mem, bytes );
    
    if( buf ){
        b->mem = buf;
        b->total = bytes;
        return BUF_OK;
    }
    
    return errno;
}

int buf_shift( buf_t *b, size_t from, size_t idx )
{
    size_t len = b->used - from;
    
    // right shift
    if( from < idx )
    {
        if( buf_increase( b, idx + len + 1 ) != BUF_OK ){
            return errno;
        }
        memmove( b->mem + idx, b->mem + from, len );
    }
    // left shift
    else if( from > idx ){
        memcpy( b->mem + idx, b->mem + from, len );
    }
    else {
        return BUF_OK;
    }
    
    b->used = idx + len;
    ((char*)b->mem)[b->used] = 0;
    
    return BUF_OK;
}

int buf_strnset( buf_t *b, const char *str, size_t len )
{
    if( str && len )
    {
        if( buf_increase( b, len + 1 ) == BUF_OK ){
            memcpy( b->mem, str, len );
            ((char*)b->mem)[len] = 0;
            b->used = len;
            return BUF_OK;
        }
        return errno;
    }
    
    return ( errno = EINVAL );
}

int buf_strncat( buf_t *b, const char *str, size_t len )
{
    if( str && len )
    {
        if( buf_increase( b, b->used + len + 1 ) == BUF_OK ){
            memcpy( b->mem + b->used, str, len );
            b->used += len;
            ((char*)b->mem)[b->used] = 0;
            return BUF_OK;
        }
        
        return errno;
    }
    
    return ( errno = EINVAL );
}


int buf_strccat( buf_t *b, const unsigned char c )
{
    if( buf_increase( b, b->used + 2 ) == BUF_OK ){
        ((unsigned char*)b->mem)[b->used] = c;
        b->used += 1;
        ((char*)b->mem)[b->used] = 0;
        return BUF_OK;
    }
    
    return errno;
}

int buf_strnins( buf_t *b, size_t cur, const char *str, size_t len )
{
    if( str && cur < b->used )
    {
        if( buf_shift( b, cur, cur + len ) == BUF_OK ){
            // insert string
            memcpy( b->mem + cur, str, len );
            return BUF_OK;
        }
        
        return errno;
    }
    
    return ( errno = EINVAL );
}

// replace all target string
int buf_strnsub( buf_t *b, const char *str, size_t len, const char *rep, 
                 size_t rlen )
{
    if( str && rep )
    {
        char *match = (char*)b->mem;
        ptrdiff_t shift = (ptrdiff_t)(rlen - len);
        ptrdiff_t cur;
        
        while( ( match = strstr( match, str ) ) )
        {
            cur = (intptr_t)match - (intptr_t)b->mem;
            
            if( shift != 0 && 
                buf_shift( b, (size_t)cur + len, 
                          (size_t)(cur + shift) + len ) != BUF_OK ){
                return errno;
            }
            else if( rlen ){
                memcpy( match, rep, rlen );
                match += rlen;
            }
        }
        
        return BUF_OK;
    }

    return ( errno = EINVAL );
}

int buf_strnsub_n( buf_t *b, const char *str, size_t len, const char *rep, 
                   size_t rlen, size_t num )
{
    if( str && rep && num )
    {
        char *match = (char*)b->mem;
        ptrdiff_t shift = (ptrdiff_t)(rlen - len);
        ptrdiff_t cur;
        
        while( ( match = strstr( match, str ) ) )
        {
            cur = (intptr_t)match - (intptr_t)b->mem;
            if( shift != 0 && 
                buf_shift( b, (size_t)cur + len, 
                          (size_t)(cur + shift) + len ) != BUF_OK ){
                return errno;
            }
            else if( rlen ){
                memcpy( match, rep, rlen );
                match += rlen;
            }
            
            if( --num == 0 ){
                break;
            }
        }
        
        return BUF_OK;
    }

    return ( errno = EINVAL );
}

int buf_strnsub_range( buf_t *b, size_t from, size_t to, const char *rep,
                      size_t len )
{
    if( to <= b->used && from < to )
    {
        size_t rlen = to - from;
        size_t shift = len - rlen;
        
        if( shift != 0 &&
            buf_shift( b, from + rlen, from + rlen + shift ) != BUF_OK ){
            return errno;
        }
        // replace string
        else if( len ){
            memcpy( b->mem + from, rep, len );
        }
        
        return BUF_OK;
    }
    
    return ( errno = EINVAL );
}


