/*
 *  tgaread.c
 *
 *  Copying (C) 2001-2002  Matthias Brueckner <matbrc@gmx.de>
 *  This file is part of the struct TGA library (libtga).
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
#include <strings.h>
#include <tga.h>
#include "tga_private.h"

size_t
TGARead(TGA    *tga, 
	tbyte  *buf,
	size_t 	size, 
	size_t 	n)
{
	size_t read = fread(buf, size, n, tga->fd);
	if (read != n) {
		TGA_ERROR(tga, TGA_READ_FAIL);
	}
	tga->off = ftell(tga->fd);
	return read;
}

int
TGAReadImage(TGA     *tga, 
	     TGAData *data)
{
	if (!tga) return TGA_ERROR;
	if (!data) {
		TGA_ERROR(tga, TGA_ERROR);
		return __TGA_LASTERR(tga);
	}

	TGAReadHeader(tga);
	if (!__TGA_SUCCEEDED(tga)) {
		data->flags &= ~TGA_IMAGE_INFO;
		return __TGA_LASTERR(tga);
	}
	data->flags |= TGA_IMAGE_INFO;

	data->img_id = (tbyte *) 0;
	data->cmap = (tbyte *) 0;
	data->img_data = (tbyte *) 0;

	if (data->flags & TGA_IMAGE_ID) {
		TGAReadImageId(tga, data);
		if (!__TGA_SUCCEEDED(tga)) {
			return __TGA_LASTERR(tga);
		}
	}

	if (data->flags & TGA_IMAGE_DATA) {
		if (TGA_IS_MAPPED(tga)) {
			TGAReadColorMap(tga, data);
			if (!__TGA_SUCCEEDED(tga)) {
				return __TGA_LASTERR(tga);
			}
		} else {
			data->flags &= ~TGA_COLOR_MAP;
		}

		TGAReadScanlines(tga, data);
		if (!__TGA_SUCCEEDED(tga)) {
			return __TGA_LASTERR(tga);
		}
	}
	return TGA_OK;
}

void
TGAFreeTGAData(TGAData *data)
{
	if (data->cmap)
		free(data->cmap);
	if (data->img_data)
		free(data->img_data);
	if (data->img_id)
		free(data->img_id);
}

int
TGAReadHeader (TGA *tga)
{
	if (!tga) return TGA_ERROR;

	__TGASeek(tga, 0, SEEK_SET);
	if (!__TGA_SUCCEEDED(tga)) {
		return __TGA_LASTERR(tga);
	}

	tbyte *tmp = (tbyte*) malloc(TGA_HEADER_SIZE);
	if (!tmp) {
		TGA_ERROR(tga, TGA_OOM);
		return __TGA_LASTERR(tga);
	}

	bzero(tmp, TGA_HEADER_SIZE);

	TGARead(tga, tmp, TGA_HEADER_SIZE, 1);
	if (!__TGA_SUCCEEDED(tga)) {
		free(tmp);
		return __TGA_LASTERR(tga);
	}

	tga->hdr.id_len		= tmp[ 0];
	tga->hdr.map_t		= tmp[ 1];
	tga->hdr.img_t		= tmp[ 2];
	tga->hdr.map_first	= tmp[ 3] + ((tshort) tmp[ 4]) * 256;
	tga->hdr.map_len	= tmp[ 5] + ((tshort) tmp[ 6]) * 256;
	tga->hdr.map_entry	= tmp[ 7];
	tga->hdr.x		= tmp[ 8] + ((tshort) tmp[ 9]) * 256;
	tga->hdr.y		= tmp[10] + ((tshort) tmp[11]) * 256;
	tga->hdr.width		= tmp[12] + ((tshort) tmp[13]) * 256;
	tga->hdr.height		= tmp[14] + ((tshort) tmp[15]) * 256;
	tga->hdr.depth		= tmp[16];
	tga->hdr.alpha		= tmp[17] & 0x0f;
	tga->hdr.horz		= (tmp[17] & 0x10) ? TGA_RIGHT : TGA_LEFT;
	tga->hdr.vert		= (tmp[17] & 0x20) ? TGA_TOP : TGA_BOTTOM;

	if (tga->hdr.map_t && tga->hdr.depth != 8) {
		TGA_ERROR(tga, TGA_UNKNOWN_SUB_FORMAT);
		free(tmp);
		return __TGA_LASTERR(tga);
	}

	if (tga->hdr.depth != 8 &&
	    tga->hdr.depth != 15 &&
	    tga->hdr.depth != 16 &&
	    tga->hdr.depth != 24 &&
	    tga->hdr.depth != 32) 
	{
		TGA_ERROR(tga, TGA_UNKNOWN_SUB_FORMAT);
		free(tmp);
		return __TGA_LASTERR(tga);
	}

	free(tmp);
	tga->last = TGA_OK;
	return __TGA_LASTERR(tga);
}

int
TGAReadImageId(TGA    *tga, 
	       TGAData *data)
{
	if (!tga) return TGA_ERROR;
	if (!data) {
		TGA_ERROR(tga, TGA_ERROR);
		return __TGA_LASTERR(tga);
	}
	if (tga->hdr.id_len == 0) {
		data->flags &= ~TGA_IMAGE_ID;
		return TGA_OK;
	}

	__TGASeek(tga, TGA_HEADER_SIZE, SEEK_SET);
	if (!__TGA_SUCCEEDED(tga)) {
		data->flags &= ~TGA_IMAGE_ID;
		return __TGA_LASTERR(tga);
	}
	data->img_id = (tbyte*) realloc(data->img_id, tga->hdr.id_len);
	if (!data->img_id) {
		data->flags &= ~TGA_IMAGE_ID;
		TGA_ERROR(tga, TGA_OOM);
		return __TGA_LASTERR(tga);
	}

	TGARead(tga, data->img_id, tga->hdr.id_len, 1);
	if (!__TGA_SUCCEEDED(tga)) {
		data->flags &= ~TGA_IMAGE_ID;
		return __TGA_LASTERR(tga);
	}

	data->flags |= TGA_IMAGE_ID;
	tga->last = TGA_OK;
	return TGA_OK;
}


int
convert_16_to_24(TGA *tga, tbyte *buf, size_t size_buf, tbyte **bufout)
{
	if (size_buf % 2) {
		TGA_ERROR(tga, TGA_ERROR);
		return __TGA_LASTERR(tga);
	}
	size_t new_size = size_buf + size_buf / 2;

	tbyte *newbuf = (tbyte *) malloc(new_size);
	if (!newbuf) {
		TGA_ERROR(tga, TGA_OOM);
		return __TGA_LASTERR(tga);
	}

	for(size_t buf_i = size_buf, newbuf_i = new_size; buf_i != 0; buf_i -= 2, newbuf_i -= 3) {
		tlong tmp = buf[buf_i - 2] + buf[buf_i - 1] * 256;
		newbuf[newbuf_i - 3] = (tmp >> 10) & 0x1F;
		newbuf[newbuf_i - 2] = (tmp >> 5) & 0x1F;
		newbuf[newbuf_i - 1] = (tmp >> 5) & 0x1F;
	}
	*bufout = newbuf;
	return TGA_OK;
}


int
TGAReadColorMap (TGA 	  *tga,
		 TGAData *data)
{
	if (!tga) return TGA_ERROR;
	if (!data) {
		TGA_ERROR(tga, TGA_ERROR);
		return __TGA_LASTERR(tga);
	}

	tlong n = TGA_CMAP_SIZE(tga);
	if (n == 0) {
		data->flags &= ~TGA_COLOR_MAP;
		return TGA_OK;
	}

	tlong off = TGA_CMAP_OFF(tga);
	if (tga->off != off) {
		__TGASeek(tga, off, SEEK_SET);
		if (!__TGA_SUCCEEDED(tga)) {
			data->flags &= ~TGA_COLOR_MAP;
			return __TGA_LASTERR(tga);
		}
	}

	data->cmap = (tbyte*) realloc(data->cmap, n);
	if (!data->cmap) {
		data->flags &= ~TGA_COLOR_MAP;
		TGA_ERROR(tga, TGA_OOM);
		return __TGA_LASTERR(tga);
	}

	tlong read = TGARead(tga, data->cmap, n, 1);
	if (!__TGA_SUCCEEDED(tga)) {
		data->flags &= ~TGA_COLOR_MAP;
		return __TGA_LASTERR(tga);
	}

	if (TGA_CAN_SWAP(tga->hdr.map_entry) && (data->flags & TGA_RGB)) {
		__TGAbgr2rgb(data->cmap, n, tga->hdr.map_entry / 8);
	}

	if (tga->hdr.map_entry == 15 || tga->hdr.map_entry == 16) {
		tbyte *newcmap;
		convert_16_to_24(tga, data->cmap, read, &newcmap);
		if (!__TGA_SUCCEEDED(tga)) {
			data->flags &= ~TGA_COLOR_MAP;
			return __TGA_LASTERR(tga);
		}
		free(data->cmap);
		data->cmap = newcmap;
	}

	data->flags |= TGA_COLOR_MAP;
	tga->last = TGA_OK;
	return read;
}

int
TGAReadRLE(TGA   *tga, 
	   tbyte *buf)
{
	int head;
	char sample[4];
	tbyte k, repeat = 0, direct = 0, bytes = tga->hdr.depth / 8;
	tshort x;
	tshort width = tga->hdr.width;
	FILE *fd = tga->fd;

	if (!tga || !buf) return TGA_ERROR;

	for (x = 0; x < width; ++x) {
		if (repeat == 0 && direct == 0) {
			head = getc(fd);
			if (head == EOF) return TGA_ERROR;
			if (head >= 128) {
				repeat = head - 127;
				if (fread(sample, bytes, 1, fd) < 1) 
					return TGA_ERROR;
			} else {
				direct = head + 1;
			}
		}
		if (repeat > 0) {
			for (k = 0; k < bytes; ++k) buf[k] = sample[k];
			--repeat;
		} else {
			if (fread(buf, bytes, 1, fd) < 1) return TGA_ERROR;
			--direct;
		}
		buf += bytes;
	}

	tga->last = TGA_OK;
	return TGA_OK;
}

int
TGAReadScanlines(TGA	 *tga,
		 TGAData *data)
{
	if (!tga) return TGA_ERROR;

	if (!tga->hdr.img_t) {
		data->flags &= ~TGA_IMAGE_DATA;
		return TGA_OK;
	}

	if (!data) {
		TGA_ERROR(tga, TGA_ERROR);
		return __TGA_LASTERR(tga);
	}

	data->img_data = (tbyte*) realloc(data->img_data, TGA_IMG_DATA_SIZE(tga));
	if (!data->img_data) {
		data->flags &= ~TGA_IMAGE_DATA;
		TGA_ERROR(tga, TGA_OOM);
		return __TGA_LASTERR(tga);
	}

	size_t sln_start = 0;
	size_t sln_stop = tga->hdr.height;
	size_t sln_size = TGA_SCANLINE_SIZE(tga);
	tlong off = TGA_IMG_DATA_OFF(tga) + (sln_start * sln_size);

	if (tga->off != off) {
		__TGASeek(tga, off, SEEK_SET);
		if (!__TGA_SUCCEEDED(tga)) {
			return __TGA_LASTERR(tga);
		}
	}

	if (TGA_IS_ENCODED(tga)) {
		for(size_t sln_i = sln_start; sln_i < sln_stop; ++sln_i) {
			TGAReadRLE(tga, data->img_data + (sln_i * sln_size));
			if (!__TGA_SUCCEEDED(tga)) {
				data->flags &= ~TGA_IMAGE_DATA;
				return __TGA_LASTERR(tga);
			}
		}
		tga->hdr.img_t -= 8;
	} else {
		TGARead(tga, data->img_data + (sln_start * sln_size),
			sln_size, sln_stop - sln_start);
		if (!__TGA_SUCCEEDED(tga)) {
			return __TGA_LASTERR(tga);
		}
	}

	if (TGA_CAN_SWAP(tga->hdr.depth) && (data->flags & TGA_RGB)) {
		__TGAbgr2rgb(data->img_data + (sln_start * sln_size),
			sln_size * (sln_stop - sln_start),
			tga->hdr.depth / 8);
	}

	if (tga->hdr.depth == 15 || tga->hdr.depth == 16) {
		tbyte *new_img;
		convert_16_to_24(tga, data->img_data + (sln_start * sln_size),
			(sln_stop - sln_start) * sln_size, &new_img);
		if (!__TGA_SUCCEEDED(tga)) {
			data->flags &= ~TGA_COLOR_MAP;
			return __TGA_LASTERR(tga);
		}
		free(data->img_data);
		data->img_data = new_img;
	}

	return TGA_OK;
}
