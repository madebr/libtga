/*
 * tgaread.c
 *
 * Copying (C) 2001-2002, Matthias Brueckner
 * This file is part of the struct TGA library (libtga)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */

#include <stdio.h>
#include <sys/types.h>

#include "tga.h"


size_t
TGARead(TGA *tga, tbyte *buf, size_t size, size_t n) 
{
	size_t read = fread(buf, size, n, tga->fd);
	tga->off = ftell(tga->fd);
	return read;
}


size_t
TGAReadScanlines(TGA *tga, tbyte *buf, size_t sln, size_t n)
{	
	off_t off;
	size_t total, sln_size, read;
	
	if(tga) {
		sln_size = TGA_SCANLINE_SIZE(tga);
		off = TGA_IMG_DATA_OFF(tga) + (sln * sln_size);
		
		if(tga->off != off) TGASeek(tga, off, SEEK_SET);
		if(tga->off != off) {
			TGAError(tga, TGA_SEEK_FAIL, "at %s line %d.", __FILE__, __LINE__);
			return 0;
		}
	
		if((read = TGARead(tga, buf, sln_size, n)) != n)
			TGAError(tga, TGA_READ_FAIL, "at %s line %d.", __FILE__, __LINE__);
		
		return read;
	}
	
	return 0;
}


int
TGAReadHeader(TGA *tga)
{
	tbyte *tmp;
	
	if(tga) {
		TGASeek(tga, 0, SEEK_SET);
		if(tga->off != 0) {
			TGAError(tga, TGA_SEEK_FAIL, "at %s line %d", __FILE__, __LINE__);
			return 0;
		}

		if(!(tmp = (tbyte*)malloc(TGA_HEADER_SIZE))) {
			TGAError(tga, TGA_OOM, "at %s line %d", __FILE__, __LINE__);
			return 0;
		}
		
		bzero(tmp);
		
		if(TGARead(tga, tmp, TGA_HEADER_SIZE, 1) != 1) {
			TGAError(tga, TGA_READ_FAIL, "at %s line %d", __FILE__, __LINE__);
			free(tmp);
			return 0;
		}

		tga->hdr.id_len 	= tmp[0];
		tga->hdr.map_t 		= tmp[1];
		tga->hdr.img_t 		= tmp[2];
		tga->hdr.map_first 	= tmp[3] + tmp[4] * 256;
		tga->hdr.map_len 	= tmp[5] + tmp[6] * 256;
		tga->hdr.map_entry	= tmp[7];
		tga->hdr.x 		= tmp[8] + tmp[9] * 256;
		tga->hdr.y 		= tmp[10] + tmp[11] * 256;
		tga->hdr.width 		= tmp[12] + tmp[13] * 256;
		tga->hdr.height 	= tmp[14] + tmp[15] * 256;
		tga->hdr.depth 		= tmp[16];
		tga->hdr.desc 		= tmp[17];
	
		tga->flags |= TGA_IMAGE_INFO;

		return 1;
	}
	
	return 0;
}


int
TGAReadImageId(TGA* tga, tbyte **buf)
{
	if(tga) {
	        *buf = (tbyte*)malloc(tga->hdr.id_len);
        	if(!buf) {
			TGAError(tga, TGA_OOM, "%s at %s line %d", __FILE__, __LINE__);
			return 0;
		}

	        if(TGARead(tga, *buf, tga->hdr.id_len, 1) != 1) {
        	        TGAError(tga, TGA_READ_FAIL, "at %s line %d", __FILE__, __LINE__);
			return 0;
		}

		return 1;
	}
	
        return 0;
}


int
TGAReadColorMap(TGA *tga, tbyte **buf)
{
	tuint32 n, off, read;

	if(tga) {
		n = TGA_CMAP_SIZE(tga);
		if(n <= 0) TGAMessage(tga, TGA_ERROR, "color map length is 0 => no color map existing (at %s line %d)", __FILE__, __LINE__);

		off = TGA_CMAP_OFF(tga);
		if(tga->off != off) TGASeek(tga, off, SEEK_SET);
		if(tga->off != off) {
			TGAError(tga, TGA_SEEK_FAIL, "at %s line %d", __FILE__, __LINE__);
			return 0;
		}
	
		*buf = (tbyte*)malloc(n);
		if(!buf) { 
			TGAError(tga, TGA_OOM, "at %s line %d", __FILE__, __LINE__);
			return 0;
		}

		if((read = TGARead(tga, *buf, n, 1)) != 1) {
			TGAError(tga, TGA_READ_FAIL, "at %s line %d", __FILE__, __LINE__);
			return 0;
		}	
		
		return read;
	}
	return 0;
}
