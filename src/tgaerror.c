/*
 * tgaerror.c - Error handling functions
 *
 * Copyright (C) 2001, Matthias Brückner
 * This file is part of the TGA library (libtga).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 */

#include "tga.h"


/* like strerror(3) */
char* tga_get_str_error(tga_err_t errno)
{
	errno -= 1;
	if(errno > num_errors) errno = TGA_ERROR;
	return str_errors[errno];
}

/* set user-defined error function */
void tga_set_error_fn(tga_ptr ptr, tga_err_func err_fn, tga_err_func warn_fn)
{
	ptr->err_fn = err_fn;
	ptr->warn_fn = warn_fn;
}

/* if exists calls user-defined error function else the default function */
void tga_error(tga_ptr ptr, char *msg, tga_err_t errno)
{
	if(ptr->err_fn) ptr->err_fn(ptr, msg, errno);
	tga_default_error(ptr, msg, errno);
}

/* default error function provided by the library */
void tga_default_error(tga_ptr ptr, char *msg, tga_err_t errno)
{
	fprintf(stderr, "libtga error: %s (%s)\n", msg,
		tga_get_str_error(errno));

        fclose((FILE*)ptr->io_ptr);
	tga_free_tga(ptr);

	exit(1);
}

/* if exists calls user-defined warning function else the default function */
void tga_warning(tga_ptr ptr, char *msg, tga_err_t errno)
{
	if(ptr->warn_fn) ptr->warn_fn(ptr, msg, errno);
	else tga_default_warning(ptr, msg, errno);
}

/* default warning function provided by the library */
void tga_default_warning(tga_ptr ptr, char *msg, tga_err_t errno)
{
	fprintf(stderr, "libtga warning: %s (%s)\n", msg,
		tga_get_str_error(errno));
}





