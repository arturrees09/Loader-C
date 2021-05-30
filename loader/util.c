#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <locale.h>
#include <ctype.h>
#include <limits.h>

#include "util.h"


void *try_malloc( size_t bytes ) {
	void *new_block = malloc( bytes );
	TRY_STDERR( new_block != NULL );
	return new_block;
}



static int opt_search( const char *opt, char **argv, int *argc );	


char *dupprintf( char *format, ... ) {
	va_list valist;
	va_start( valist, format );
	int required_size =  vsnprintf( NULL, 0, format, valist ) + 1;
	va_end( valist );
	
	va_start( valist, format );
	char *dupstr = try_malloc( sizeof( char ) * required_size );
	vsnprintf( dupstr, required_size, format, valist );
	va_end( valist );
	return dupstr;
}

char *my_strdup( char *str ) {
	size_t str_len = strlen( str ) + 1; //soma 1 para o \0
	char *dupstr = try_malloc( sizeof( char ) * str_len );
	strncpy( dupstr, str, str_len );
	return dupstr;
}


char *realloc_strcat( char *a, char *b ) {
	size_t len_a = strlen( a ) * sizeof( char );
	size_t len_b = strlen( b ) * sizeof( char );
	size_t required_size = len_a + len_b + 1 * sizeof( char );
	
	a = realloc( a, required_size );
	TRY_STDERR( a != NULL );
	
	a = strncat( a, b, len_b );
	TRY_STDERR( a != NULL );

	return a;
}


void f_assert_int_equal( int expected,
			 int received,
			 const char *exp,
			 const char *file,
			 const char *func,
			 int line ) {
	if ( !( expected == received )) {
		fprintf( stderr, ERR_MSG, file, func, line );
		fprintf( stderr, "\n" );

		fprintf( stderr, "\tExpressão: %s\n", exp );
		fprintf( stderr, "\tEsperado: %d\n", expected );
		fprintf( stderr, "\tRecebido: %d\n", received );
		abort();
	}
}


void newline( void ) {
	printf( "\n" );
}


void f_assert_str_equal( char *expected,
			 char *received,
			 const char *file,
			 const char *func,
			 int line ) {
	if ( strcmp( expected, received ) != 0 ) {
		fprintf( stderr, ERR_MSG, file, func, line );
		fprintf( stderr, "\n" );

		fprintf( stderr, "\tEsperado: %s\n", expected );
		fprintf( stderr, "\tRecebido: %s\n", received );
		abort();
	}
	
}


char *readline( void ) {
	size_t buff_size = STR_MALLOC_BUFFER;
	size_t read = 0;
	
	char *str = try_malloc( buff_size );

	int c;
	while ( (c = getchar()) != '\n' && c != EOF ) {
		
		if ( read >= buff_size ) { // Realloc
			buff_size += STR_MALLOC_BUFFER;
			str = realloc( str, buff_size );
			TRY_STDERR( str != NULL );
		}
		
		str[ read ] = c;
		read++;
		
	}

	str[ read ] = '\0';

	return str;

}

int strblank( char *str ) {
	if ( *str == '\0' ) {
		return TRUE;
	}
	
	for ( char * restrict s = str; *s != '\0' ; s++ ) {
		if ( !isspace( *s ) ) {
			return FALSE;
		}
	}
	return TRUE;
}

char *ltrim( char *str ) {
	size_t len = strlen( str );
	size_t first_nom_blank = strspn( str, "\t\n\v\f\r " );

	if ( first_nom_blank > 0 ) {
		if ( first_nom_blank == len ) {
			str[ 0 ] = '\0';
		} else {
			memmove( str, str + first_nom_blank, len + 1 - first_nom_blank );
		}
	}

	return str;
}


char *rtrim( char *str ) {
	char * restrict s = str;
	for ( s = str + strlen( str ) - 1; s >= str; s-- ) {
		if ( isspace( *s )) {
			*s = '\0';
		} else {
			break;
		}
	}

	
	return str;
}


char *strtrim( char *str ) {
	str = ltrim( str );
	str = rtrim( str );
	return str;
}


static int opt_search( const char *opt, char **argv, int *argc ) {
	for ( int i = 1; i < *argc; i++ ) {
		if ( strcmp( opt, argv[ i ] ) == 0 ) {
			return i;
		}
	}
	return -1;
}



int gopt_bool( const char *opt, char **argv, int *argc ) {
	int i = opt_search( opt, argv, argc );

	if ( i != -1 ) {
		memmove( argv + i, argv + i + 1, (*argc - i) * sizeof( char* ));
		(*argc)--;
		return TRUE;		
	}
	return FALSE;
}




