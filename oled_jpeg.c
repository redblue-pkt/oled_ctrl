/*
 *   Copyright (C) redblue 2019
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "oled_jpeg.h"
#include "oled_main.h"

int fh_jpeg_id(const char *name)
{
	int fd;
	unsigned char id[10];
	fd = open(name, O_RDONLY);
	if (fd == -1)
		return(0);
	read(fd, id,10);
	close(fd);
	if (id[6] == 'J' && id[7] == 'F' && id[8] == 'I' && id[9] == 'F')
		return(1);
	if (id[0] == 0xff && id[1] == 0xd8 && id[2] == 0xff)
		return(1);
	return(0);
}

static inline void jpeg_cb_error_exit(j_common_ptr cinfo)
{
	struct r_jpeg_error_mgr *mptr;
	mptr = (struct r_jpeg_error_mgr*) cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(mptr->envbuffer,1);
}

int fh_jpeg_load(const char *name, unsigned char **buffer, int* x, int* y)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_decompress_struct *ciptr;
	struct r_jpeg_error_mgr emgr;
	unsigned char *bp;
	int px,py,c, ix;
	FILE *fh;
	JSAMPLE *lb;

	ciptr=&cinfo;
	if (!(fh=fopen(name,"rb")))
		return(FH_ERROR_FILE);
	ciptr->err=jpeg_std_error(&emgr.pub);
	emgr.pub.error_exit=jpeg_cb_error_exit;
	if (setjmp(emgr.envbuffer)==1)
	{
		// FATAL ERROR - Free the object and return...
		jpeg_destroy_decompress(ciptr);
		fclose(fh);
		return(FH_ERROR_FORMAT);
	}

	jpeg_create_decompress(ciptr);
	jpeg_stdio_src(ciptr,fh);
	jpeg_read_header(ciptr,TRUE);
	ciptr->out_color_space=JCS_RGB;
	ciptr->dct_method=JDCT_FASTEST;
	ix = (int)ciptr->image_width;
	if (*x == ix)
		ciptr->scale_denom=1;
#if __cplusplus < 201103
	else if (abs(*x*2 - ix) < 2)
		ciptr->scale_denom=2;
	else if (abs(*x*4 - ix) < 4)
		ciptr->scale_denom=4;
	else if (abs(*x*8 - ix) < 8)
		ciptr->scale_denom=8;
#else
	else if (std::abs(*x*2 - ix) < 2)
		ciptr->scale_denom=2;
	else if (std::abs(*x*4 - ix) < 4)
		ciptr->scale_denom=4;
	else if (std::abs(*x*8 - ix) < 8)
		ciptr->scale_denom=8;
#endif
	else
		ciptr->scale_denom=1;

	jpeg_start_decompress(ciptr);

	px=ciptr->output_width; py=ciptr->output_height;
	c=ciptr->output_components;
	if (px > *x || py > *y)
	{
		// pic act larger, e.g. because of not responding jpeg server
		free(*buffer);
		*buffer = (unsigned char*) malloc(px*py*3);
		*x = px;
		*y = py;
	}

	if (c == 3)
	{
		lb=(JSAMPLE*)(*ciptr->mem->alloc_small)((j_common_ptr) ciptr,JPOOL_PERMANENT,c*px);
		bp=*buffer;
		while (ciptr->output_scanline < ciptr->output_height)
		{
			jpeg_read_scanlines(ciptr, &lb, 1);
			memmove(bp,lb,px*c);
			bp+=px*c;
		}

	}
	jpeg_finish_decompress(ciptr);
	jpeg_destroy_decompress(ciptr);
	fclose(fh);
	return(FH_ERROR_OK);
}

int fh_jpeg_getsize(const char *name,int *x,int *y)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_decompress_struct *ciptr;
	struct r_jpeg_error_mgr emgr;
	int px,py,c;
	FILE *fh;

	ciptr=&cinfo;
	if(!(fh=fopen(name,"rb")))
		return(FH_ERROR_FILE);

	ciptr->err=jpeg_std_error(&emgr.pub);
	emgr.pub.error_exit=jpeg_cb_error_exit;
	if (setjmp(emgr.envbuffer)==1)
	{
		// FATAL ERROR - Free the object and return...
		jpeg_destroy_decompress(ciptr);
		fclose(fh);
		return(FH_ERROR_FORMAT);
	}

	jpeg_create_decompress(ciptr);
	jpeg_stdio_src(ciptr,fh);
	jpeg_read_header(ciptr,TRUE);
	ciptr->out_color_space=JCS_RGB;
	jpeg_start_decompress(ciptr);
	px=ciptr->output_width;
	py=ciptr->output_height;
	c=ciptr->output_components;
	*x=px;
	*y=py;
	jpeg_destroy_decompress(ciptr);
	fclose(fh);
	return(FH_ERROR_OK);
}

