/*
 * tgawrite.c
 *
 * Copyright (C) 2001-2002, Matthias Brueckner
 * This file is part of the TGA library (libtga).
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
TGAWrite(TGA *tga, const tbyte *buf, size_t size, size_t n)
{
	size_t wrote = fwrite(buf, size, n, tga->fd);
	tga->off = ftell(tga->fd);
	return wrote;
}


size_t
TGAWriteScanlines(TGA *tga, const tbyte *buf, size_t sln, size_t n)
{
	off_t off, wrote, sln_size;

	if(tga) {
		if(!buf) {
			TGAError(tga, TGA_BAD_PTR, "at %s line %d", __FILE__, __LINE__);
			n = 0;
		}

		sln_size = TGA_SCANLINE_SIZE(tga);
		off = TGA_IMG_DATA_OFF(tga) + (sln * sln_size);

		if(tga->off != off) TGASeek(tga, off, SEEK_SET);
		if(tga->off != off) {
			TGAError(tga, TGA_SEEK_FAIL, "at %s line %d", __FILE__, __LINE__);
			return 0;
		}
			
		if((wrote = TGAWrite(tga, buf, sln_size, n)) != n)
			TGAError(tga, TGA_WRITE_FAIL, "at %s line %d.", __FILE__, __LINE__);

	        return wrote;
	}
	return 0;
}


int
TGAWriteHeader(TGA *tga)
{
	tbyte *tmp;

	if(tga) {
		if(TGASeek(tga, 0, SEEK_SET) != 0) {
			TGAError(tga, TGA_SEEK_FAIL, "at %s line %d", __FILE__, __LINE__);
			return 0;
		}

		if(!(tmp = (tbyte*)malloc(18))) {
			TGAError(tga, TGA_OOM, "at %s line %d", __FILE__, __LINE__);
			return 0;
		}

		tmp[0] = tga->hdr.id_len;
		tmp[2] = tga->hdr.img_t;
		
		if(tga->hdr.map_t != 0) {
			tmp[1] = 1;
			tmp[3] = tga->hdr.map_first % 256;
			tmp[4] = tga->hdr.map_first /256;
			tmp[5] = tga->hdr.map_len % 256;
			tmp[6] = tga->hdr.map_len / 256;
			tmp[7] = tga->hdr.map_entry;
		} else {
			tmp[1] = 0;
			bzero(tmp + 4, 5);
		}
	
		tmp[8] = tga->hdr.x % 256;
		tmp[9] = tga->hdr.x / 256;
		tmp[10] = tga->hdr.y % 256;
		tmp[11] = tga->hdr.y / 256;
		tmp[12] = tga->hdr.width % 256;
		tmp[13] = tga->hdr.width / 256;
		tmp[14] = tga->hdr.height % 256;
		tmp[15] = tga->hdr.height / 256;
		tmp[16] = tga->hdr.depth;
		tmp[17] = tga->hdr.desc;

		if(TGAWrite(tga, tmp, TGA_HEADER_SIZE, 1) != 1) {
			TGAError(tga, TGA_WRITE_FAIL, "at %s line %d", __FILE__, __LINE__);
			return 0;
		}

		free(tmp);
		return TGA_HEADER_SIZE;
	}
	return 0;
}

int
TGAWriteImageId(TGA *tga, const tbyte *buf)
{
	tuint16 wrote;
	
	if(tga) {
		if(!buf) {
			TGAError(tga, TGA_BAD_PTR, "at %s line %d", __FILE__, __LINE__);
			return 0;
		}
		
		if(TGASeek(tga, TGA_HEADER_SIZE, SEEK_SET) != TGA_HEADER_SIZE) {
			TGAError(tga, TGA_SEEK_FAIL, "at %s line %d", __FILE__, __LINE__);
			return 0;
		}
		
		if((wrote = TGAWrite(tga, buf, tga->hdr.id_len, 1)) != 1)
			TGAError(tga, TGA_WRITE_FAIL, "at %s line %d", __FILE__, __LINE__);
		
		return wrote;
	}
	return 0;
}


int
TGAWriteColorMap(TGA *tga, const tbyte *buf)
{
	tuint32 n, off, wrote;

	if(tga) { 
		if(!buf) {
			TGAError(tga, TGA_BAD_PTR, "at %s line %d", __FILE__, __LINE__);
			return 0;
		}
		
		n = TGA_CMAP_SIZE(tga);
		off = TGA_CMAP_OFF(tga);
		
		if(TGASeek(tga, off, SEEK_SET) != off)  {
			TGAError(tga, TGA_SEEK_FAIL, "at %s line %d", __FILE__, __LINE__);
			return 0;
		}
		
		if((wrote = TGAWrite(tga, buf, n, 1)) != 1)
			TGAError(tga, TGA_WRITE_FAIL, "at %s line %d", __FILE__, __LINE__);
	
		return wrote;
	}
	return 0;
}
