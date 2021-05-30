#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


#define TRUE (1)
#define FALSE (0)


#define ABORT_ON_ERR (1)
#define STR_MALLOC_BUFFER (2048)
#define ERR_MSG_BUFFER_SIZE (2048)
#define ERR_MSG "ERRO: Arquivo: %s, Função: %s, Linha: %d"
#define TRY_STDERR( try )	do {					\
		if ( !(try) ) {						\
			char errmsg[ ERR_MSG_BUFFER_SIZE ];		\
			snprintf( errmsg, ERR_MSG_BUFFER_SIZE, ERR_MSG, __FILE__, __func__, __LINE__ ); \
			perror( errmsg );				\
			if ( (ABORT_ON_ERR) ) abort();			\
		}							\
	} while (0);


enum GOP_STATUS {
	NO_OPTION,
	NO_VAL,
	READ_ERR,
	OK,
};

#define ASSERT_int_equal( exp, rec ) ( f_assert_int_equal( (exp), (rec), #rec, __FILE__, __func__, __LINE__ ))

#define ASSERT_str_equal( exp, rec ) ( f_assert_str_equal( (exp), (rec), __FILE__, __func__, __LINE__ ))


void *try_malloc( size_t bytes );
char *my_strdup( char *str );
char *dupprintf( char *format, ... );
char *realloc_strcat( char *a, char *b );
void newline( void );
int strblank( char *str );

char *ltrim( char *str );
char *rtrim( char *str );
char *strtrim( char *str );

void f_assert_int_equal( int expected,
			 int received,
			 const char *exp,
			 const char *file,
			 const char *func,
			 int line );

void f_assert_str_equal( char *expected,
			 char *received,
			 const char *file,
			 const char *func,
			 int line );

int gopt_bool( const char *opt, char **argv, int *argc );
int gopt_int( const char *opt, char **argv, int *argc, int *read_val );
int gopt_str( const char *opt,  char **argv, int *argc, char **read_val );


void test_util( void );
void test_gopt( void );
char *readline( void );


#endif //__UTIL_H__
