#ifndef bmper_h
#define bmper_h

/**
 * bmper.h:
 * - following Windows DIB header BITMAPINFOHEADER
 */

#include <stdint.h>
#include <string.h>

#ifndef DEL_NONE
#define DEL_NONE   0x1
#define DEL_STRUCT 0x2
#define DEL_DATA   0x4
#define DEL_KEYS   0x8
#define DEL_VALS   0x16
#endif

#ifndef byte
#define byte unsigned char
#endif

// { File headers
typedef struct {
	int16_t type;
	int32_t size;
	int16_t reserved1;
	int16_t reserved2;
	int32_t off_bits;
} BMPFileHeader;
BMPFileHeader* init_BMPFileHeader(BMPFileHeader* h) {
	h->type = 19778;
	h->reserved1 = 0;
	h->reserved2 = 0;
	h->off_bits = 1078;
}
byte* BMPFileHeader_buffer(BMPFileHeader* h, byte* b) {
	// ASSUME: len(b) >= 14
	int16_t* p2;
	int32_t* p4;
	p2 = (int16_t*)&b[0];
	*p2 = h->type;
	p4 = (int32_t*)&b[2];
	*p4 = h->size;
	p2 = (int16_t*)&b[6];
	*p2 = h->reserved1;
	p2++;
	*p2 = h->reserved2;
	p4 = (int32_t*)&b[10];
	*p4 = h->off_bits;
	return b;
}

typedef struct {
	int32_t size;
	int32_t width;
	int32_t height;
	int16_t planes;
	int16_t bit_count;
	int32_t compression;
	int32_t size_image;
	int32_t x_pix_meter;
	int32_t y_pix_meter;
	int32_t clr_used;
	int32_t clr_important;
} BMPInfoHeader;
BMPInfoHeader* init_BMPInfoHeader(BMPInfoHeader* h) {
	h->size = 40;
	h->width = 100;
	h->height = 100;
	h->planes = 1; // should this be 0?
	h->bit_count = 8;
	h->compression = 0;
	h->size_image = 0;
	h->x_pix_meter = 2835; // or 0?
	h->y_pix_meter = 2835;
	h->clr_used = 0;
	h->clr_important = 0;
}
// }

// { Pixels
typedef struct {
	byte blue;
	byte green;
	byte red;
	byte reserved;
} pix32;
pix32* init_pix32(pix32* p) {
	p->reserved = 0;
}

#define def_pix32(r,g,b) {b,g,r,0}
pix32 PIX32_BLACK = def_pix32(0,0,0);

typedef struct {
	byte blue;
	byte green;
	byte red;
} pix24;

// UNCERTAIN: Should I keep pix24*, the norm for my functions, or go without the
//   pointer, since sizeof(pix24) is 3, less than 8 for a pointer proper?
char* str_pix24(pix24 p, char* buf) {
	sprintf(buf,"(r:%i,g:%i,b:%i)",p.red,p.green,p.blue);
	return buf;
}

#define def_pix24(r,g,b) {b,g,r}
pix24 PIX24_BLACK = def_pix24(0,0,0);

pix24 pix24_rgb(byte r, byte g, byte b) {
	pix24 p = def_pix24(r,g,b);
	return p;
}

pix24* pix24_pix_arr(pix24* pr, pix24* r2, int count) {
	int i;
	for (i=0; i < count; ++i)
		pr[i] = r2[i];
	return pr;
}

pix24 pix24_int(int32_t i) {
	return *(pix24*)&i;
}
pix24* pix24_int_arr(pix24* pr, int32_t* r, int count) {
	int i;
	for (i=0; i < count; ++i)
		pr[i] = *(pix24*)&r[i];
	return pr;
}
pix24* pix24_int_narr(pix24* pr, int32_t* nr) {
	// negative-ending array
	while (*nr >= 0) {
		*pr = *(pix24*)nr;
		nr++;
		pr++;
	}
	return pr;
}

// }


typedef struct {
	BMPFileHeader file;
	BMPInfoHeader info;
	union {
		pix24* pixels;
		byte* bytes;
	};
	int32_t bsize;
	union {
		pix24** pr;
		byte** br;
	};
} BMP24File;

#define true_row_width(w) ((((int)(w)-1)/4+1)*4)

BMP24File* alloc_BMP24File() {
	return (BMP24File*)malloc(sizeof(BMP24File));
}
BMP24File* init_BMP24File(BMP24File* bmp, int w, int h) {
	init_BMPFileHeader(&(bmp->file));
	init_BMPInfoHeader(&(bmp->info));
	bmp->info.bit_count = 24;
	bmp->info.width = w;
	bmp->info.height = h;
	bmp->bsize = true_row_width(bmp->info.width*3) * bmp->info.height;
	bmp->file.size = 14 + 40 + bmp->bsize;
	bmp->file.off_bits = 14+40;
	bmp->info.size_image = bmp->bsize;
	
	bmp->pixels = (pix24*)malloc(bmp->bsize);
	//memset(bmp->pixels,0,bmp->bsize);
	bmp->pr = (pix24**)malloc(sizeof(pix24*)*bmp->info.height);
	int i;
	for (i=0; i < bmp->info.height; ++i)
		bmp->pr[i] = &(bmp->pixels[(h-i-1)*w]);
	
	return bmp;
}
void del_BMP24File(BMP24File* bmp, byte del_flags) {
	if (bmp->pixels)
		free(bmp->pixels);
	if (bmp->pr)
		free(bmp->pr);
	if (del_flags & DEL_STRUCT)
		free(bmp);
}

void BMP24File_fill(BMP24File* bmp, pix24 p) {
	int i,j;
	for (i=0; i < bmp->info.height; ++i)
		for (j=0; j < bmp->info.width; ++j)
			bmp->pr[i][j] = p;
}

FILE* BMP24File_write(BMP24File* bmp, FILE* f) {
	// NOTE: The compiler held the BMPFileHeader struct at an even 16 bytes,
	//   despite only needing 14, by spacing .type and .size oddly.  This prompted
	//   the creation of a buffer function to ensure accurate spacing.
	//   BMPInfoHeader, on the other hand, seems not to have this problem.
	byte buf[14];
	BMPFileHeader_buffer(&(bmp->file),buf);
	
	fwrite(&buf,14,1,f);
	fwrite(&(bmp->info.size),40,1,f);
	fwrite(bmp->bytes,1,bmp->bsize,f);
	return f;
}

#endif


