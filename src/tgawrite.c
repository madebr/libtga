/*
 *  tgawrite.c
 *
 *  Copyright (C) 2001-2002  Matthias Brueckner  <matbrc@gmx.de>
 *  This file is part of the TGA library (libtga).
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tga.h>
#include "tga_private.h"

#define LSB_SH(SHORT) ((SHORT) & 0xff)
#define MSB_SH(SHORT) ((SHORT) >> 8)

size_t
TGAWrite(TGA 	     *tga, 
	 const tbyte *buf, 
	 size_t       size, 
	 size_t       n)
{
	size_t wrote = fwrite(buf, size, n, tga->fd);
	if (wrote != n) {
		TGA_ERROR(tga, TGA_WRITE_FAIL);
	}
	tga->off = ftell(tga->fd);
	return wrote;
}


int TGAWriteImage(TGA 	  *tga, 
		  TGAData *data)
{
	if (!tga) return TGA_ERROR;

	if (data->flags & TGA_IMAGE_ID) {
		TGAWriteImageId(tga, data);
		if (!__TGA_SUCCEEDED(tga)) {
			return __TGA_LASTERR(tga);
		}
	}

	if (data->flags & TGA_IMAGE_DATA) {
		if (data->cmap) {
			if (!TGAWriteColorMap(tga, data->cmap, data->flags)) {
				data->flags &= ~TGA_COLOR_MAP;
				TGA_ERROR(tga, tga->last);
				tga->hdr.map_t = 0;
			}
			tga->hdr.img_t = 1;
			tga->hdr.map_t = 1;
		} else {
			tga->hdr.map_t = 0;
		}

		if (TGAWriteScanlines(tga, data->img_data, 0, tga->hdr.height, data->flags) != tga->hdr.height) {
			data->flags &= ~TGA_IMAGE_DATA;
			TGA_ERROR(tga, tga->last);
			tga->hdr.img_t = 0;
		}
	}
	
	if (TGAWriteHeader(tga) != TGA_OK) {
		TGA_ERROR(tga, tga->last);
	}
	
	return tga->last;
}


int
TGAWriteHeader(TGA *tga)
{
	if (!tga) return 0;

	__TGASeek(tga, 0, SEEK_SET);
	if (!__TGA_SUCCEEDED(tga)) {
		return __TGA_LASTERR(tga);
	}

	tbyte tmp[TGA_HEADER_SIZE];

	tmp[0] = tga->hdr.id_len;
	tmp[2] = tga->hdr.img_t;
		
	if (tga->hdr.map_t != 0) {
		tmp[1] = 1;
		tmp[3] = LSB_SH(tga->hdr.map_first);
		tmp[4] = MSB_SH(tga->hdr.map_first);
		tmp[5] = LSB_SH(tga->hdr.map_len);
		tmp[6] = MSB_SH(tga->hdr.map_len);
		tmp[7] = tga->hdr.map_entry;
	} else {
		tmp[1] = 0;
		memset(tmp + 4, 0, 5);
	}

	tmp[8] = LSB_SH(tga->hdr.x);
	tmp[9] = MSB_SH(tga->hdr.x);
	tmp[10] = LSB_SH(tga->hdr.y);
	tmp[11] = MSB_SH(tga->hdr.y);
	tmp[12] = LSB_SH(tga->hdr.width);
	tmp[13] = MSB_SH(tga->hdr.width);
	tmp[14] = LSB_SH(tga->hdr.height);
	tmp[15] = MSB_SH(tga->hdr.height);
	tmp[16] = tga->hdr.depth;
	tmp[17] = tga->hdr.alpha;
	tmp[17] |= (tga->hdr.horz << 4);
	tmp[17] |= (tga->hdr.vert << 5);

	TGAWrite(tga, tmp, TGA_HEADER_SIZE, 1);
	if (!__TGA_SUCCEEDED(tga)) {
		return __TGA_LASTERR(tga);
	}

	return TGA_OK;
}


int
TGAWriteImageId(TGA	*tga,
		TGAData	*data)
{
	if (!tga) return TGA_ERROR;

	if (tga->hdr.id_len == 0) return TGA_OK;

	if (!data) {
		TGA_ERROR(tga, TGA_ERROR);
		return __TGA_LASTERR(tga);
	}

	if (!data->img_id) {
		data->flags &= ~TGA_IMAGE_ID;
		TGA_ERROR(tga, TGA_ERROR);
		return __TGA_LASTERR(tga);
	}

	__TGASeek(tga, TGA_HEADER_SIZE, SEEK_SET);
	if (!__TGA_SUCCEEDED(tga)) {
		return __TGA_LASTERR(tga);
	}

	if (!TGAWrite(tga, data->img_id, tga->hdr.id_len, 1)) {
		TGA_ERROR(tga, TGA_WRITE_FAIL);
		return __TGA_LASTERR(tga);
	}

	return TGA_OK;
}


int
TGAWriteColorMap(TGA     *tga, 
		 tbyte   *buf,
		 tuint32  flags)
{
	tlong n, off;

	if (!tga || !buf) return 0;

	n = TGA_CMAP_SIZE(tga);
	off = TGA_CMAP_OFF(tga);

	if (TGA_CAN_SWAP(tga->hdr.map_entry) && (flags & TGA_RGB)) {
		__TGAbgr2rgb(buf, n, tga->hdr.map_entry / 8);
	}

	__TGASeek(tga, off, SEEK_SET);
	if (!__TGA_SUCCEEDED(tga)) {
		return __TGA_LASTERR(tga);
	}
		
	if (!TGAWrite(tga, buf, n, 1)) {
		TGA_ERROR(tga, TGA_WRITE_FAIL);
		return 0;
	}
		
	tga->last = TGA_OK;
	return TGA_OK;
}


int
TGAWriteRLE(TGA   *tga, 
	    tbyte *buf)
{
	tuint8 *from, repeat = 0, direct = 0, bytes = tga->hdr.depth / 8;
	tshort x;
	tlong width = tga->hdr.width;
	FILE *fd = tga->fd;
	
	if (!tga || !buf) return 0;

	from = buf;
	
	for (x = 1; x < width; ++x) {
		if (memcmp(buf, buf + bytes, bytes)) {
			if (repeat) {
				putc(128 + repeat, fd);
				fwrite(from, bytes, 1, fd);
				from = buf + bytes; 
				repeat = 0;
				direct = 0;
			} else { 
				direct += 1;
			}
		} else {
			if (direct) {
				putc(direct - 1, fd);
				fwrite(from, bytes, direct, fd);
				from = buf; 
				direct = 0;
				repeat = 1;
			} else {
			      repeat += 1;
			}
		}
		if (repeat == 128) {
			putc(255, fd);
			fwrite(from, bytes, 1, fd);
			from = buf + bytes;
			direct = 0;
			repeat = 0;
		} else if (direct == 128) {
			putc(127, fd);
			fwrite(from, bytes, direct, fd);
			from = buf + bytes;
			direct = 0;
			repeat = 0;
		}
		buf += bytes;
	}

	if (repeat > 0) {
		putc(128 + repeat, fd);
		fwrite(from, bytes, 1, fd);
	} else {
		putc(direct, fd);
		fwrite(from, bytes, direct + 1, fd);
	}

	tga->last = TGA_OK;
	return TGA_OK;
}


size_t
TGAWriteScanlines(TGA 	  *tga, 
		  tbyte   *buf, 
		  size_t   sln, 
		  size_t   n,
		  tuint32  flags)
{
	tlong off;
	size_t wrote, sln_size;

	if (!tga || !buf) return 0;

	sln_size = TGA_SCANLINE_SIZE(tga);
	off = TGA_IMG_DATA_OFF(tga) + (sln * sln_size);
	
	if (tga->off != off) {
		__TGASeek(tga, off, SEEK_SET);
		if (!__TGA_SUCCEEDED(tga)) {
			return __TGA_LASTERR(tga);
		}
	}

	if (TGA_CAN_SWAP(tga->hdr.depth) && (flags & TGA_RGB)) 
		__TGAbgr2rgb(buf + (sln * sln_size), sln_size * n, 
			   tga->hdr.depth / 8);
	
	if (flags & TGA_RLE_ENCODE) {
		for(wrote = 0; wrote < n; ++wrote) {
			if(TGAWriteRLE(tga, buf + ((sln + wrote)*sln_size)) !=
				TGA_OK) break;
		}
		tga->hdr.img_t += 8;
	} else {
		wrote = TGAWrite(tga, buf, sln_size, n);
	}
	if (wrote != n) {
		TGA_ERROR(tga, TGA_WRITE_FAIL);
		return wrote;
	}
		
	tga->last = TGA_OK;
	return wrote;
}
