/*
 * tgadump.c - Simple example program that shows how to use the Libtga.
 * Copyright (C) 2001, Matthias Brückner
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 * CHANGES:
 * Oct 11 2001
 * 	Fixed SIGSEGV if input file was omitted
 * 	Changed 3rd arg of dump_seek() from tga_uint_8 to tga_off_t
 * Oct 10 2001
 * 	Now shows x and y orientation (from libtga-0.1.0)
 * Sep 10 2001
 *	Modified dump_read, dump_write and dump_seek to be compatible to libtga-0.0.9;
 * Sep 06 2001
 * 	Now parses command line options via getopt(3);
 * Sep 07 2001
 *	Added replacement read, write, seek, error and warning functions
 * 	for the library defaults;
 *
 * Current version: 0.0.7 (alpha)
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <error.h>

extern int errno;

#include <unistd.h>

extern char *optarg;
extern int optind, opterr, optopt;

#include <tga.h>

#define EXIT_SUCCESS 1;
#define EXIT_FAILURE 0;

char help[] = "tgadump-0.0.7\n\
Usage: tgadump -h\n\
       tgadump -i file [ -f TGA_IMAGE_INFO ] [ -f TGA_IMAGE_ID ] \n\n";

tga_uint_32 flags, verbose;


/* replacement for tga_default_read_data */
void dump_read(tga_ptr ptr, void *buf, tga_uint_32 len)
{
        if(verbose >= 3) printf("[read] size=%i\n", len);
        tga_default_read_data(ptr, buf, len);
}

/* replacement for tga_default_write_data */
void dump_write(tga_ptr ptr, void *buf, tga_uint_32 len)
{
        if(verbose >= 3) printf("[write] size=%i\n", len);
        tga_default_write_data(ptr, buf, len);
}

/* replacement for tga_default_seek */
void dump_seek(tga_ptr ptr, tga_off_t off, tga_off_t whence)
{
        if(verbose >= 2) printf("[seek] offset=%ul, from=%s\n", off, (whence == SEEK_SET) ? "start" : "end");
	fseek((FILE*)ptr->io_ptr, off, whence);
}

/* replacement for tga_default_error */
void dump_error(tga_ptr ptr, char *msg, tga_err_t err)
{
        fprintf(stderr, "[error] %s: %s, errno=%i\n", msg, tga_get_str_error(err), errno);
	if(feof((FILE*)ptr->io_ptr) != 0) fprintf(stderr, "[error] EOF\n");

        tga_free_tga(ptr);
        exit(0);
}

/* replacement for tga_default_warning */
void dump_warning(tga_ptr ptr, char *msg, tga_err_t err)
{
        fprintf(stderr, "[warning] %s (%s)\n", msg, tga_get_str_error(err));
}


/* parse arguments of the -f option */
void dump_set_flag(char *str)
{
        if(strcmp(str, "TGA_IMAGE_INFO") == 0) flags |= TGA_IMAGE_INFO;
        else if(strcmp(str, "TGA_IMAGE_ID") == 0) flags |= TGA_IMAGE_ID;
        else fprintf(stderr, "[error] invalid flag %s\n", str);

}

int main(int argc, char *argv[])
{
	tga_info_ptr info;
	tga_ptr ptr;
	FILE *in;
        char c, *bottom = "bottom", *top = "top", *left = "left", *right = "right";
	char *input = 0;

        while((c=getopt(argc, argv, "hvi:f:")) != -1) {
                switch (c) {
                        case 'h': printf("%s\n", help); exit(0);
                        case 'v': ++verbose; break;
			case 'i': {
				input = (char*)malloc(strlen(optarg));
				strcpy(input, optarg);
				break;
                        } case 'f': dump_set_flag(optarg); break;
                        default: fprintf(stderr, "[error] invalid option\n");
                }
        }

        if(!input) {
                fprintf(stderr, "[error] no input file\n");
                return EXIT_FAILURE;
        }

	info = (tga_info_ptr)malloc(sizeof(struct tga_info));
	ptr = (tga_ptr)malloc(sizeof(struct tga));

	if(!ptr || !info) {
		fprintf(stderr, "[error] memory allocation error\n");
		return EXIT_FAILURE;
	}

        if(verbose >= 1) printf("[open] name=%s, mode=%s\n", input, "r");
	
	if((in = fopen(input, "r")) == NULL) dump_error(ptr, "main(tgadump.c)", TGA_BAD_FD);

        if(verbose >= 3) {
		printf("[init] io=%x, read=%x, write=%x, seek=%x, error=%x, warning=%x\n",
                in, dump_read, dump_write, dump_seek, dump_error, dump_warning);
	}

	tga_init_ptr(ptr, in, dump_read, dump_write, dump_seek, dump_error, dump_warning);

	tga_read_tga(ptr, info, flags);

        if(flags & TGA_IMAGE_INFO) {
                if(ptr->flags & TGA_IMAGE_INFO) {
	                printf("[info] width=%lu\n", info->width);
	                printf("[info] height=%lu\n", info->height);
	                printf("[info] map type=%i\n", info->map_t);
	                printf("[info] image type=%i\n", info->img_t);
	                printf("[info] depth=%i\n", info->depth);
	                printf("[info] x=%i\n", info->x);
	                printf("[info] y=%i\n", info->y);
			printf("[info] orientation=%s-%s\n",
			(tga_get_y_orientation(info) == TGA_BOTTOM) ? "bottom" : "top",
		 	(tga_get_x_orientation(info) == TGA_LEFT) ? "left" : "right");
                 } else dump_error(ptr, "no image info available", TGA_ERROR);
        }
        if(flags & TGA_IMAGE_ID) {
                if(ptr->flags & TGA_IMAGE_ID) printf("image id: %s\n", ptr->img_id);
                else dump_warning(ptr, "no image id data available\n", TGA_ERROR);
        }

	fclose(in);
	free(info);
	tga_free_tga(ptr);

        printf("[exit] main\n");

	return EXIT_SUCCESS;
}
