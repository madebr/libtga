/*
 * tgaread.c - Read functions
 *
 * Copying (C) 2001, Matthias Brückner
 * This file is part of the TGA library (libtga)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */

#include "tga.h"


/* set user-defined read function */
void tga_set_read_fn(tga_ptr ptr, tga_io_func read_fn)
{
	if(ptr && read_fn) ptr->read_fn = read_fn;
}

void tga_read_data(tga_ptr ptr, tga_byte *buf, tga_uint_32 len)
{
	if(ptr->read_fn) ptr->read_fn(ptr, buf, len);
	else tga_default_read_data(ptr, buf, len);
}

/* read len bytes into buf */
void tga_default_read_data(tga_ptr ptr, tga_byte *buf, tga_uint_32 len)
{
        tga_uint_32 i = 0;
        tga_byte *tmp;

        if(len < 255) {
		tga_read_chunk(ptr, buf, len);
		return;
	}

	tmp = (tga_byte*)tga_malloc(ptr, 255);
	while(len > 255) {
		len -= 255;
		tga_read_chunk(ptr, tmp, 255);
		tga_memcpy(tmp, buf, i, 255);
		i += 255;
	}
	tga_read_chunk(ptr, tmp, len);
	tga_memcpy(tmp, buf, i, len);
	free(tmp);
}

/* read len bytes into buf */
void tga_read_chunk(tga_ptr ptr, tga_byte *buf, tga_uint_8 len)
{
	if(fread(buf, 1, len, (FILE*)ptr->io_ptr) != len)
	        tga_error(ptr, "tga_read_chunk(tgaread.c)", TGA_ERROR_READ);
}

/* set user-defined seek function */
void tga_set_seek_fn(tga_ptr ptr, tga_seek_func seek_fn)
{
	if(ptr && seek_fn) ptr->seek_fn = seek_fn;
}

void tga_seek(tga_ptr ptr, tga_off_t off, tga_uint_8 whence)
{
	if(ptr->seek_fn) ptr->seek_fn(ptr, off, whence);
	else tga_default_seek(ptr, off, whence);
}

/* seek off bytes from position indicated by whence */
void tga_default_seek(tga_ptr ptr, tga_off_t off, tga_uint_8 whence)
{
	fseek((FILE*)ptr->io_ptr, off, whence);
}



/* read header and all sections specified in flags */
void tga_read_tga(tga_ptr ptr, tga_info_ptr info, tga_uint_32 flags)
{
	tga_uint_32 tmp = info->map_len * info->map_entry_size;

	if(!ptr) return;
	if(!info) tga_error(ptr, "tga_read_tga(tgaread.c)", TGA_BAD_ARG);

	tga_read_info(ptr, info, flags);
	tga_set_flag(ptr, TGA_IMAGE_INFO);

	if((flags & TGA_IMAGE_ID) && info->id_len != 0 ) {
		ptr->img_id = (tga_byte*)tga_malloc(ptr, info->id_len);
		tga_read_data(ptr, ptr->img_id, info->id_len);
		tga_set_flag(ptr, TGA_IMAGE_ID);
	}
	if((flags & TGA_COLOR_MAP) && (info->map_t == 1)) {
		tga_seek(ptr, 18 + info->id_len, SEEK_SET);
		ptr->color_map = (tga_byte*)tga_malloc(ptr, tmp);
		tga_read_data(ptr, ptr->color_map, tmp);
		tga_set_flag(ptr, TGA_COLOR_MAP);
	}
	if((flags & TGA_IMAGE_DATA) && info->img_t != 0) {
		tga_seek(ptr, 18 + info->id_len + tmp, SEEK_SET);
		tga_read_image_data(ptr, info, flags);
		tga_set_flag(ptr, TGA_IMAGE_DATA);
	}
}

/* read TGA header */
void tga_read_info(tga_ptr ptr, tga_info_ptr info, tga_uint_32 flags)
{
	tga_byte *tmp = (tga_byte*)tga_malloc(ptr, 18);
	tga_seek(ptr, 0, SEEK_SET);
	tga_read_data(ptr, tmp, 18);

	info->id_len = tmp[0];
	info->map_t = tmp[1];
	info->img_t = tmp[2];
	info->map_first_entry = tga_get_uint_16(tmp + 3);
	info->map_len = tga_get_uint_16(tmp + 5);
	info->map_entry_size = tmp[7];
	info->x = tga_get_uint_16(tmp + 8);
	info->y = tga_get_uint_16(tmp + 10);
	info->width = tga_get_uint_16(tmp + 12);
	info->height = tga_get_uint_16(tmp + 14);
	info->depth = tmp[16];
	info->desc = tmp[17];
}

/* read image data */
void tga_read_image_data(tga_ptr ptr, tga_info_ptr info, tga_uint_32 flags)
{
	tga_uint_32 pixels, len, tmp = 0;

	pixels = info->width * info->height;

	if(info->img_t == 0)
		tga_warning(ptr, "tga_read_image_data(tgaread.c)", TGA_BAD_FORMAT);

	/* uncompressed image data */
	if(info->img_t > 0 && info->img_t < 4) {
		if(info->img_t == 1 || info->img_t == 3)
			len = pixels;
		if(info->img_t == 2)
			len = pixels * info->depth / 8;

		ptr->img_data = (tga_byte*)tga_malloc(ptr, len);
                tga_read_data(ptr, ptr->img_data, len);
	}
}

