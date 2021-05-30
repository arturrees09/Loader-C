#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include "util.h"


// SOIL é a biblioteca para leitura das imagens
#include "SOIL.h"



/** Um pixel RGB */
typedef struct {
	unsigned char r, g, b;
} RGB;


/** Uma imagem em RGB */
typedef struct {
	int width, height;
	RGB* pixels;
} Img;



//http://paulbourke.net/dataformats/asciiart/
char *ascii10 = " .:-=+*#%@";

//Removido os símbolos <, >, & porque causavam problemas de formatação
char *ascii70 = " .'`^\",:;Il!i~+_-?][}{1)(|/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW8%B@$";

char *unishade[] = {" ", "░", "▒", "▓", "█", NULL };
char *unibox[] = {" ", "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█", NULL };



static int opt_help = FALSE;
static int opt_term = FALSE;
static int opt_block_width = 3;
static int opt_block_height = 5;
static int opt_scale = 1;
static char *opt_pallete = "ascii10";
static char *opt_custom_pallete = "";

static char* PROGRAM_NAME = NULL;



	



/** Devolve pixel pela linha e coluna. */
#define PIXEL_ARRAY_XY( pixels, x, y, width ) ((pixels)[ (y) * (width) + (x) ] )

/** Devolve pixel da imagem pela linha e coluna. */
#define PIXEL_XY( img, x, y ) PIXEL_ARRAY_XY( ((img)->pixels), (x), (y), (img)->width )



// Protótipos
/** Carrega a imagem pelo nome. */
void load( char* name, Img* pic );

/** Converte a imagem para escala de cinza usando a fórmula da intensidade. */
void greyscale( Img *img );


/** Cor média do bloco de pixels. */
RGB avg_block( Img *img, int block_width, int block_height, int x, int y );


/** Reduz a imagem em blocos com a cor média. */
void reduce_blocks( Img *img, int width, int height );

/** Converte para texto multibytes (UTF-8). */
void print_img_multi( Img *img, FILE *f, char **chrs );

/** Converte para ASCII */
void print_img_ascii( Img *img, FILE *f, char *chrs );

void print_html_start( FILE *f );
void print_img( FILE *f, Img *pic, char *char_name );
void print_html_end( FILE * );

char *filename( char *s );
char *filename_no_ext( char *s );






void grayscale( Img *img ) {
	int img_size = img->width * img->height;

	for ( int i = 0; i < img_size; i++ ) {
		RGB px = img->pixels[ i ];
		int color_i = px.r * 0.3 + px.g * 0.59 + px.b * 0.11;

		img->pixels[ i ].r = color_i;
		img->pixels[ i ].g = color_i;
		img->pixels[ i ].b = color_i;		
	}
}



RGB avg_block( Img *img, int block_width, int block_height, int x, int y ) {
	int start_x = x * block_width;
	int end_x = start_x + block_width;
	int start_y = y * block_height;
	int end_y = start_y + block_height;
	// Médias
	int r, g, b, pxs;
	r = g = b = pxs = 0;
        //pxs é o total de pixeis, calcula isso porque o bloco pode estar fora da imagem
	
	for ( int ix = start_x; ix < end_x; ix++ ) {
		if ( ix >= img->width ) {
			break;
		}
		for ( int iy = start_y; iy <= end_y; iy++ ) {
			if ( iy >= img->height ) {
				break;
			}
			RGB pixel = PIXEL_XY( img, ix, iy );
			r += pixel.r;
			g += pixel.g;
			b += pixel.b;
			pxs++;
		}
	}
	if ( pxs != 0 ) {
		r /= pxs;
		g /= pxs;
		b /= pxs;
	}
	return (RGB){ r, g, b };
}



void reduce_blocks( Img *img, int width, int height ) {
	int new_width = ceil( 1.0 * img->width / width );
	int new_height = ceil( 1.0 * img->height / height );
	RGB *pixels = malloc( sizeof( RGB ) * new_width * new_height );

	for ( int ix = 0; ix < new_width; ix++ ) {
		for( int iy = 0; iy < new_height; iy++ ) {
			PIXEL_ARRAY_XY( pixels, ix, iy, new_width ) = avg_block( img, width, height, ix, iy );
		}
	}
	free( img->pixels );
	img->pixels = pixels;
	img->height = new_height;
	img->width = new_width;
}


void print_html_start( FILE *f ) {
	fprintf( f,
		 "<html>\n"
		 "  <head></head>\n"
		 "  <body style=\"background: black;\" leftmargin=0 topmargin=0>\n"
		 "  <style>\n"
		 "    pre {\n"
		 "        color: white;\n"
		 "        font-family: Courier, monospace;\n"
		 "        font-size: 8px;\n"
		 "    }\n"
		 "  </style>\n"
		 "    <pre>\n" );
}