int gopt_int( const char *opt, char **argv, int *argc, int *read_val ) {
	int i = opt_search( opt, argv, argc );
	if ( i == -1 ) {
		return NO_OPTION;
	}
	if ( (i + 1) == *argc ) {
		return NO_VAL;
	}
		
	char *end_ptr;
	int ret = strtol( argv[ i + 1 ], &end_ptr, 10 );
	TRY_STDERR( ret != LONG_MAX && ret != LONG_MIN );
	if ( argv[ i + 1 ][0] != '\0' && *end_ptr == '\0' ) {
		*read_val = ret;
		memmove( argv + i, argv + i + 2, ((*argc - i + 1) * sizeof( char* )));
		(*argc) -= 2;
		return OK;
	}
	return READ_ERR;
}


int gopt_str( const char *opt,  char **argv, int *argc, char **read_val ) {
	int i = opt_search( opt, argv, argc );
	if ( i == -1 ) {
		return NO_OPTION;
	}
	if ( i + 1 == *argc ) {
		return NO_VAL;
	}
	
	char *cpy = my_strdup( argv[  i + 1 ] );
	memmove( argv + i, argv + i + 2, ((*argc - i + 1) * sizeof( char* )));
	(*argc) -= 2;
	*read_val = cpy;
	return OK;
}

void test_gopt( ) {
	char *opt_1[] = { "./blap", "flower.jpg" };
	char *opt_2[] = { "./blap", "flower.jpg", "-test" };
	char *opt_3[] = { "./hei", "-test", "-hehehe", "naisu.png" };

	int argc = 2;
	ASSERT_int_equal( FALSE, gopt_bool( "-no", opt_1, &argc ));
	ASSERT_int_equal( 2, argc );
	ASSERT_str_equal( "./blap", opt_1[ 0 ] );
	ASSERT_str_equal( "flower.jpg", opt_1[ 1 ] );

	argc = 3;
	ASSERT_int_equal( TRUE, gopt_bool( "-test", opt_2, &argc ));
	ASSERT_int_equal( 2, argc );
	ASSERT_str_equal( "./blap", opt_2[ 0 ] );
	ASSERT_str_equal( "flower.jpg", opt_2[ 1 ] );

	argc = 4;
	ASSERT_int_equal( TRUE, gopt_bool( "-test", opt_3, &argc ));
	ASSERT_int_equal( 3, argc );
	ASSERT_str_equal( "./hei", opt_3[ 0 ] );
	ASSERT_str_equal( "-hehehe", opt_3[ 1 ] );
	ASSERT_str_equal( "naisu.png", opt_3[ 2 ] );


	char *opt4[] = { "./blap", "hi.tga" };
	argc = 2;
	char *val = NULL;
	ASSERT_int_equal( NO_OPTION, gopt_str( "-pupu", opt4, &argc, &val ));
	ASSERT_int_equal( 2, argc );
	ASSERT_int_equal( TRUE, val == NULL );
	ASSERT_str_equal( "./blap", opt4[ 0 ] );
	ASSERT_str_equal( "hi.tga", opt4[ 1 ] );


	char *opt5[] = { "./blap", "hi.tga", "-pupu" };
	argc = 3;
	ASSERT_int_equal( NO_VAL, gopt_str( "-pupu", opt5, &argc, &val ));
	ASSERT_int_equal( 3, argc );
	ASSERT_int_equal( TRUE, val == NULL );		
	ASSERT_str_equal( "./blap", opt5[ 0 ] );
	ASSERT_str_equal( "hi.tga", opt5[ 1 ] );
	ASSERT_str_equal( "-pupu", opt5[ 2 ] );

	char *opt6[] = { "./blap", "hi.tga", "-pupu", "pipi" };
	argc = 4;
	ASSERT_int_equal( OK, gopt_str( "-pupu", opt6, &argc, &val ));
	ASSERT_int_equal( 2, argc );
	ASSERT_str_equal( "pipi", val );
	ASSERT_str_equal( "./blap", opt6[ 0 ] );
	ASSERT_str_equal( "hi.tga", opt6[ 1 ] );
	free( val );
	val = NULL;

	
	char *opt7[] = { "./blap", "-he", "-lets", "go", "hi.tga", "-pupu", "pipi" };
	argc = 7;
	ASSERT_int_equal( OK, gopt_str( "-lets", opt7, &argc, &val ));
	ASSERT_int_equal( 5, argc );
	ASSERT_str_equal( "go", val );
	ASSERT_str_equal( "./blap", opt7[ 0 ] );
	ASSERT_str_equal( "-he", opt7[ 1 ] );
	ASSERT_str_equal( "hi.tga", opt7[ 2 ] );
	ASSERT_str_equal( "-pupu", opt7[ 3 ] );
	ASSERT_str_equal( "pipi", opt7[ 4 ] );			
	free( val );
	val = NULL;	



	char *opt8[] = { "./plap", "hehe.bmp" };
	int retv = INT_MIN;	
	argc = 2;
	ASSERT_int_equal( NO_OPTION, gopt_int( "-sad", opt8, &argc, &retv ));
	ASSERT_int_equal( INT_MIN, retv );
	ASSERT_int_equal( 2, argc );
	ASSERT_str_equal( "./plap", opt8[ 0 ] );
	ASSERT_str_equal( "hehe.bmp", opt8[ 1 ] );

	char *opt9[] = { "./plap", "hehe.bmp", "-num" };
	retv = INT_MIN;	
	argc = 3;
	ASSERT_int_equal( NO_VAL, gopt_int( "-num", opt9, &argc, &retv ));
	ASSERT_int_equal( INT_MIN, retv );
	ASSERT_int_equal( 3, argc );
	ASSERT_str_equal( "./plap", opt9[ 0 ] );
	ASSERT_str_equal( "hehe.bmp", opt9[ 1 ] );
	ASSERT_str_equal( "-num", opt9[ 2 ] );	

	
	char *opt10[] = { "./plap", "hehe.bmp", "-num", "66612" };
	retv = INT_MIN;	
	argc = 4;
	ASSERT_int_equal( OK, gopt_int( "-num", opt10, &argc, &retv ));
	ASSERT_int_equal( 66612, retv );
	ASSERT_int_equal( 2, argc );
	ASSERT_str_equal( "./plap", opt10[ 0 ] );
	ASSERT_str_equal( "hehe.bmp", opt10[ 1 ] );

	
	char *opt11[] = { "./plap", "hehe.bmp", "-num", "HAHAHA" };
	retv = INT_MIN;	
	argc = 4;
	ASSERT_int_equal( READ_ERR, gopt_int( "-num", opt11, &argc, &retv ));
	ASSERT_int_equal( INT_MIN, retv );
	ASSERT_int_equal( 4, argc );
	ASSERT_str_equal( "./plap", opt11[ 0 ] );
	ASSERT_str_equal( "hehe.bmp", opt11[ 1 ] );
	ASSERT_str_equal( "-num", opt11[ 2 ] );
	ASSERT_str_equal( "HAHAHA", opt11[ 3 ] );
	
	char *opt12[] = { "./plap", "-stuff", "-2223", "hehe.bmp" };
	retv = INT_MIN;	
	argc = 4;
	ASSERT_int_equal( OK, gopt_int( "-stuff", opt12, &argc, &retv ));
	ASSERT_int_equal( -2223, retv );
	ASSERT_int_equal( 2, argc );
	ASSERT_str_equal( "./plap", opt12[ 0 ] );
	ASSERT_str_equal( "hehe.bmp", opt12[ 1 ] );


	char *opt13[] = { "./plap", "-stuff", "fdfa", "hehe.bmp" };
	retv = INT_MIN;	
	argc = 4;
	ASSERT_int_equal( READ_ERR, gopt_int( "-stuff", opt13, &argc, &retv ));
	ASSERT_int_equal( INT_MIN, retv );
	ASSERT_int_equal( 4, argc );
	ASSERT_str_equal( "./plap", opt13[ 0 ] );
	ASSERT_str_equal( "-stuff", opt13[ 1 ] );
	ASSERT_str_equal( "fdfa", opt13[ 2 ] );
	ASSERT_str_equal( "hehe.bmp", opt13[ 3 ] );	
	


	
	fprintf( stderr, "%-24s -> OK\n", __func__ );		
}




