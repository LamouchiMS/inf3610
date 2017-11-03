///////////////////////////////////////////////////////////////////////////////
//
//	Sobelv2.cpp
//
///////////////////////////////////////////////////////////////////////////////
#include "Sobelv2.h"

///////////////////////////////////////////////////////////////////////////////
//
//	Constructeur
//
///////////////////////////////////////////////////////////////////////////////
Sobelv2::Sobelv2(sc_module_name name) : sc_module(name)
{
	SC_THREAD(thread);
}

///////////////////////////////////////////////////////////////////////////////
//
//	Destructeur
//
///////////////////////////////////////////////////////////////////////////////
Sobelv2::~Sobelv2()
{
	/*
	
	� compl�ter
	
	*/
}

///////////////////////////////////////////////////////////////////////////////
//
//	thread
//
///////////////////////////////////////////////////////////////////////////////
void Sobelv2::thread(void)
{

	//Obtenir la longueur et la largeur
	int width, length;
	read(0, width);
	read(4, length);

	int size = width * length;
	uint8_t *res = (uint8_t *)malloc(size * sizeof(uint8_t));

	// Assignation des contours a 0
	for (int i = width; i < size - width; i++)
	{
		if (i % width == 0 || (i % width) == width - 1)
		{
			res[i] = 0;
		}
		wait(clk->posedge_event());
	}
	for (int i = 0; i < width; i++)
	{
		res[i] = 0;
		res[i + size - width] = 0;
		wait(clk->posedge_event());
	}

	// Lire l'image
	unsigned int len = 3;
	for (int i = width + 8; i < size - width + 8; i += width)
	{
		unsigned int *addr = (unsigned int *)i;
		readFromCache(addr, len);
		int pixels = *addr;

		for (int j = i + 1; j < i + i * width - 1; j++)
		{
			res[j] = sobel_operator(j, width, addr);
		}
		wait(clk->posedge_event());
	}

	// Ecriture de l'image filtree sur Writer
	for (int i = 0; i < size; i += 4)
	{
		unsigned int data = (res[i] << 24) | (res[i + 1] << 16) | (res[i + 2] << 8) | res[i + 3];
		write(i + 8, data);
	}

	delete image;
	delete res;

	sc_stop();
	wait();
}

void Sobelv2::readFromCache(unsigned int &addr, unsigned int len)
{
	addressRes.write(addr);
	length.write(len);
	requestCache.write(true);
	do
	{
		wait(clk->posedge_event());
	} while (!ackCache.read());

	wait(clk->posedge_event());
}

void Sobelv2::read(unsigned int add, int &output)
{
	address.write(add);
	requestRead.write(true);
	do
	{
		wait(clk->posedge_event());
	} while (!ackReaderWriter.read());

	output = dataRW.read();
	requestRead.write(false);
	wait(clk->posedge_event());
}

void Sobelv2::write(unsigned int add, unsigned int dataW)
{
	address.write(add);
	dataRW.write(dataW);
	requestWrite.write(true);
	do
	{
		wait(clk->posedge_event());
	} while (!ackReaderWriter.read());

	requestWrite.write(false);
	wait(clk->posedge_event());
}

///////////////////////////////////////////////////////////////////////////////
//
//	Sobelv2_operator
//
///////////////////////////////////////////////////////////////////////////////
static inline uint8_t getVal(int index, int xDiff, int yDiff, int img_width, uint8_t *Y)
{
	return Y[index + (yDiff * img_width) + xDiff];
};

uint8_t Sobelv2::Sobelv2_operator(const int index, const int imgWidth, uint8_t *image)
{
	int x_weight = 0;
	int y_weight = 0;

	unsigned edge_weight;
	uint8_t edge_val;

	const char x_op[3][3] = {{-1, 0, 1},
													 {-2, 0, 2},
													 {-1, 0, 1}};

	const char y_op[3][3] = {{1, 2, 1},
													 {0, 0, 0},
													 {-1, -2, -1}};

	//Compute approximation of the gradients in the X-Y direction
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
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
