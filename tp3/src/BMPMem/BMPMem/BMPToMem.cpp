#include <inttypes.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "ap_bmp.h"

const int32_t IMG_WIDTH = 1920;
const int32_t IMG_HEIGHT = 1080;
const int IMG_SIZE = IMG_WIDTH * IMG_HEIGHT;

// RGB to Y Conversion
// Resulting luminance value used in edge detection
static inline uint8_t rgb2y(uint8_t R, uint8_t G, uint8_t B)
{
	return ((66 * R + 129 * G + 25 * B + 128) >> 8) + 16;
}

int main()
{
	const char * inputImage = "image.bmp";
	const char * rawOutput = "image.mem";

	uint8_t *R = new uint8_t[IMG_SIZE];
	uint8_t *G = new uint8_t[IMG_SIZE];
	uint8_t *B = new uint8_t[IMG_SIZE];
	uint8_t *Y = new uint8_t[IMG_SIZE];

	// Fill a frame with data
	int read_tmp = BMP_Read(inputImage, IMG_HEIGHT, IMG_WIDTH, R, G, B);
	if (read_tmp != 0) {
		std::cerr << "Loading image failed" << std::endl;
		return EXIT_FAILURE;
	}

	for (int i = 0; i < IMG_SIZE; ++i)
		Y[i] = rgb2y(R[i], G[i], B[i]);

	std::ofstream outFile;
	outFile.open(rawOutput, std::ios::out | std::ios::binary);

	outFile.write((char*)&IMG_WIDTH, sizeof(IMG_WIDTH));
	outFile.write((char*)&IMG_HEIGHT, sizeof(IMG_HEIGHT));
	outFile.write((char*)Y, IMG_SIZE);
	outFile.close();

	delete R, G, B, Y;
	
	return EXIT_SUCCESS;
}