/*
 * tgawrite.c - Write functions
 *
 * Copyright (C) 2001, Matthias Brückner
 * This file is part of the TGA library (libtga).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */

#include "tga.h"


/* set user-defined write function */
void tga_set_write_fn(tga_ptr ptr, tga_io_func write_fn)
{
	if(ptr && write_fn) ptr->write_fn = write_fn;
}

/* write len bytes from buf */
void tga_write_data(tga_ptr ptr, tga_byte *buf, tga_uint_32 len)
{
	if(ptr->write_fn) ptr->write_fn(ptr, buf, len);
	else tga_default_write_data(ptr, buf, len);
}

/* write len bytes from buf (more than 255) */
void tga_default_write_data(tga_ptr ptr, tga_byte *buf, tga_uint_32 len)
{
	tga_uint_32 i = 0;

	while(len > 255) {
		len -= 255;
		tga_write_chunk(ptr, buf + i, 255);
		i += 255;
	}
	tga_write_chunk(ptr, buf + i, len);
}

/* write len bytes from buf */
void tga_write_chunk(tga_ptr ptr, tga_byte *buf, tga_uint_8 len)
{
	if(ptr->io_ptr) {
                if(fwrite(buf, 1, len, (FILE*)ptr->io_ptr) != len)
                        tga_error(ptr, "tga_write_chunk(tgawrite.c)", TGA_ERROR_WRITE);
        } else
                tga_error(ptr, "tga_write_chunk(tgawrite.c)", TGA_BAD_FD);
}

/* write header and all sections specified in flags */
void tga_write_tga(tga_ptr ptr, tga_info_ptr info, tga_uint_32 flags)
{
	tga_write_info(ptr, info, flags);

	if(tga_is_flag(ptr, TGA_IMAGE_ID) && info->id_len != 0) {
		tga_seek(ptr, 18, SEEK_SET);
		tga_write_data(ptr, ptr->img_id, info->id_len);
	}

	if(tga_is_flag(ptr, TGA_COLOR_MAP) && info->map_t == 1) {
		tga_write_data(ptr, ptr->color_map, info->map_len * info->map_entry_size);
	}

	if(tga_is_flag(ptr, TGA_IMAGE_DATA) && info->img_t != 0) {
		tga_write_image_data(ptr, info, flags);
	}
}

/* write TGA header */
void tga_write_info(tga_ptr ptr, tga_info_ptr info, tga_uint_32 flags)
{
	tga_byte *tmp = (tga_byte*)tga_malloc(ptr, 18);
	tga_seek(ptr, 0, SEEK_SET);

	tmp[0] = info->id_len;
	tmp[2] = info->img_t;
	if(flags & TGA_COLOR_MAP) {
		tmp[1] = 1;
		tga_cpy_uint_16((tga_byte*)(&info->map_first_entry), tmp, 3);
		tga_cpy_uint_16((tga_byte*)(&info->map_len), tmp, 5);
		tmp[7] = info->map_entry_size;
	} else {
		tmp[1] = 0;
		bzero(tmp + 4, 5);
	}
	tga_cpy_uint_16((tga_byte*)(&info->x), tmp, 8);
	tga_cpy_uint_16((tga_byte*)(&info->y), tmp, 10);
	tga_cpy_uint_16((tga_byte*)(&info->width), tmp, 12);
	tga_cpy_uint_16((tga_byte*)(&info->height), tmp, 14);
	tmp[16] = info->depth;
	tmp[17] = info->desc;

	tga_write_data(ptr, tmp, 18);
	free(tmp);
}

/* write image data */
void tga_write_image_data(tga_ptr ptr, tga_info_ptr info, tga_uint_32 flags)
{
	tga_uint_32 len;
	if(info->img_t > 0 && info->img_t < 4) {
		if(info->img_t == 1 || info->img_t == 3)
			len = info->width * info->height;
		if(info->img_t == 2)
			len = info->width * info->height * info->depth / 8;

		tga_write_data(ptr, ptr->img_data, len);
	} else {
		tga_warning(ptr, "tga_write_image_data(tgawrite.c)", info->img_t);
	}
}

