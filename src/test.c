#include <stdio.h>
#include <stdlib.h>
#include "bmper.h"

int main(int argc, char** argv) {
	
	printf("dayam\n");
	
	BMP24File* bmp = init_BMP24File(alloc_BMP24File(),4,5);
	
	printf("confirm union: %x=%x\n",bmp->pixels,bmp->bytes);
	
	BMPFileHeader a,b;
	BMPFileHeader r[] = {a,b};
	printf("%x, %x, %x\n",&a,&a.off_bits,&b);
	printf("%i\n%i\n%i\n%i\n%i\n",
		&a.type,&a.size,&a.reserved1,&a.reserved2,&a.off_bits);
	
	printf("sizes: %i, %i, %i, %i\n",
		sizeof(pix24*),sizeof(int32_t*),sizeof(byte*),sizeof(void*));
	pix24 p[2];
	printf("p sizes: %i, %i, %i\n",
		sizeof(pix24),sizeof(pix32),sizeof(p));
	
	char buf[2048];
	p[0] = pix24_rgb(63,127,255);
	p[1] = pix24_rgb(100,10,1);
	printf("These two look like: %s, %s\n",
		str_pix24(p[0],buf),
		str_pix24(p[1],&buf[100]));
	
	int nr[] = {0x010203,0xffcc33,-1};
	pix24 psh = pix24_int(0x00030507);
	pix24_int_narr(p,nr);
	printf("Do the ints: %s,%s,%s\n",
		str_pix24(psh,buf),
		str_pix24(p[0],&buf[100]),
		str_pix24(p[1],&buf[200]));
	
	int i;
	for (i=0; i < 24; ++i)
		printf("(%i: %i) ",i,true_row_width(i));
	printf("\n");
	
	bmp->pr[0][0] = pix24_int(0x0000ff);
	bmp->pixels[0] = pix24_int(0xff0000);
	
	FILE* f = fopen("dasbmp.bmp","wb");
	BMP24File_write(bmp,f);
	fclose(f);

	del_BMP24File(bmp,DEL_STRUCT);

	return 0;
}

