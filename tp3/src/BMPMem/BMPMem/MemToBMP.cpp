#include <inttypes.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "ap_bmp.h"

int main()
{
	const char * inputRaw = "DataRAM";
	const char * outputImage = "output.bmp";

	std::ifstream inFile;
	inFile.open(inputRaw, std::ios::in | std::ios::binary);
	int32_t imgWidth, imgHeight;
	inFile.read((char*)&imgWidth, sizeof(imgWidth));
	inFile.read((char*)&imgHeight, sizeof(imgHeight));
	int imgSize = imgWidth * imgHeight;
	uint8_t * raw = new uint8_t[imgSize];
	inFile.read((char*)raw, imgSize);
	inFile.close();

	//Write the image back to disk
	int write_tmp = BMP_Write(outputImage, imgHeight, imgWidth, raw, raw, raw);
	if (write_tmp != 0) {
		std::cerr << "WriteBMP " << outputImage << " failed" << std::endl;
		return EXIT_FAILURE;
	}

	delete raw;

	return EXIT_SUCCESS;
}