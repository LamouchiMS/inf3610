///////////////////////////////////////////////////////////////////////////////
//
//	Sobel.cpp
//
///////////////////////////////////////////////////////////////////////////////
#include "Sobel.h"


///////////////////////////////////////////////////////////////////////////////
//
//	Constructeur
//
///////////////////////////////////////////////////////////////////////////////
Sobel::Sobel( sc_module_name name ) : sc_module(name)
{
	SC_THREAD(thread);
}


///////////////////////////////////////////////////////////////////////////////
//
//	Destructeur
//
///////////////////////////////////////////////////////////////////////////////
Sobel::~Sobel()
{
}

void Sobel::read(unsigned int add, int& output) {
	address.write(add);
	requestRead.write(true);
	do {
		wait(clk->posedge_event());
	} while (!ack.read());

	output = data.read();
	requestRead.write(false);
	wait(clk->posedge_event());
}

void Sobel::write(unsigned int add, unsigned int dataW) {
	address.write(add);
	data.write(dataW);
	requestWrite.write(true);
	do {
		wait(clk->posedge_event());
	} while (!ack.read());

	requestWrite.write(false);
	wait(clk->posedge_event());
}


///////////////////////////////////////////////////////////////////////////////
//
//	thread
//
///////////////////////////////////////////////////////////////////////////////
void Sobel::thread(void)
{

	//Obtenir la longueur et la largeur
	int width, length;
	read(0, width);
	read(4, length);
	
	int size = width * length;
	uint8_t* image = (uint8_t*)malloc(size * sizeof(uint8_t));
	uint8_t* res = (uint8_t*)malloc(size * sizeof(uint8_t));

	for (int i = 8; i < size + 8; i += 4) {
		int pixels;
		read(i, pixels);

		image[i - 8] = pixels >> 24;
		image[i - 7] = (pixels << 8) >> 24;
		image[i - 6] = (pixels << 16) >> 24;
		image[i - 5] = (pixels << 24) >> 24;
	}

	for (int i = width; i < size - width; i++) {
		if (!(i % width == 0 || (i % width) == width - 1)) {
			res[i] = sobel_operator(i, width, image);
		}
		else {
			res[i] = 0;
		}
		wait(clk->posedge_event());
	}

	for (int i = 0; i < width; i++) {
		res[i] = 0;
		res[i + size - width] = 0;
		wait(clk->posedge_event());
	}

	for (int i = 0; i < size; i += 4) {
		unsigned int data = (res[i] << 24) | (res[i + 1] << 16) | (res[i + 2] << 8) | res[i + 3];
		write(i + 8, data);
	}
	delete image;
	delete res;

	sc_stop();
	wait();

}


///////////////////////////////////////////////////////////////////////////////
//
//	sobel_operator
//
///////////////////////////////////////////////////////////////////////////////
static inline uint8_t getVal(int index, int xDiff, int yDiff, int img_width, uint8_t * Y) 
{ 
	return Y[index + (yDiff * img_width) + xDiff]; 
};

uint8_t Sobel::sobel_operator(const int index, const int imgWidth, uint8_t * image)
{
	int x_weight = 0;
	int y_weight = 0;

	unsigned edge_weight;
	uint8_t edge_val;

	const char x_op[3][3] = {	{ -1,0,1 },
								{ -2,0,2 },
								{ -1,0,1 } };

	const char y_op[3][3] = {	{ 1,2,1 },
								{ 0,0,0 },
								{ -1,-2,-1 } };

	//Compute approximation of the gradients in the X-Y direction
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
		// X direction gradient
		x_weight = x_weight + (getVal(index, i - 1, j - 1, imgWidth, image) * x_op[i][j]);

		// Y direction gradient
		y_weight = y_weight + (getVal(index, i - 1, j - 1, imgWidth, image) * y_op[i][j]);
		}
	}

	edge_weight = std::abs(x_weight) + std::abs(y_weight);

	edge_val = (255 - (uint8_t)(edge_weight));

	//Edge thresholding
	if (edge_val > 200)
		edge_val = 255;
	else if (edge_val < 100)
		edge_val = 0;

	return edge_val;
}