void test_util( void ) {
	ASSERT_int_equal( TRUE, strblank( "" ));

	ASSERT_int_equal( TRUE, strblank( "                 " ));
	ASSERT_int_equal( TRUE, strblank( " " ));
	ASSERT_int_equal( TRUE, strblank( "\n\n\n\n\n\t\f\r    \v \n" ));
	ASSERT_int_equal( FALSE, strblank( "     -    " ));
	ASSERT_int_equal( FALSE, strblank( "-\n\n\n\n\v\r   " ));
	ASSERT_int_equal( FALSE, strblank( "    \n\n\t\fA" ));


	char *s = my_strdup( "oie" );
	ASSERT_str_equal( "oie", s );
	free( s );

	s = my_strdup( "   oie" );
	ASSERT_str_equal( "oie", strtrim( s ));
	free( s );

	s = my_strdup( "oie     " );
	ASSERT_str_equal( "oie", strtrim( s ));
	free( s );
	
	s = my_strdup( "    oie    " );
	ASSERT_str_equal( "oie", strtrim( s ));
	free( s );
	
	s = my_strdup("\n\n\n\r\t                                    você quer uma sopinha?\t\t\t\r\v\r\v\n\n       \n" );
	ASSERT_str_equal( "você quer uma sopinha?", strtrim( s ));
	free( s );
	
	s = my_strdup( "                        " );
	ASSERT_str_equal( "", strtrim( s ));
	free( s );

	s = my_strdup( "" );
	ASSERT_str_equal( "", strtrim( s ));
	free( s );
	
	
	fprintf( stderr, "%-24s -> OK\n", __func__ );	
}