void print_img( FILE *f, Img *pic, char *char_name ) {
	if ( strcmp( char_name, "ascii10" ) == 0 ) {
		print_img_ascii( pic, f, ascii10 );		
	} else if ( strcmp( char_name, "ascii70" ) == 0 ) {
		print_img_ascii( pic, f, ascii70 );
	} else if ( strcmp( char_name, "unishade" ) == 0 ) {
		print_img_multi( pic, f, unishade );		
	} else if ( strcmp( char_name, "unibox" ) == 0 ) {
		print_img_multi( pic, f, unibox );
	} else {
		fprintf( stderr, "Paleta \"%s\" não reconhecida.\n", char_name );
		exit( EXIT_FAILURE );
	}
}



void print_html_end( FILE *f ) {
	fprintf( f,
		 "    </pre>\n"
		 "  </body>\n"
		 "</html>\n" );
}


	

		

void load( char* name, Img* pic ) {
	int chan;
	pic->pixels = (RGB*) SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
	
	if( pic->pixels == NULL ) {
		fprintf( stderr, "SOIL loading error: '%s'\n", SOIL_last_result() );
		fprintf( stderr, "File: '%s'\n", name );
		exit( EXIT_FAILURE );
	}
	
	printf("Carregado %s: %d x %d x %d\n", name, pic->width, pic->height, chan);
}




void print_img_ascii( Img *img, FILE *f, char *chrs ) {
	int chrs_len = strlen( chrs );
	if (chrs_len != 0 ) chrs_len--;

	int size = img->width * img->height;
	for ( int i = 0; i < size; i++ ) {
		double color = img->pixels[ i ].r; //Só usa uma cor
		int chrs_idx = round( color / 255.0 * chrs_len );
		fprintf( f, "%c", chrs[ chrs_idx ] );		
		//Linha
		if ( (i + 1) % img->width == 0 ) {
			fprintf( f, "\n" );
		}
	}
}




void print_img_multi( Img *img, FILE *f, char **chrs ) {
	int chrs_len;
	for ( chrs_len = 0; chrs[ chrs_len ] != NULL; chrs_len++ ){
	}

	if (chrs_len != 0 ) chrs_len--;
	
	int size = img->width * img->height;
	for ( int i = 0; i < size; i++ ) {
		double color = img->pixels[ i ].r; //Só usa uma cor
		int chrs_idx = round( color / 255.0 * chrs_len );
		fprintf( f, "%s", chrs[ chrs_idx ] );		
		//Linha
		if ( (i + 1) % img->width == 0 ) {
			fprintf( f, "\n" );
		}
	}
}


char *filename( char *s ) {
	char *fname = strrchr( s, '/' ); //Unix
	if ( fname == NULL ) {
		fname = strrchr( s, '\\' ); //Windows
	}
	if ( fname == NULL ) {
		return s;
	}
	return fname+1;
}


char *filename_no_ext( char *s ) {
	char *ss = my_strdup( filename( s ));
	char *fname;
	while ( (fname = strrchr( ss, '.' )) != NULL ) {
		*fname = '\0';
	}
	return ss;
}



void test_filename( void ) {
	ASSERT_str_equal( "", filename( "" ));
	ASSERT_str_equal( "HAHAHA", filename( "HAHAHA" ));			
	ASSERT_str_equal( "flower.jpg", filename( "flower.jpg" ));	
	ASSERT_str_equal( "flower.jpg", filename( "~/blap/BLUPT/hehehe/flower.jpg" ));
	ASSERT_str_equal( "flower.jpg", filename( "~/blap/../.././././././BLUPT/hehehe/flower.jpg" ));

	char *s;	
	ASSERT_str_equal( "", s = filename_no_ext( filename( "" )));
	free( s );
	ASSERT_str_equal( "flower", s = filename_no_ext( "flower" ));
	free( s );
	ASSERT_str_equal( "flower", s = filename_no_ext( "flower.jpg" ));
	free( s );	
	ASSERT_str_equal( "flower", s = filename_no_ext( "flower.jpg.tar.gz" ));
	free( s );	
	ASSERT_str_equal( "flower", s = filename_no_ext( "~/blap/BLUPT/hehehe/flower.jpg" ));
	free( s );	
	ASSERT_str_equal( "flower", s = filename_no_ext( "~/blap/../.././././././BLUPT/hehehe/flower.jpg" ));
	free( s );	
}


void print_usage( void ) {
	printf( "Uso: %s [OPÇÕES] [IMAGENS]\n", PROGRAM_NAME );	
}

