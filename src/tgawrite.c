/*
 *  tgawrite.c
 *
 *  Copyright (C) 2001-2002  Matthias Brueckner <matbrc@gmx.de>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <sys/types.h>

#include "tga.h"

int TGAWriteRLE(TGA *tga, tbyte *buf);

size_t
TGAWrite(TGA 		*tga, 
	 const tbyte 	*buf, 
	 size_t 	 size, 
	 size_t 	 n)
{
	size_t wrote = fwrite(buf, size, n, tga->fd);
	tga->off = ftell(tga->fd);
	return wrote;
}

int TGAWriteImage(TGA 		*tga, 
		  TGAData 	*data)
{
	int ret = TGA_OK;

	if (!tga) return 0;

	if ((data->flags & TGA_IMAGE_ID) && tga->hdr.id_len > 0) {
		if (!TGAWriteImageId(tga, data->img_id)) {
			data->flags &= ~TGA_IMAGE_ID;
			TGA_ERROR(tga, tga->last);
			tga->hdr.id_len = 0;
			ret = 0;
		}
	}	
	if (data->flags & TGA_IMAGE_DATA) {
		if (data->cmap) {
			if (!TGAWriteColorMap(tga, data->cmap, data->flags)) {
				data->flags &= ~TGA_COLOR_MAP;
				TGA_ERROR(tga, tga->last);
				tga->hdr.map_t = 0;
				ret = 0;
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
			ret = 0;
		}
	}
	
	if (TGAWriteHeader(tga) != TGA_HEADER_SIZE) {
		TGA_ERROR(tga, tga->last);
		ret = 0;
	}
	
	return ret;
}

int
TGAWriteHeader(TGA *tga)
{
	tbyte *tmp;

	if (!tga) return 0;

	TGASeek(tga, 0, SEEK_SET);
	if(tga->off != 0) {
		TGA_ERROR(tga, TGA_SEEK_FAIL);
		return 0;
	}

	tmp = (tbyte*)malloc(18);
	if(!tmp) {
		TGA_ERROR(tga, TGA_OOM);
		return 0;
	}

	tmp[0] = tga->hdr.id_len;
	tmp[2] = tga->hdr.img_t;
		
	if (tga->hdr.map_t != 0) {
		tmp[1] = 1;
		tmp[3] = tga->hdr.map_first % 256;
		tmp[4] = tga->hdr.map_first /256;
		tmp[5] = tga->hdr.map_len % 256;
		tmp[6] = tga->hdr.map_len / 256;
		tmp[7] = tga->hdr.map_entry;
	} else {
		tmp[1] = 0;
		memset(tmp + 4, 0, 5);
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
	tmp[17] = tga->hdr.alpha;
	tmp[17] |= (tga->hdr.vert << 5);
	tmp[17] |= (tga->hdr.horz << 4);

	if (!TGAWrite(tga, tmp, TGA_HEADER_SIZE, 1)) {
		free(tmp);
		TGA_ERROR(tga, TGA_WRITE_FAIL);
		return 0;
	}

	free(tmp);
	tga->last = TGA_OK;
	return TGA_HEADER_SIZE;
}

int
TGAWriteImageId(TGA 		*tga, 
		const tbyte 	*buf)
{
	if (!tga || !buf || tga->hdr.id_len == 0) return 0;
		
	TGASeek(tga, TGA_HEADER_SIZE, SEEK_SET);
	if(tga->off != TGA_HEADER_SIZE) {
		TGA_ERROR(tga, TGA_SEEK_FAIL);
		return 0;
	}
	       
	if (!TGAWrite(tga, buf, tga->hdr.id_len, 1)) {
		TGA_ERROR(tga, TGA_WRITE_FAIL);
		return 0;
	}
		
	tga->last = TGA_OK;
	return TGA_OK;
}

int
TGAWriteColorMap(TGA    *tga, 
		 tbyte  *buf,
		 tuint32 flags)
{
	tuint32 n, off;

	if (!tga || !buf) return 0;

	n = TGA_CMAP_SIZE(tga);
	off = TGA_CMAP_OFF(tga);

	if (TGA_CAN_SWAP(tga->hdr.map_entry) && (flags & TGA_RGB)) {
		TGAbgr2rgb(buf, n, tga->hdr.map_entry / 8);
	}

	TGASeek(tga, off, SEEK_SET);
	if(tga->off != off) {
		TGA_ERROR(tga, TGA_SEEK_FAIL);
		return 0;
	}
		
	if(!TGAWrite(tga, buf, n, 1)) {
		TGA_ERROR(tga, TGA_WRITE_FAIL);
		return 0;
	}
		
	tga->last = TGA_OK;
	return TGA_OK;
}

size_t
TGAWriteScanlines(TGA 		*tga, 
		  tbyte 	*buf, 
		  size_t 	 sln, 
		  size_t 	 n,
		  tuint32        flags)
{
	off_t off;
	size_t wrote, sln_size, i;

	if (!tga || !buf) return 0;

	sln_size = TGA_SCANLINE_SIZE(tga);
	off = TGA_IMG_DATA_OFF(tga) + (sln * sln_size);
	
	if (tga->off != off) TGASeek(tga, off, SEEK_SET);
	if (tga->off != off) {
		TGA_ERROR(tga, TGA_SEEK_FAIL);
		return 0;
	}

	if (TGA_CAN_SWAP(tga->hdr.depth) && (flags & TGA_RGB)) 
		TGAbgr2rgb(buf + (sln * sln_size), sln_size * n, 
			   tga->hdr.depth / 8);
	
	if (flags & TGA_RLE_ENCODE) {
		for(wrote = 0; wrote < n; ++wrote) {
			if(!TGAWriteRLE(tga, buf + ((sln + wrote)*sln_size)))
				break;
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

int
TGAWriteRLE(TGA   *tga, 
	    tbyte *buf)
{
	tuint8 repeat = 0, direct = 0;
	tbyte bytes = tga->hdr.depth / 8, *from;
	tshort x;

	if (!tga || !buf) return 0;

	from = buf;
	
	for (x = 1; x < tga->hdr.width; ++x) {
		if (memcmp(buf, buf + bytes, bytes)) {
			/* next pixel is different */
			if (repeat) {
				putc(128 + repeat, tga->fd);
				fwrite(from, bytes, 1, tga->fd);
				from = buf + bytes; /* point to first different pixel */
				repeat = 0;
				direct = 0;
			} else { 
				direct += 1;
			}
		} else {
			/* next pixel is the same */
			if (direct) {
				putc(direct - 1, tga->fd);
				fwrite(from, bytes, direct, tga->fd);
				from = buf; /* point to first identical pixel */
				direct = 0;
				repeat = 1;
			} else {
			      repeat += 1;
			}
		}
		if (repeat == 128) {
			putc(255, tga->fd);
			fwrite(from, bytes, 1, tga->fd);
			from = buf + bytes;
			direct = 0;
			repeat = 0;
		} else if (direct == 128) {
			putc(127, tga->fd);
			fwrite(from, bytes, direct, tga->fd);
			from = buf + bytes;
			direct = 0;
			repeat = 0;
		}
		buf += bytes;
	}

	if (repeat > 0) {
		putc(128 + repeat, tga->fd);
		fwrite(from, bytes, 1, tga->fd);
	} else {
		putc(direct, tga->fd);
		fwrite(from, bytes, direct + 1, tga->fd);
	}

	tga->last = TGA_OK;
	return TGA_OK;
}
