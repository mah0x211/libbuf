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

int buf_init( Buf_t *b, size_t unit )
{
    memset( (void*)b, 0, sizeof( Buf_t ) );
    if( ( b->mem = malloc( unit ) ) ){
        b->unit = b->total = unit;
        b->used = 0;
        return BUF_OK;
    }
    
    return errno;
}

void buf_dispose( Buf_t *b )
{
    free( (void*)b->mem );
}

int buf_realloc( Buf_t *b, size_t bytes )
{
    void *buf = realloc( b->mem, bytes );
    
    if( buf ){
        b->mem = buf;
        b->total = bytes;
        return BUF_OK;
    }
    
    return errno;
}

int buf_shift( Buf_t *b, size_t from, size_t idx )
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
    
    b->used = idx + len;
    ((char*)b->mem)[b->used] = 0;
    
    return BUF_OK;
}

int buf_strset( Buf_t *b, const char *str )
{
    if( str )
    {
        size_t len = strlen( str );
        
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

int buf_strnset( Buf_t *b, const char *str, size_t len )
{
    if( len )
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

int buf_strcat( Buf_t *b, const char *str )
{
    if( str )
    {
        size_t len = strlen( str );
        
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

int buf_strncat( Buf_t *b, const char *str, size_t len )
{
    if( len )
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


int buf_strccat( Buf_t *b, const unsigned char c )
{
    if( buf_increase( b, b->used + 2 ) == BUF_OK ){
        ((char*)b->mem)[b->used] = c;
        b->used += 1;
        ((char*)b->mem)[b->used] = 0;
        return BUF_OK;
    }
    
    return errno;
}

int buf_strins( Buf_t *b, size_t cur, const char *str )
{
    if( str && cur < b->used )
    {
        size_t len = strlen( str );
        
        if( buf_increase( b, b->used + len + 1 ) == BUF_OK ){
            // insert string
            memcpy( b->mem + cur, str, len );
            ((char*)b->mem)[b->used] = 0;
            return BUF_OK;
        }
        
        return errno;
    }
    
    return ( errno = EINVAL );
}

// replace all target string
int buf_strsub( Buf_t *b, const char *str, const char *rep )
{
    if( str && rep )
    {
        char *match = (char*)b->mem;
        size_t len = strlen( str );
        size_t rlen = strlen( rep );
        ssize_t shift = rlen - len;
        ptrdiff_t cur;
        
        while( ( match = strstr( match, str ) ) )
        {
            cur = (uintptr_t)match - (uintptr_t)b->mem;
            
            if( shift != 0 && 
                buf_shift( b, cur + len, cur + len + shift ) != BUF_OK ){
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

int buf_strnsub( Buf_t *b, const char *str, const char *rep, size_t num )
{
    if( str && rep && num )
    {
        char *match = (char*)b->mem;
        size_t len = strlen( str );
        size_t rlen = strlen( rep );
        ssize_t shift = rlen - len;
        ptrdiff_t cur;
        
        while( ( match = strstr( match, str ) ) )
        {
            cur = (uintptr_t)match - (uintptr_t)b->mem;
            if( shift != 0 && 
                buf_shift( b, cur + len, cur + len + shift ) != BUF_OK ){
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

int buf_strsub_range( Buf_t *b, size_t from, size_t to, const char *rep )
{
    if( to < b->used && from < to )
    {
        size_t len = strlen( rep );
        size_t rlen, shift;
        
        if( to > b->used ){
            rlen = b->used - from;
        }
        else if( to == from ){
            rlen = 1;
        }
        else {
            rlen = to - from;
        }
        shift = len - rlen;
        
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

ssize_t buf_strtoll( const char *str )
{
    ssize_t bytes = 0;
    char *delim = NULL;
    
    errno = 0;
    bytes = strtoll( str, &delim, 10 );
    // invalid number format
    if( errno ){
        return errno;
    }
    else if( ( delim && *delim ) ){
        return ( errno = EINVAL );
    }
    
    return bytes;
}

