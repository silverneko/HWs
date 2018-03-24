/*** The programe reads out image data from image file and write into it ***/

// here I used color scale image(RGB) "image.raw" as an example.

// it will read the image data to image_data array

// it also write data in image_data array to image file "image1.raw" 


#include "stdio.h"

#include "iostream"

#include "stdlib.h"

// define image size 256*256

// you can change it to other size

#define SIZE 256
using namespace std;

int main(void)
{
	// file pointer
	FILE *file;

	// read image "XXX.raw" into image data matrix
	unsigned char image_data[3][SIZE][SIZE];

	if (!(file = fopen("image.raw", "rb")))
	{
		printf("Cannot open file!\n");

		exit(1);
	}

	fread(image_data, sizeof(unsigned char), SIZE* SIZE * 3, file);

	fclose(file);

	// do some image processing task...


	// write image data to "image1.raw"

	if (!(file = fopen("image1.raw", "wb")))

	{

		printf("Cannot open file!\n");

		exit(1);

	}

	fwrite(image_data, sizeof(unsigned char), SIZE* SIZE * 3, file);

	fclose(file);

	exit(0);
}










