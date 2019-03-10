#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <png.h>

#include "png_assist.h"

//Append the file with this (128 bytes)
uint8_t pcx_header[] =
{
10, 5, 1, 8, 0, 0, 0, 0, 127, 2, 31, 3, 44, 1, 44, 1,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 1, 128, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

//(1 Bytes)			manufacturer "10"
//(1 Bytes)			encoder version "5"
//(1 Bytes)			encoding code "1"
//(1 Bytes)			bpp "8"
//(2 Bytes)			xmin "0"
//(2 Bytes)			ymin "0"
//(2 Bytes)			xmax "639" (00000010 01111111 = 639) Note its zero indexed
//(2 Bytes)			ymax "799" (00000011 00011111)
//(2 Bytes)			hres "300" (1 00101100)
//(2 Bytes)			vres "300"
//(1 Bytes * 48)	palette
//(1 Bytes)			(RESERVED)
//(1 Bytes)			clrplanes "1"
//(2 Bytes)			bytes per line "640" (10 10000000)
//(2 Bytes)			grey or colour palette flag "256" (1 00000000)
//(1 Bytes * 58)	filler

size_t crayon_assist_read_file(void ** buffer, char * path, size_t size_bytes, uint8_t allocated){
	if(!allocated){
		*buffer = malloc(size_bytes);
	}
	FILE * file = fopen(path, "rb");
	if(!file){return 1;}
	if(!size_bytes){	//Passing zero will instead set it to the length of the file
		fseek(file, 0, SEEK_END);
		size_bytes = ftell(file);
		fseek(file, 0, SEEK_SET);
	}
	size_t res = fread(*buffer, size_bytes, 1, file);
	fclose(file);
	if(res != 1){
		return 0;
	}
	return size_bytes;
}

//Don't forget to use this when MAKING the new map format

// uint32_t get_twiddled_index(uint16_t w, uint16_t h, uint32_t p){
// 	uint32_t ddx = 1, ddy = w;
// 	uint32_t q = 0;

// 	for(int i = 0; i < 16; i++){
// 		if(h >>= 1){
// 			if(p & 1){q |= ddy;}
// 			p >>= 1;
// 		}
// 		ddy <<= 1;
// 		if(w >>= 1){
// 			if(p & 1){q |= ddx;}
// 			p >>= 1;
// 		}
// 		ddx <<= 1;
// 	}

// 	return q;
// }

// 	// for(int i = 0; i < 16 * 16; i++){
// 	// 	printf("%d,	", get_twiddled_index(16, 16, i));
// 	// 	if(i % 16 == 15){
// 	// 		printf("\n");
// 	// 	}
// 	// }

//Used for the preview function
uint8_t PAL8BPP_to_png_details(uint8_t * pixel_data, uint32_t * palette, uint16_t width, int height, png_details_t * p_det){
	p_det->height = height;
	p_det->width = width;

	p_det->color_type = PNG_COLOR_MASK_COLOR + PNG_COLOR_MASK_ALPHA;	//= 2 + 4 = 6. They describe the color_type field in png_info
	p_det->bit_depth = 8;	//rgba8888, can be 1, 2, 4, 8, or 16 bits/channel (from IHDR)

	//Allocate space
	p_det->row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * p_det->height);
	if(!p_det->row_pointers){printf("Ran out of memory. Terminating now\n"); return 1;}
	for(int y = 0; y < p_det->height; y++){
		p_det->row_pointers[y] = (png_byte*)malloc(sizeof(png_byte) * p_det->width * 4);
		if(!p_det->row_pointers[y]){
			for(int i = 0; i < y; i++){	//Free the rest of the array
				free(p_det->row_pointers[i]);
			}
			free(p_det->row_pointers);
			printf("Ran out of memory. Terminating now\n");
			return 1;
		}
	}

	for(int y = 0; y < p_det->height; y++){
		for(int x = 0; x < p_det->width; x++){
			png_bytep px = &(p_det->row_pointers[y][x * 4]);

			int element = (y * width) + x;
			px[0] = bit_extracted(palette[pixel_data[element]], 8, 24);
			px[1] = bit_extracted(palette[pixel_data[element]], 8, 16);
			px[2] = bit_extracted(palette[pixel_data[element]], 8, 8);
			px[3] = bit_extracted(palette[pixel_data[element]], 8, 0);
		}
	}
	return 0;
}

