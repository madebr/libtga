#include <tga.h>

#include <errno.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <strings.h>

#define OK      1
#define NOK     0

int global_ok;

void v_check_message(int ok, const char *format, va_list ap) {
	char message[128];
	vsnprintf(message, sizeof(message) - 1, format, ap);
	message[sizeof(message)-1] = 0;
	if (errno) {
		printf("[ ] %s (errno=%d)\n", strerror(errno), errno);
		errno = 0;
	}
	printf("[%c] %s", ok ? 'x' : ' ', message);
	if (!ok) {
		global_ok = 0;
	}
}

void check_message(int ok, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	v_check_message(ok, format, ap);
	va_end(ap);
}

void check_message_tga(TGA *tga, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	v_check_message(TGA_SUCCEEDED(tga), format, ap);
	va_end(ap);
	TGAClearError(tga);
}

void check_message_fail(int ok, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	int real_ok = ok && !errno;
	if (errno) {
		printf("[ ] %s (errno=%d)\n", strerror(errno), errno);
		errno = 0;
	}
	v_check_message(ok, format, ap);
	va_end(ap);
	if (!real_ok) {
		printf("Fatal error\n");
		exit(1);
	}
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s INPUT\n", argv[0]);
		return 1;
	}
	global_ok = OK;

	printf("Opening file...\n");
	TGA *tga = TGAOpen(argv[1], "r");
	check_message_tga(tga, "TGAOpen(\"%s\")\n", argv[1]);
	check_message_fail(TGA_SUCCEEDED(tga), "Open \"%s\"\n", argv[1]);

	const int hasid = TGA_HAS_ID(tga);
	printf("Reading header:\n");
	TGAReadHeader(tga);
	check_message_tga(tga, "TGAReadHeader()\n");

	check_message(OK, "ID section length = %u\n", tga->hdr.id_len);

#	define TGA_IMGTYPE_KNOWN(tga) (0 \
		|| ((tga)->hdr.img_t == TGA_IMGTYPE_NOIMAGE) \
		|| ((tga)->hdr.img_t == TGA_IMGTYPE_UNCOMP_CMAP) \
		|| ((tga)->hdr.img_t == TGA_IMGTYPE_UNCOMP_TRUEC) \
		|| ((tga)->hdr.img_t == TGA_IMGTYPE_UNCOMP_BW) \
		|| ((tga)->hdr.img_t == TGA_IMGTYPE_RLE_CMAP) \
		|| ((tga)->hdr.img_t == TGA_IMGTYPE_RLE_TRUEC) \
		|| ((tga)->hdr.img_t == TGA_IMGTYPE_RLE_BW) \
		)

	const int mapped = TGA_IS_MAPPED(tga);
	printf("Checking color map data in header...\n");
	check_message(OK, "tga->hdr.map_t = 0x%x (%scolormapped)\n",
		tga->hdr.map_t,
		mapped ? "" : "not ");
	check_message(mapped ? tga->hdr.map_len > 0 : OK,
		"tga->hdr.map_len = %u\n", tga->hdr.map_len);
	check_message(mapped ? tga->hdr.map_first < tga->hdr.map_len : OK,
		"tga->hdr.map_first = %u\n", tga->hdr.map_first);
	check_message(mapped ? TGA_IMGTYPE_IS_MAPPED(tga) : OK,
		"tga->hdr.img_t = 0x%x (colormapped = 0x%x flag)\n",
		tga->hdr.img_t, TGA_IMGTYPE_CMAP_FLAG);
	const tbyte map_depth = tga->hdr.map_entry;
#	define MAP_DEPTH_OK(D) ((D == 15) || (D == 16) \
		|| (D == 24) || (D == 32))
	check_message(mapped ? MAP_DEPTH_OK(map_depth) : OK,
		"Colormap depth = %u bit\n", map_depth);

	const int hasimg = TGA_IMGTYPE_AVAILABLE(tga);
	const tbyte depth = tga->hdr.depth;
	printf("Checking image data in header...\n");
	check_message(OK, "tga->hdr.img_t = 0x%x (image %savailable)\n",
		tga->hdr.img_t,
		hasimg ? "" : "not ");
	check_message(TGA_IMGTYPE_KNOWN(tga),
		"tga->hdr.img_t = 0x%x (image type %sknown)\n",
		tga->hdr.img_t, TGA_IMGTYPE_KNOWN(tga) ? "" : "not ");
#	define IMG_DEPTH_OK(D) ((D == 8) || (D == 16) || \
		(D == 24) || (D == 32))
	check_message(hasimg ? IMG_DEPTH_OK(depth) : OK,
		"Image depth = %u bit\n", depth);
	check_message((tga->hdr.alpha & 0xf) == tga->hdr.alpha,
		"tga->hdr.alpha = %u (attribute bits per pixel)\n",
		tga->hdr.alpha);
	check_message((tga->hdr.horz & 0x1) == tga->hdr.horz,
		"tga->hdr.horz = %u (%s)\n", tga->hdr.horz,
		tga->hdr.horz == TGA_LEFT ? "left-to-right" : "right-to-left");
	check_message((tga->hdr.vert & 0x1) == tga->hdr.vert,
		"tga->hdr.vert = %u (%s)\n", tga->hdr.vert,
		tga->hdr.horz == TGA_BOTTOM ? "bottom-to-top" : "top-to-bottom");
	check_message((tga->hdr.width > 0) ? hasimg : OK,
		"tga->hdr.width = width = %u\n", tga->hdr.width);
	check_message((tga->hdr.height > 0) ? hasimg : OK,
		"tga->hdr.height = %u\n", tga->hdr.height);

	printf("Checking ID data in header...\n");
	check_message(OK, "tga->hdr.id_len = %u\n", tga->hdr.id_len);

	TGAData data;
	bzero(&data, sizeof(data));

	printf("Reading ID header...\n");
	TGAReadImageId(tga, &data);
	check_message_tga(tga, "TGAReadImageId()\n");
	check_message(hasid ? data.img_id != 0 : OK, "data.img_id != NULL\n");

	printf("Reading colormap...\n");
	TGAReadColorMap(tga, &data);
	check_message_tga(tga, "TGAReadColorMap()\n");

	printf("Reading image data...\n");
	TGAReadScanlines(tga, &data);
	check_message_tga(tga, "TGAReadScanlines\n");

	printf("Freeing TGAData...\n");
	TGAFreeTGAData(&data);
	check_message(OK, "TGAFreeTGAData()\n");

	printf("Closing tga...\n");
	TGAClose(tga);
	check_message(OK, "TGAClose()\n");

	printf("Calculating size of \"%s\"...\n", argv[1]);
	FILE *f = fopen(argv[1], "r");
	check_message_fail(f != NULL, "fopen(\"%s\")\n", argv[1]);
	int result = fseek(f, 0, SEEK_END);
	check_message(result == 0, "fseek()\n");
	const long filesize = ftell(f);
	check_message(filesize > 0, "ftell()\n");
	result = fclose(f);
	check_message(result == 0, "fclose()\n");
	printf("size = %ld bytes\n", filesize);

	if (global_ok) {
		printf("Everything ok!\n");
		return 0;
	} else {
		printf("Some check(s) failed!\n");
		return 1;
	}
}