void print_help( void ) {
	print_usage();
	printf( "Saída em arquivos terminados em \"-out.html\"\n" );

	printf( "\n" );

	printf( "Opções:\n"
		"  -term\t\t\tSaída no terminal ao invés de em arquivos HTML.\n"
		"\n"
		"  -width INTEIRO\tTamanho dos caracteres por pixel da imagem. Padrão: %dx%d\n"
		"  -height INTEIRO\n"
		"\n"
		"  -scale INTEIRO\tFator de multiplicação do tamanho dos caracteres. Padrão: %d\n"
		"\n",
		opt_block_width, opt_block_height, opt_scale );

	printf( "  -pallete NOME\t\tEscolhe a paleta de caracteres desejada.\n" );
	printf( "\t\t\tPaletas disponíveis:\n" );
	printf( "\t\t\tascii10:  %s\n", ascii10 );
	printf( "\t\t\tascii70:  %s\n", ascii70 );
	printf( "\t\t\tunishade: " );
	for ( char ** restrict s = unishade; *s != NULL; s++ ) {
		printf( "%s", *s );
	}
	printf( "\n" );
	printf( "\t\t\tunibox:   " );
	for ( char ** restrict s = unibox; *s != NULL; s++ ) {
		printf( "%s", *s );
	}
	printf( "\n" );
	printf( "\t\t\tPadrão: %s\n", opt_pallete );
	printf( "\n" );
	printf( "  -custom \"SEQUÊNCIA\"\tPaleta de caracteres customizada.\n" );
	printf( "\n" );

	printf ("  -help\t\t\tMostra essa informação de ajuda.\n" );

		
}



int main( int argc, char **argv ) {
	setlocale( LC_ALL, "" );
	PROGRAM_NAME = argv[0];

      	//test_filename();
	//test_gopt();
	
	if ( gopt_bool( "-help", argv, &argc )) {
		print_help();
		exit( EXIT_SUCCESS );
	}

	opt_term = gopt_bool( "-term", argv, &argc );
	int retv;
	retv = gopt_int( "-width", argv, &argc, &opt_block_width );
	if ( retv == NO_VAL || retv == READ_ERR ) {
		fprintf( stderr, "Opção -width fornecida incorretamente.\n" );
	}
	
	retv = gopt_int( "-height", argv, &argc, &opt_block_height );
	if ( retv == NO_VAL || retv == READ_ERR ) {
		fprintf( stderr, "Opção -height fornecida incorretamente.\n" );
	}
	
	retv = gopt_int( "-scale", argv, &argc, &opt_scale );
	if ( retv == NO_VAL || retv == READ_ERR ) {
		fprintf( stderr, "Opção -scale fornecida incorretamente.\n" );
	}

	retv = gopt_str( "-custom", argv, &argc, &opt_custom_pallete );
	if ( retv == NO_VAL ) {
		fprintf( stderr, "Opção -custom fornecida incorretamente.\n" );
	}

	retv = gopt_str( "-pallete", argv, &argc, &opt_pallete );
	if ( retv == NO_VAL ) {
		fprintf( stderr, "Opção -pallete fornecida incorretamente.\n" );
	}


	if(argc == 1) {
		fprintf( stderr, "Nenhuma imagem fornecida\n" );
		print_usage();
		printf( "Para mais informações de ajuda: %s -help\n", PROGRAM_NAME );
		exit( EXIT_FAILURE );
	}


	for ( int i = 1; i < argc; i++ ) {
		Img *pic = &(Img){};

		load(argv[ i ], pic );

		grayscale( pic );
		reduce_blocks( pic, opt_scale * opt_block_width, opt_scale * opt_block_height );
		


		if ( opt_term ) {
			if ( *opt_custom_pallete == '\0' ) {
				print_img( stdout, pic, opt_pallete );
			} else {
				print_img_ascii( pic, stdout, opt_custom_pallete );
			}
			continue;
		}

		char *out_name = filename_no_ext( argv[ i ] );
		out_name = filename_no_ext( argv[ i ]);
		out_name = realloc_strcat( out_name, "-out.html" );


		// Exemplo: gravando um arquivo saida.html
		FILE* arq = fopen( out_name, "w"); // criar o arquivo: w
		if( arq == NULL ) {
			printf("Erro abrindo arquivo de saída\n");
			exit(1);
		}


		print_html_start( arq );

		if ( *opt_custom_pallete == '\0' ) {
			print_img( arq, pic, opt_pallete );
		} else {
			print_img_ascii( pic, arq, opt_custom_pallete );
		}
		
		print_html_end( arq );		

		printf("Escrito %s\n", out_name );
		
		free( out_name );
		fclose( arq );
		free( pic->pixels );
	}
}
