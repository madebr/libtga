/*
 * tgacpy.c - Make an exact copy of an image
 * Copyright (C) 2001-2002  Matthias Brueckner
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 * Current version: 0.3
 *
 */

#include <stdio.h>  /* printf(), fprintf() */
#include <malloc.h> /* malloc() */
#include <string.h> /* strlen(), strcmp(), strcpy() */

#include <unistd.h> /* getopt() */

extern char *optarg;
extern int optind, opterr, optopt;

#include <tga.h>    /* standard libtga include header */

#define EXIT_SUCCESS 0;
#define EXIT_FAILURE 1;


int verbose, flags;


void usage()
{
	printf("tgadump-0.3\n\
		USAGE: tgacpy OPTIONS\n\
		OPTIONS:\n\
		-h print this help message and exit\n\
		-i\tinput file\n\
		-o\toutput file\n\
		-v\tbe verbose\n");

	exit(0);
}

/* copy all image header fields */
void copy_header(TGA *src, TGA *dest)
{
	dest->hdr.id_len 	= src->hdr.id_len;
	dest->hdr.map_t		= src->hdr.map_t;
	dest->hdr.img_t 	= src->hdr.img_t;
	dest->hdr.map_first 	= src->hdr.map_first;
	dest->hdr.map_entry 	= src->hdr.map_entry;
	dest->hdr.map_len	= src->hdr.map_len;
	dest->hdr.x 		= src->hdr.x;
	dest->hdr.y 		= src->hdr.y;
	dest->hdr.width 	= src->hdr.width;
	dest->hdr.height 	= src->hdr.height;
	dest->hdr.depth 	= src->hdr.depth;
	dest->hdr.desc 		= src->hdr.desc;
}


int main(int argc, char *argv[])
{
       	TGA *in, *out;

        char c, *input = 0, *output = 0;
	tbyte *id, *cmap, *sl;
	tuint32 cmap_len, i;

	/* parse command line arguments */
        while((c = getopt(argc, argv, "hvi:o:")) != -1) {
                switch (c) {
                        case 'h': usage();
                        case 'v': verbose = 1; break;
			case 'i': {
				input = (char*)malloc(strlen(optarg));
				if(!input) {
					fprintf(stderr, "[error] out of memory at %s line %d\n", __FILE__, __LINE__);
					exit(1);
				}
				strcpy(input, optarg);
				break;
                        } case 'o': {
				output = (char*)malloc(strlen(optarg));
				if(!output) {
					fprintf(stderr, "[error] out of memory at %s line %d\n", __FILE__, __LINE__);
					exit(1);
				}
				strcpy(output, optarg);
				break;
			} default: {
				fprintf(stderr, "[error] invalid option\n");
				break;
			}
                }
        }

	/* check if we have an input file */
        if(!input) {
                fprintf(stderr, "[error] no input file\n");
                return EXIT_FAILURE;
        }

	/* check if we have an output file */
	if(!output) {
		fprintf(stderr, "[error] no output file\n");
		free(input);
		return EXIT_FAILURE;
	}

	/* open 'input' in read-only mode */
        in = TGAOpen(input, "r");
	if(verbose) printf("[open] name=%s, mode=%s\n", input, "rl");

	/* open 'output' in write-only mode */
	out = TGAOpen(output, "w");
        if(verbose) printf("[open] name=%s, mode=%s\n", output, "wl");

	/* read the image header of the input file */
	TGAReadHeader(in);

	/* copy all image header fields from input to output (see above) */
	copy_header(in, out);

	/* write out the output image header */
	TGAWriteHeader(out);

	id = (tbyte*)malloc(in->hdr.id_len);
	if(!id) {
		fprintf(stderr, "[error] out of memory at %s line %d\n", __FILE__, __LINE__);
		exit(1);
	}

	/* read image id from original file */
	TGAReadImageId(in, &id);

	/* write out the image id */
	TGAWriteImageId(out, id);

	/* get the total color map size in bytes */
	cmap_len = TGA_CMAP_SIZE(in);

	/* check if theres any color map data available */
	if(cmap_len > 0) {
		/* read color map data from input file*/
		if(TGAReadColorMap(in, &cmap) != cmap_len) {
			TGAError(in, TGA_READ_FAIL, "tga_read_color_map failed at %s line %d", __FILE__, __LINE__);
		} else {
			/* write color map data to output file */
			if(TGAWriteColorMap(out, cmap) != cmap_len) {
				TGAError(out, TGA_READ_FAIL, "tga_write_color_map failed at %s line %d", __FILE__, __LINE__);
			}
		}
	}


	/* image type 0 means no image data in file */
	if(in->hdr.img_t != 0) {
		if(verbose) printf("[copy] copying %d scanlines\n", in->hdr.height);

		/* TGA_SCANLINE_SIZE is width * depth / 8 */
		sl = (tbyte*)malloc(TGA_IMG_DATA_SIZE(in));
		if(!sl) TGAError(NULL, TGA_OOM, "at %s line %d", __FILE__, __LINE__);

		/* copy image */
		TGAReadScanlines(in, sl, 0, in->hdr.height);
		TGAWriteScanlines(out, sl, 0, in->hdr.height);
	}

	/* close fds, free memory */
	if(verbose) printf("[close] %s\n[close] %s\n", in->name, out->name);
        TGAClose(in);
	TGAClose(out);

        if(verbose) printf("[exit] main\n");

	return EXIT_SUCCESS;
}