int old_main(int argC, char ** argV){
	if(argC != 3){
		printf("Not enough arguments, please provide a .LEV file, and new .pcx filename\n");
		return 1;
	}
	printf("%s, %s\n", argV[1], argV[2]);
	uint8_t * buffer = malloc(sizeof(uint8_t) * 1024 * 1024);	//1MB buffer

	for(int i = 0; i < 128; i++){
		buffer[i] = pcx_header[i];
	}

	FILE * file_r = fopen(argV[1], "rb");
	if(!file_r){free(buffer); printf("Error1\n"); return 1;}

	//Get size of level
	fseek(file_r, 0, SEEK_END);
	size_t size_bytes = ftell(file_r);
	fseek(file_r, 0, SEEK_SET);

	//The .LEV files have a 128 byte header too, although not much is used
	size_t offset = 128;
	size_bytes -= offset;
	fseek(file_r, offset, SEEK_SET);

	size_t res = fread(buffer + offset, size_bytes, 1, file_r);
	fclose(file_r);
	if(res != 1){free(buffer); printf("Error2\n"); return 1;}
	

	FILE * file_w = fopen(argV[2], "wb");
	if(!file_w){free(buffer); printf("Error3\n"); return 1;}
	res = fwrite(buffer, sizeof(uint8_t), offset + size_bytes, file_w);
	fclose(file_w);
	if(res != offset + size_bytes){free(buffer); printf("Error4\n"); return 1;}

	printf("%lu\n", offset + size_bytes);
	free(buffer);
	return 0;
}

int main(int argC, char ** argV){
	if(argC != 3){
		printf("Not enough arguments, please provide a .LEV file, and new .png filename\n");
		return 1;
	}

	uint16_t width = 640;
	uint16_t height = 800;
	uint32_t num_bytes = width * height;
	uint8_t * pixel_data = malloc(num_bytes);	//Times 2? Shouldn't that be divided by 2?
	//note pixel_data WAS 16-bits long, but for PAL8BPP we don't need all that

	uint32_t * palette = malloc(sizeof(uint32_t) * 256);	//NOTE. The maps are in RGBA888 format, we want to convert to full alpha RGBA8888

	//Code lifted and modified from KOS' libpcx port
		//https://github.com/losinggeneration/kos-ports/blob/master/libpcx/pcx.c
	int runlen;
	uint8_t current_byte;
	uint32_t bytes_read = 0;
	FILE * file_r = fopen(argV[1], "rb");
	fseek(file_r, 128, SEEK_SET);	//Skip the header
	do{
		fread(&current_byte, 1, 1, file_r);

		if((current_byte & 0xc0) == 0xc0){	//high 2 bits set is packet (0xc0 == 192 == 11000000)
			runlen = (current_byte & 0x3f);	//AND off the high bits (0x3f == 63)
			fread(&current_byte, 1, 1, file_r);
			while(runlen--){
				pixel_data[bytes_read++] = current_byte;
			}
		}
		else{
			pixel_data[bytes_read++] = current_byte;
		}
	}while(bytes_read < num_bytes);

	long int length1 = ftell(file_r);
	fseek(file_r, 0, SEEK_END);
	long int length2 = ftell(file_r);
	fseek(file_r, length1, SEEK_SET);

	fread(&current_byte, 1, 1, file_r);	//There's a marker byte after the image and before the palette. Its value is meaningless
	int i;
	for(i = 0; i < 256; i++){
		palette[i] = 255;
		fread(&current_byte, 1, 1, file_r);
		palette[i] += current_byte << 24;
		fread(&current_byte, 1, 1, file_r);
		palette[i] += current_byte << 16;
		fread(&current_byte, 1, 1, file_r);
		palette[i] += current_byte << 8;
	}

	png_details_t p_det;
	PAL8BPP_to_png_details(pixel_data, palette, 640, 800, &p_det);
	write_png_file(argV[2], &p_det);

	free(pixel_data);
	free(palette);

	return 0;
}

