/*
 * tgadump.c - Dump out image information
 * Copyright (C) 2001-2002  Matthias Brueckner
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 * Current Version: 0.3
 *
 * Libtga functions used in this program are:
 * 	TGAOpen()
 *	TGAReadHeader()
 * 	TGAGetXOrientation()
 * 	TGAGetYOrientation()
 * 	TGAReadImageId()
 *	TGAClose()
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


tuint32 flags, verbose, do_text;


char *cmap[2] = {"not color mapped", "color mapped" };
char *imgt[7] = { "no image data included" , "uncompressed color mapped",
		  "uncompressed truecolor" , "uncompressed grayscale",
		  "compressed color mapped", "compressed truecolor",
		  "compressed grayscale" };

void usage() 
{
	printf("tgadump-0.3\n\
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
        if(strcmp(str, "TGA_IMAGE_INFO") == 0) flags |= TGA_IMAGE_INFO;
        else if(strcmp(str, "TGA_IMAGE_ID") == 0) flags |= TGA_IMAGE_ID;
        else fprintf(stderr, "[error] invalid flag %s\n", str);

}

int main(int argc, char *argv[])
{
        TGA *tga;

	FILE *in;
        char c, *input = 0;
	unsigned int *img_id;

	/* parse command line arguments */
        while((c = getopt(argc, argv, "hvti:s:")) != -1) {
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

        if(!input) {
                fprintf(stderr, "[error] no input file\n");
                return EXIT_FAILURE;
        }

	/* open read-only */
        if(verbose) printf("[open] name=%s, mode=%s\n", input, "rl");
	tga = TGAOpen(input, "r");
	
	if(!tga) exit(1);
	
	/* now read the image header */
	if(verbose) printf("[read] section=header\n");
	TGAReadHeader(tga);
	if(tga->last != 0) { 
		TGAClose(tga);
		exit(1);
	}

	/* check if image header was intented to be read */
        if(flags & TGA_IMAGE_INFO) {

		/* check if image header was actually read */
                if(tga->flags & TGA_IMAGE_INFO) {
	                printf("[info] width=%lu\n", tga->hdr.width);
	                printf("[info] height=%lu\n", tga->hdr.height);

	                printf("[info] color map type=%i\n", tga->hdr.map_t);
			if(do_text) printf("-> [text] %s\n", cmap[tga->hdr.map_t]);

	                printf("[info] image type=%i\n", tga->hdr.img_t);
			if(do_text) printf("-> [text] %s\n", (tga->hdr.img_t > 8) ?
				imgt[tga->hdr.img_t - 8] : imgt[tga->hdr.img_t]);

	                printf("[info] depth=%i\n", tga->hdr.depth);
	                printf("[info] x=%i\n", tga->hdr.x);
	                printf("[info] y=%i\n", tga->hdr.y);
			printf("[info] orientation=%s-%s\n",
			(TGAGetYOrientation(tga) == TGA_BOTTOM) ?
				"bottom" : "top",
		 	(TGAGetXOrientation(tga) == TGA_LEFT) ?
				"left" : "right");
                 } else fprintf(stderr, "[error] no image header found\n");
        }

	/* check if image id should be read */
        if(flags & TGA_IMAGE_ID) {
		if(TGAReadImageId(tga, &img_id) == 1) {
	                printf("-> image id: %s\n", img_id);
		}
        }


	/* clean-up */
        TGAClose(tga);

        if(verbose) printf("[exit] main\n");

	return EXIT_SUCCESS;
}
