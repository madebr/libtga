/*
 * tgadump.c - Dump out image information
 * Copyright (C) 2001-2002  Matthias Brueckner
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Functions demonstrated: TGAOpen(), TGAReadImage(), TGAClose()
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <tga.h>
#include "utils.h"


char *cmap[2] = {"not color mapped", "color mapped" };
char *imgt[7] = { "no image data included" , "uncompressed color mapped",
		  "uncompressed truecolor" , "uncompressed grayscale",
		  "compressed color mapped", "compressed truecolor",
		  "compressed grayscale" };


int main(int argc, char *argv[])
{
	if(argc != 2) {
		fprintf(stderr, "Usage: %s INPUTFILE\n", argv[0]);
		return 1;
	}

	printf("[open] name=%s, mode=%s\n", argv[1], "r");
	TGA *tga = TGAOpen(argv[1], "r");
	if (!TGA_SUCCEEDED(tga)) {
		TGA_EXAMPLE_ERROR(TGAStrError(tga));
		return 1;
	}

	TGAData data;
	bzero(&data, sizeof(data));
	data.flags = TGA_IMAGE_INFO;

	printf("[read] image\n");
	TGAReadImage(tga, &data);
	if (!TGA_SUCCEEDED(tga)) {
		TGA_EXAMPLE_ERROR(TGAStrError(tga));
		return 1;
	}

	if (data.flags & TGA_IMAGE_INFO) {
		printf("[info] width=%i\n", tga->hdr.width);
		printf("[info] height=%i\n", tga->hdr.height);

		printf("[info] color map type=%i\n", tga->hdr.map_t);
		printf("-> [text] %s\n", cmap[tga->hdr.map_t]);

		printf("[info] image type=%i\n", tga->hdr.img_t);
		printf("-> [text] %s\n", (tga->hdr.img_t > 8) ?
			imgt[tga->hdr.img_t - 5] : imgt[tga->hdr.img_t]);

		printf("[info] depth=%i\n", tga->hdr.depth);
		printf("[info] x=%i\n", tga->hdr.x);
		printf("[info] y=%i\n", tga->hdr.y);
			printf("[info] orientation=%s-%s\n",
				(tga->hdr.vert == TGA_BOTTOM) ?
				"bottom" : "top",
			(tga->hdr.horz == TGA_LEFT) ?
				"left" : "right");
	}

	TGAFreeTGAData(&data);

	printf("[close]\n");
	TGAClose(tga);
	printf("[exit] main\n");

	return 0;
}
