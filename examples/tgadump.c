/*
 * tgadump.c - Dump out image information
 * Copyright (C) 2001-2002  Matthias Brueckner
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Functions: TGAOpen(), TGAReadImage(), TGAClose()
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


tuint32 flags, verbose, do_text;


char *cmap[2] = {"not color mapped", "color mapped" };
char *imgt[7] = { "no image data included" , "uncompressed color mapped",
		  "uncompressed truecolor" , "uncompressed grayscale",
		  "compressed color mapped", "compressed truecolor",
		  "compressed grayscale" };

void usage() 
{
	printf("tgadump-0.4\n\
		USAGE: tgadump OPTIONS\n\
		OPTIONS:\n\
		-h\tprint this help message and exit\n\
		-i\tinput file\n
		-s\timage section to be read:\n\
		\tTGA_IMAGE_ID, TGA_IMAGE_INFO\n\
		-v\tbe verbose\n\
		-t\tgive text information if possible\n");
	
	exit(0);
}


/* parse arguments of the -s option */
void set_flag(char *str)
{
        if (strcmp(str, "TGA_IMAGE_INFO") == 0) flags |= TGA_IMAGE_INFO;
        else if (strcmp(str, "TGA_IMAGE_ID") == 0) flags |= TGA_IMAGE_ID;
        else fprintf(stderr, "[error] invalid flag %s\n", str);

}

int main(int argc, char *argv[])
{
        TGA *tga;
	TGAData *data;

	FILE *in;
        char c, *input = 0;

        while ((c = getopt(argc, argv, "hvti:s:")) != -1) {
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
                        } case 's': set_flag(optarg); break;
			  case 't': do_text = 1; break;
                        default: fprintf(stderr, "[error] invalid option\n");
                }
        }

        if (!input) {
                fprintf(stderr, "[error] no input file\n");
                return EXIT_FAILURE;
        }

	data = (TGAData*)malloc(sizeof(TGAData));
	if(!data) {
		TGA_ERROR((TGA*)NULL, TGA_OOM);
		return 0;
	}
	
        if (verbose) printf("[open] name=%s, mode=%s\n", input, "r");
	tga = TGAOpen(input, "r");
	if (!tga || tga->last != TGA_OK) {
		TGA_ERROR(tga, TGA_OPEN_FAIL);
		return 0;
	}
	

	if (verbose) printf("[read] image\n");
	data->flags = flags;
	if (!TGAReadImage(tga, data)) {
		TGA_ERROR(tga, TGA_READ_FAIL);
		return 0;
	}

        if ((flags & TGA_IMAGE_INFO) && (data->flags & TGA_IMAGE_INFO)) {
		printf("[info] width=%lu\n", tga->hdr.width);
		printf("[info] height=%lu\n", tga->hdr.height);
		
		printf("[info] color map type=%i\n", tga->hdr.map_t);
		if (do_text) printf("-> [text] %s\n", cmap[tga->hdr.map_t]);

		printf("[info] image type=%i\n", tga->hdr.img_t);
		if (do_text) printf("-> [text] %s\n", (tga->hdr.img_t > 8) ?
				    imgt[tga->hdr.img_t - 5] : imgt[tga->hdr.img_t]);

		printf("[info] depth=%i\n", tga->hdr.depth);
		printf("[info] x=%i\n", tga->hdr.x);
		printf("[info] y=%i\n", tga->hdr.y);
			printf("[info] orientation=%s-%s\n",
			       (tga->hdr.vert == TGA_BOTTOM) ?
			       "bottom" : "top",
		 	(tga->hdr.horz == TGA_LEFT) ?
			       "left" : "right");
	} else fprintf(stderr, "[error] no image header found\n");

        if ((flags & TGA_IMAGE_ID) && (data->flags & TGA_IMAGE_ID)) {
	                printf("-> image id: %s\n", data->img_id);
        }

        TGAClose(tga);
        if (verbose) printf("[exit] main\n");

	return EXIT_SUCCESS;
}
