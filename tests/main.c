#include "libbuf.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LOG(_fmt,...)({ \
    printf( "[%s:%d] " _fmt "\n", __func__, __LINE__, ##__VA_ARGS__ ); \
})

#define LOG_ERR(_fmt,...)({ \
    printf( "[%s:%d] " _fmt " | %s\n", __func__, __LINE__, ##__VA_ARGS__, \
            ( errno ) ? strerror(errno) : "OK" ); \
})

#define TEST_FN(b,r,f,...)({ \
    int rc = f( b, __VA_ARGS__ ); \
    LOG_ERR( #f "( " #__VA_ARGS__ " ) | used:%zd, total:%zd", \
             buf_used(b), buf_total(b) ); \
    assert( rc == r ); \
})

#define TEST_CMP(b,c)({ \
    int rc = strcmp( buf_mem(b), c ); \
    LOG( "compare: %s == " c " -> %s", \
         (char*)buf_mem(b), ( rc ) ? "FAIL" : "OK" ); \
    if( rc ){ exit(1); } \
})

#define TEST_FN2(b,r,f,...)({ \
    int rc = f( b, __VA_ARGS__ ); \
    LOG_ERR( #f "( " #__VA_ARGS__ " )" ); \
    assert( rc == r ); \
})

#define TEST_CMP2(s,c)({ \
    int rc = strcmp( s, c ); \
    LOG( "compare: %s == " c " -> %s", s, ( rc ) ? "FAIL" : "OK" ); \
    if( rc ){ exit(1); } \
})

#define TEST_FN3(b,f,...)({ \
    char *rc = f( b, __VA_ARGS__ ); \
    LOG_ERR( #f "( " #__VA_ARGS__ " )" ); \
    assert( rc != NULL ); \
    rc; \
})

void test( void )
{
    buf_t buf;
    buf_t *b = &buf;
    size_t unit = 10;
    
    if( buf_init( b, unit ) != BUF_OK ){
        LOG_ERR( "failed to buf_init()" );
        return;
    }
    
    TEST_FN( b, BUF_OK, buf_strset, "9876543210" );
    TEST_CMP( b, "9876543210" );
    
    TEST_FN( b, BUF_OK, buf_strnset, "9876543210", 9 );
    TEST_CMP( b, "987654321" );
    
    TEST_FN( b, BUF_OK, buf_strcat, "BBBBBBBKLL" );
    TEST_CMP( b, "987654321BBBBBBBKLL" );
    
    TEST_FN( b, BUF_OK, buf_strncat, "cat text", 5 );
    TEST_CMP( b, "987654321BBBBBBBKLLcat t" );
    
    TEST_FN( b, BUF_OK, buf_strccat, 'A' );
    TEST_CMP( b, "987654321BBBBBBBKLLcat tA" );
    
    TEST_FN( b, BUF_OK, buf_strins, 2, "X" );
    TEST_CMP( b, "98X7654321BBBBBBBKLLcat tA" );
    
    TEST_FN( b, BUF_OK, buf_strsub, "B", "" );
    TEST_CMP( b, "98X7654321KLLcat tA" );

    TEST_FN( b, BUF_OK, buf_strsub_n, "L", "B", 1 );
    TEST_CMP( b, "98X7654321KBLcat tA" );
    
    TEST_FN( b, BUF_OK, buf_strsub_range, 2, 12, "delete" );
    TEST_CMP( b, "98deleteLcat tA" );
    
    TEST_FN( b, BUF_OK, buf_strnins, 12, "YY", 1 );
    TEST_CMP( b, "98deleteLcatY tA" );

    TEST_FN( b, BUF_OK, buf_strnins, 12, "ZZ", 2 );
    TEST_CMP( b, "98deleteLcatZZY tA" );
    
    buf_dispose( b );
}


int main (int argc, const char * argv[])
{
    #pragma unused(argc,argv)
    test();
    return 0;
}

