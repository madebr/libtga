/*
 *  tgacpy - Encode / Decode TGA image
 *  Copyright (C) 2002  Matthias Brueckner
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Functions: TGAOpen(), TGAReadImage(), TGAWriteImage(), TGAClose()
 *
 */

#include <stdio.h>
#include <malloc.h> 
#include <string.h> 

#include <unistd.h> 

extern char *optarg;
extern int optind, opterr, optopt;

#include <tga.h>

#define EXIT_SUCCESS 1;
#define EXIT_FAILURE 0;


int verbose, flags, encode;


void usage()
{
	printf("tgarle-0.2\n\
		USAGE: tgarle OPTIONS\n\
		OPTIONS:\n\
		-h print this help message and exit\n\
		-i\tinput file\n\
		-o\toutput file\n\
		-e\tencode\n\
		-d\tdecode\n\
		-v\tbe verbose\n");

	exit(0);
}


void make_header(TGA *src, TGA *dest)
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
	dest->hdr.vert 	        = src->hdr.vert;
	dest->hdr.horz          = src->hdr.horz;
	dest->hdr.alpha         = src->hdr.alpha;
}


int main(int argc, char *argv[])
{
       	TGA *in, *out;
	TGAData *data;

        char c, *input = 0, *output = 0;
	tuint32 cmap_len, i;

	while ((c = getopt(argc, argv, "hvdei:o:")) != -1) {
                switch (c) {
                        case 'h': usage();
                        case 'v': verbose = 1; break;
			case 'e': encode = 1; break;
			case 'd': encode = 0; break;
			case 'i': {
				input = (char*)malloc(strlen(optarg));
				if (!input) {
					TGA_ERROR((TGA*)NULL, TGA_OOM);
					exit(0);
				}
				strcpy(input, optarg);
				break;
                        } case 'o': {
				output = (char*)malloc(strlen(optarg));
				if (!output) {
					TGA_ERROR((TGA*)NULL, TGA_OOM);
					exit(0);
				}
				strcpy(output, optarg);
				break;
			} default: {
				fprintf(stderr, "[error] invalid option\n");
				break;
			}
                }
        }

	data = (TGAData*)malloc(sizeof(TGAData));
	if(!data) {
		TGA_ERROR((TGA*)NULL, TGA_OOM);
		return 0;
	}

        if (!input) {
                fprintf(stderr, "[error] no input file\n");
                return 0;
        }

	if (!output) {
		fprintf(stderr, "[error] no output file\n");
		free(input);
		return 0;
	}

        in = TGAOpen(input, "r");
	if (verbose) printf("[open] name=%s, mode=%s\n", input, "r");

	out = TGAOpen(output, "w");
        if (verbose) printf("[open] name=%s, mode=%s\n", output, "w");
	

	data->flags = TGA_IMAGE_ID | TGA_IMAGE_DATA | TGA_RGB;
	TGAReadImage(in, data);
	if (in->last != TGA_OK) {
		TGA_ERROR(in, in->last);
		return EXIT_FAILURE;
	}

	make_header(in, out);

	if (compress) data->flags |= TGA_RLE_ENCODE;
	TGAWriteImage(out, data);
	if (out->last != TGA_OK) {
		TGA_ERROR(out, out->last);
		return EXIT_FAILURE;
	}

	if (verbose) printf("[close] %s\n[close] %s\n", in->name, out->name);
        TGAClose(in);
	TGAClose(out);

        if (verbose) printf("[exit] main\n");

	return EXIT_SUCCESS;
}
