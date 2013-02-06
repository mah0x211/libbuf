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
    else {
        return BUF_OK;
    }
    
    b->used = idx + len;
    ((char*)b->mem)[b->used] = 0;
    
    return BUF_OK;
}

int buf_strnset( Buf_t *b, const char *str, size_t len )
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

int buf_strncat( Buf_t *b, const char *str, size_t len )
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

int buf_strnins( Buf_t *b, size_t cur, const char *str, size_t len )
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


int buf_strfmt_init( BufStrFmt_t *fmt, const char *str, size_t len, uint8_t nsub )
{
    // allocate template format
    char *buf = (char*)malloc( sizeof(char*) * len + 1 );
    
    if( buf )
    {
        // allocate freeable-variable
        BufStrFmtIdx_t *idx = malloc(0);
        
        if( idx )
        {
            size_t remain = len;
            size_t newlen = 0;
            size_t num = 0;
            char *ptr = buf;
            char *head = (char*)str;
            char *tail = NULL;
            char *needle = NULL;
            uint8_t sid = 0;
            ptrdiff_t backward = 0;
            
            nsub++;
            // find placeholder
            while( ( tail = memchr( head, '$', remain ) ) )
            {
                // found escape sequence
                if( head != tail && *(tail-1) == '\\' ){
                    tail += 1;
                    len = (uintptr_t)tail - (uintptr_t)head;
                    // set next head
                    head = tail;
                    // set remain, add backward-length
                    remain -= len;
                    backward += len;
                }
                else
                {
                    // string decimal to uint8
                    needle = tail + 1;
                    errno = 0;
                    sid = (uint8_t)buf_strudec2u8( needle, needle );
                    // set remain, move head to backward
                    remain -= (uintptr_t)needle - (uintptr_t)head;
                    head -= backward;
                    
                    // found placeholder symbol
                    if( !errno && sid && sid < nsub )
                    {
                        // copy str to buffer, update buffer-length
                        len = (uintptr_t)tail - (uintptr_t)head;
                        memcpy( ptr, head, len );
                        newlen += len;
                        // move to buffer-tail
                        ptr += len;
                        
                        // realloc for formatter
                        if( ( idx = realloc( idx, sizeof( BufStrFmtIdx_t ) * 
                                             ( num + 1 ) ) ) ){
                            // save substitution-id and distance(length) and
                            // increment number of substitution index
                            idx[num].sid = sid;
                            idx[num].dist = len;
                            num++;
                        }
                        // error
                        else {
                            free( (void*)buf );
                            free( (void*)idx );
                            return errno;
                        }
                    }
                    else {
                        // copy str to buffer, update buffer-length
                        len = (uintptr_t)needle - (uintptr_t)head;
                        memcpy( ptr, head, len );
                        newlen += len;
                        // move buffer-tail
                        ptr += len;
                    }
                    // set next-head, reset backward-length
                    head = needle;
                    backward = 0;
                }
            }
            
            // format strings remains
            if( *head ){
                // move head to backward, copy str to buffer
                head -= backward;
                memcpy( (void*)ptr, (void*)head, remain );
                newlen += remain;
            }

            // set null terminator
            buf[newlen] = 0;
            
            fmt->str = buf;
            fmt->len = newlen;
            fmt->nsub = nsub;
            fmt->num = num;
            fmt->idx = idx;
            
            return BUF_OK;
        }
        free( (void*)buf );
    }
    
    return errno;
}

void buf_strfmt_dispose( BufStrFmt_t *fmt )
{
    free( (void*)fmt->str );
    free( (void*)fmt->idx );
}

char *buf_strfmt( BufStrFmt_t *fmt, uint8_t nsub, const char **subs, size_t *len )
{
    size_t blen = fmt->len;
    // allocate formatted string buf
    char *buf = (char*)malloc( sizeof( char ) * blen + 1 );
    
    *len = 0;
    if( buf )
    {
        // substitution group > 0
        if( nsub )
        {
            char *head = fmt->str;
            char *ptr = buf;
            const char *sub = NULL;
            size_t slen = 0;
            uint8_t sid = 0;
            ptrdiff_t pos = 0;
            size_t i = 0;
            
            for( i = 0; i < fmt->num; i++ )
            {
                // has sub-string
                sid = fmt->idx[i].sid;
                if( sid <= nsub && 
                    ( sub = subs[sid-1] ) && 
                    ( slen = strlen( sub ) ) )
                {
                    // expand buffer and copy sub-string
                    blen += slen;
                    if( ( buf = realloc( (void*)buf, sizeof(char) * blen + 1 ) ) ){
                        ptr = buf + pos;
                        memcpy( ptr + fmt->idx[i].dist, sub, slen );
                        // move position
                        pos += slen;
                    }
                    // error
                    else {
                        free( (void*)buf );
                        return NULL;
                    }
                }
                
                // copy format strings
                memcpy( ptr, (void*)head, fmt->idx[i].dist );
                // set next head
                head += fmt->idx[i].dist;
                // set next position
                pos += fmt->idx[i].dist;
                ptr = buf + pos;
            }
            
            // format strings remains
            if( *head ){
                memcpy( (void*)ptr, (void*)head, 
                        fmt->len - ( (uintptr_t)head - (uintptr_t)fmt->str ) );
            }
        }
        // copy all of format strings
        else {
            memcpy( (void*)buf, (void*)fmt->str, blen );
        }
        // set terminator
        buf[blen] = 0;
        *len = blen;
    }
    
    return buf;
}

