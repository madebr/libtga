/*
 *  tga.c
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
#include <stdlib.h>
#include <stdarg.h>

#include "tga.h"


off_t
TGASeek(TGA 	*tga, 
	off_t 	 off, 
	int 	 whence)
{
	fseek(tga->fd, off, whence);
	tga->off = ftell(tga->fd);
	return tga->off;
}

TGA*
TGAOpen(char *file, 
	char *mode)
{
 	TGA *tga;
	FILE *fd;

	tga = (TGA*)malloc(sizeof(TGA));
	if (!tga) {
		TGA_ERROR(tga, TGA_OOM);
		return NULL;
	}
		
	tga->name = (char*)malloc(strlen(file));
	if (!tga->name) {
		TGA_ERROR(tga, TGA_OOM);
		free(tga);
		return NULL;
	}
	
	strcpy(tga->name, file);
	
	tga->off = 0;

	fd = fopen(file, mode);
	if (!fd) {
		TGA_ERROR(tga, TGA_OPEN_FAIL);
		free(tga);
		return NULL;
	}
	tga->fd = fd;
	tga->last = TGA_OK;
	return tga;
}

void
TGAbgr2rgb(tbyte *data, size_t size, size_t bytes)
{
	size_t i;
	tbyte  tmp;
	
	for (i = 0; i < size; i += bytes) {
		tmp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = tmp;
	}
}

void 
TGAClose(TGA *tga)
{
	if (tga) {
		fclose(tga->fd);
		free(tga->name);
		free(tga);
	}
}

char*
TGAStrError(tuint8 code)
{
	if (code >= TGA_ERRORS) code = TGA_ERROR;
	return tga_error_strings[code - 1];
}
