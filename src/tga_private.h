#ifndef TGA_PRIVATE_H
#define TGA_PRIVATE_H

#include <tga.h>

tlong __TGASeek(TGA *tga, tlong off, int whence);

void __TGAbgr2rgb(tbyte *data, size_t size, size_t bytes);

#define TGA_HEADER_SIZE         18
#define TGA_CMAP_SIZE(tga)      ((tga)->hdr.map_len * (tga)->hdr.map_entry / 8)
#define TGA_CMAP_OFF(tga) 	(TGA_HEADER_SIZE + (tga)->hdr.id_len)
#define TGA_IMG_DATA_OFF(tga) 	(TGA_HEADER_SIZE + (tga)->hdr.id_len + TGA_CMAP_SIZE(tga))
#define TGA_IMG_DATA_SIZE(tga)	((tga)->hdr.width * (tga)->hdr.height * (tga)->hdr.depth / 8)
#define TGA_SCANLINE_SIZE(tga)	((tga)->hdr.width * (tga)->hdr.depth / 8)
#define TGA_CAN_SWAP(depth)     (depth == 24 || depth == 32)

#define TGA_IS_MAPPED(tga)      ((tga)->hdr.map_t == 1)
#define TGA_IS_ENCODED(tga)     ((tga)->hdr.img_t > 8 && (tga)->hdr.img_t < 12)

const char *__TGAStrError(tuint8 code);

#define TGA_ERROR(tga, code) \
if((tga) && (tga)->error) (tga)->error(tga, code);\
fprintf(stderr, "Libtga:%s:%d: %s\n", __FILE__, __LINE__, TGAStrErrorCode(code));\
if(tga) (tga)->last = code\

#endif /* TGA_PRIVATE_H */