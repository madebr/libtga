/*
 * tga.c
 *
 * Copyright (C) 2001-2002, Matthias Brueckner
 * This file is part of the TGA library (libtga).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "tga.h"


int
TGAIsEncoded(TGA *tga)
{
 	return (tga) ? tga->hdr.img_t > 8 && tga->hdr.img_t < 12 : 1;
}


int
TGAGetXOrientation(TGA *tga)
{
	return (tga) ? ((tga->hdr.desc & (1 << 4)) ? TGA_RIGHT : TGA_LEFT) : 0;
}


int
TGAGetYOrientation(TGA *tga)
{
	return (tga) ? ((tga->hdr.desc & (1 << 5)) ? TGA_TOP : TGA_BOTTOM) : 0;
}


off_t
TGASeek(TGA *tga, off_t off, int whence)
{
	fseek(tga->fd, off, whence);
	tga->off = ftell(tga->fd);
	return tga->off;
}


TGA*
TGAOpen(char *file, char *mode)
{
 	TGA *tga;
	FILE *f;

	tga = (TGA*)malloc(sizeof(TGA));
	if(!tga) {
		TGAError(NULL, TGA_OOM, "at %s line %d", __FILE__, __LINE__);
		return NULL;
	}
		
	tga->name = (char*)malloc(strlen(file));
	if(!tga->name) {
		TGAError(tga, TGA_OOM, "at %s line %d", __FILE__, __LINE__);
		free(tga);
		return NULL;
	}
	
	strcpy(tga->name, file);
	
	tga->off = 0;
	tga->row = 0;

	f = fopen(file, mode);
	if(!f) {
		TGAError(tga, TGA_OPEN_FAIL, "at %s line %d", __FILE__, __LINE__);
		free(tga);
		return NULL;
	}
	tga->fd = f;

	return tga;
}


void TGAClose(TGA *tga)
{
	if(tga) {
		fclose(tga->fd);
		free(tga->name);
		free(tga);
	}
}


void TGAError(TGA *tga, int code, char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	if(tga && tga->error) tga->error(code, fmt, arg);
	else { 
		vfprintf(stderr, fmt, arg);
		fprintf(stderr, ": TODO\n");
	}

	va_end(arg);
	tga->last = code;
}


void TGAMessage(TGA *tga, int code, char *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);

	if(tga && tga->message) tga->message(code, fmt, arg);
	else {
		vfprintf(stderr, fmt, arg);
		fprintf(stderr, ": TODO\n");
	}

	va_end(arg);
	tga->last = code;
}
