/*
------------------DR VASILIOS KELEFOURAS-----------------------------------------------------
------------------COMP1001 ------------------------------------------------------------------
------------------COMPUTER SYSTEMS MODULE-------------------------------------------------
------------------UNIVERSITY OF PLYMOUTH, SCHOOL OF ENGINEERING, COMPUTING AND MATHEMATICS---
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <emmintrin.h>
#include <limits.h>
#include <pmmintrin.h>
#include <immintrin.h>

//function declarations
void Gaussian_Blur();
void Sobel();
int initialize_kernel();
void read_image(const char* filename);
void read_image_and_put_zeros_around(char* filename);
void write_image2(const char* filename, unsigned char* output_image);
void openfile(const char* filename, FILE** finput);
int getint(FILE* fp);
void allocateImageArrays(unsigned char** frame1, unsigned char** filt, unsigned char** gradient, int M, int N);
void Sobel_SSE();

//CRITICAL POINT: images' paths - You need to change these paths
#define IN "C:\\Users\\mohd2\\Documents\\Comp1001\\github_fork\\COMP1001\\COMP1001-master\\23_24_coursework\\Report\\question3\\VS\\code_to_start\\input_images\\a0.pgm"
#define OUT "C:\\Users\\mohd2\\Documents\\Comp1001\\github_fork\\COMP1001\\COMP1001-master\\23_24_coursework\\Report\\question3\\VS\\code_to_start\\output_images\\blurred.pgm"
#define OUT2 "C:\\Users\\mohd2\\Documents\\Comp1001\\github_fork\\COMP1001\\COMP1001-master\\23_24_coursework\\Report\\question3\\VS\\code_to_start\\output_images\\edge_detection.pgm"

//IMAGE DIMENSIONS
// #define M 512  //cols
// #define N 512  //rows

int M, N; // Variables to store image dimensions


//CRITICAL POINT:these arrays are defined statically. Consider creating these arrays dynamically instead.
//unsigned char frame1[N * M];//input image
//unsigned char filt[N * M];//output filtered image
//unsigned char gradient[N * M];//output image

unsigned char* frame1, * filt, * gradient;



const signed char Mask[5][5] = {//2d gaussian mask with integers
	{2,4,5,4,2} ,
	{4,9,12,9,4},
	{5,12,15,12,5},
	{4,9,12,9,4},
	{2,4,5,4,2}
};

const signed char GxMask[3][3] = {
	{-1,0,1} ,
	{-2,0,2},
	{-1,0,1}
};

const signed char GyMask[3][3] = {
	{-1,-2,-1} ,
	{0,0,0},
	{1,2,1}
};

char header[100];
errno_t err;


int main() {


		// Define the paths for input and output images
		char input[200];
		char path[200] = "C:\\Users\\mohd2\\Documents\\Comp1001\\github_fork\\COMP1001\\COMP1001-master\\23_24_coursework\\Report\\question3\\VS\\code_to_start\\input_images\\a";	// Base path for input images
		char pathOut1[200] = "C:\\Users\\mohd2\\Documents\\Comp1001\\github_fork\\COMP1001\\COMP1001-master\\23_24_coursework\\Report\\question3\\VS\\code_to_start\\output_images\\blurred";	// Base path for output blurred images
		char pathOut2[200] = "C:\\Users\\mohd2\\Documents\\Comp1001\\github_fork\\COMP1001\\COMP1001-master\\23_24_coursework\\Report\\question3\\VS\\code_to_start\\output_images\\edge_detection";	// Base path for output edge detected images
		
		size_t inputSize = sizeof(input);	// Size of the input buffer
		char format[5] = ".pgm";	// File format extension
		int counter = 0;	// Counter to iterate through images
		
		
		// Loop to process multiple images
		for (int i = 0; i <= 30; i++) {
			
			// Construct the full path for the current input image
			snprintf(input ,inputSize, "%s%d", path, counter);
			strcat_s(input, inputSize, format);	// Append file format to the input path

			read_image(input); // Read the current image

			Gaussian_Blur(); // Blur the image (reduce noise)
			//Sobel(); // Apply edge detection
			Sobel_SSE();

			// Construct file names for the output images
			char out1[200], out2[200];
			snprintf(out1, sizeof(out1), "%s%d%s", pathOut1, counter, format); // Construct output file name for blurred image
			snprintf(out2, sizeof(out2), "%s%d%s", pathOut2, counter, format); // Construct output file name for edge detection image

			write_image2(out1, filt); // Write the blurred image to disk
			write_image2(out2, gradient); // Write the edge detected image to disk
			counter++;// Increment counter for the next image
		}
	
	return 0;
}






void Gaussian_Blur() {

	int row, col, rowOffset, colOffset;
	int newPixel;
	unsigned char pix;
	//const unsigned short int size=filter_size/2;
	const unsigned short int size = 2;

	/*---------------------- Gaussian Blur ---------------------------------*/
	for (row = 0; row < N; row++) {
		for (col = 0; col < M; col++) {
			newPixel = 0;
			for (rowOffset = -size; rowOffset <= size; rowOffset++) {
				for (colOffset = -size; colOffset <= size; colOffset++) {

					if ((row + rowOffset < 0) || (row + rowOffset >= N) || (col + colOffset < 0) || (col + colOffset >= M))
						pix = 0;
					else
						pix = frame1[M * (row + rowOffset) + col + colOffset];

					newPixel += pix * Mask[size + rowOffset][size + colOffset];

				}
			}
			filt[M * row + col] = (unsigned char)(newPixel / 159);

		}
	}

}


/*void Sobel() {

	int row, col, rowOffset, colOffset;
	int Gx, Gy;

	
	for (row = 1; row < N - 1; row++) {
		for (col = 1; col < M - 1; col++) {

			Gx = 0;
			Gy = 0;

			

			for (rowOffset = -1; rowOffset <= 1; rowOffset++) {
				for (colOffset = -1; colOffset <= 1; colOffset++) {

					Gx += filt[M * (row + rowOffset) + col + colOffset] * GxMask[rowOffset + 1][colOffset + 1];
					Gy += filt[M * (row + rowOffset) + col + colOffset] * GyMask[rowOffset + 1][colOffset + 1];
				}
			}

			gradient[M * row + col] = (unsigned char)sqrt(Gx * Gx + Gy * Gy); /* Calculate gradient strength		
			//gradient[row][col] = abs(Gx) + abs(Gy); // this is an optimized version of the above

		}
	}


}
*/

void Sobel_SSE() {
	int row, col; // Variables to keep track of the current row and column in the image

	// Variables to hold the gradient in x and y directions for a block of pixels
	__m128i Gx, Gy;

	__m128i sum; // Variable to store the sum of gradients for a block of pixels

	// Masks for computing the x-gradient. These are like filters applied over the image data
	__m128i mask_x0, mask_x1, mask_x2;

	// Masks for computing the y-gradient. Similar to x-gradient masks, but for vertical edges
	__m128i mask_y0, mask_y1, mask_y2;

	// Set up the masks with specific values. These values are used in edge detection
	// _mm_setr_epi8 is a special function that helps set these values
	mask_x0 = _mm_setr_epi8(-1, 0, 1, -2, 0, 2, -1, 0, 1, 0, 0, 0, 0, 0, 0, 0);
	mask_x1 = _mm_setr_epi8(-2, 0, 2, -1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	mask_x2 = _mm_setr_epi8(-1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	mask_y0 = _mm_setr_epi8(-1, -2, -1, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0);
	mask_y1 = _mm_setr_epi8(0, 0, 0, -1, -2, -1, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0);
	mask_y2 = _mm_setr_epi8(1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	// Loop through each row of the image, but skip the first and last row
	for (row = 1; row < N - 1; row++) {
		// Loop through each column in steps of 16 (because we process 16 pixels at a time)
		for (col = 0; col <= M - 16; col += 16) {
			// Load data for the current and neighboring rows into special variables (data0, data1, data2)
		   // These variables can hold more data than usual because of SSE (SIMD)
			__m128i data0 = _mm_loadu_si128((__m128i*)(filt + M * (row - 1) + col));
			__m128i data1 = _mm_loadu_si128((__m128i*)(filt + M * row + col));
			__m128i data2 = _mm_loadu_si128((__m128i*)(filt + M * (row + 1) + col));

			// Compute the gradient in the x direction (Gx) by applying the masks to the image data
		   // This is like measuring how sharp the change is in the horizontal direction
			Gx = _mm_add_epi16(
				_mm_add_epi16(
					_mm_maddubs_epi16(data0, mask_x0),
					_mm_maddubs_epi16(data1, mask_x1)),
				_mm_maddubs_epi16(data2, mask_x2));

			// Compute the gradient in the y direction (Gy) by applying the masks to the image data
			Gy = _mm_add_epi16(
				_mm_add_epi16(
					_mm_maddubs_epi16(data0, mask_y0),
					_mm_maddubs_epi16(data1, mask_y1)),
				_mm_maddubs_epi16(data2, mask_y2));

			// Add up the absolute values of Gx and Gy to get the total gradient
			sum = _mm_add_epi16(_mm_abs_epi16(Gx), _mm_abs_epi16(Gy));

			// Scale down the sum to fit into an 8-bit number (like a regular grayscale pixel value)
			// This is because the gradient might be a large number, and we want to store it as a pixel value
			__m128i maxVal = _mm_set1_epi16(1448);
			// Approximate division by 1448 to scale the value
			sum = _mm_mulhrs_epi16(sum, _mm_set1_epi16((short)((1 << 15) / 1448)));

			// Store the scaled gradient sum in the gradient array for the image
			_mm_storeu_si128((__m128i*)(gradient + M * row + col), sum);

			// For any remaining pixels in the row that were not processed in the block of 16,
			// we calculate the gradient using a regular method (non-SSE)
			for (; col < M; col++) {
				int Gx_scalar = 0, Gy_scalar = 0;
				// Compute gradients for each individual pixel
				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; j++) {
						int pixelX = col + j;
						int pixelY = row + i;
						// Check for valid pixel coordinates
						if (pixelX >= 0 && pixelX < M && pixelY >= 0 && pixelY < N) {
							// Compute scalar gradients
							Gx_scalar += filt[M * pixelY + pixelX] * GxMask[i + 1][j + 1];
							Gy_scalar += filt[M * pixelY + pixelX] * GyMask[i + 1][j + 1];
						}
					}
				}
				// Compute the magnitude of the gradient
				int magnitude = abs(Gx_scalar) + abs(Gy_scalar);
				// Clamp magnitude to 255
				magnitude = magnitude > 255 ? 255 : magnitude;
				// Store the magnitude in the gradient array
				gradient[M * row + col] = (unsigned char)magnitude;
			}
		}
	}
}

void read_image(const char* filename)
{

	int c;
	FILE* finput;
	int i, j, temp;

	printf("\nReading %s image from disk ...", filename);
	finput = NULL;
	openfile(filename, &finput);

	if ((header[0] == 'P') && (header[1] == '5')) { //if P5 image

		for (j = 0; j < N; j++) {
			for (i = 0; i < M; i++) {

				//if (fscanf_s(finput, "%d", &temp,20) == EOF)
				//	exit(EXIT_FAILURE);
				temp = getc(finput);

				frame1[M * j + i] = (unsigned char)temp;
			}
		}
	}
	else if ((header[0] == 'P') && (header[1] == '2'))  { //if P2 image
		for (j = 0; j < N; j++) {
			for (i = 0; i < M; i++) {

				if (fscanf_s(finput, "%d", &temp,20) == EOF)
					exit(EXIT_FAILURE);

				frame1[M * j + i] = (unsigned char)temp;
			}
		}
	}
	else {
		printf("\nproblem with reading the image");
		exit(EXIT_FAILURE);
	}

	fclose(finput);
	printf("\nimage successfully read from disc\n");

}



void write_image2(const char* filename, unsigned char* output_image)
{

	FILE* foutput;
	int i, j;



	printf("  Writing result to disk ...\n");

	if ((err = fopen_s(&foutput,filename, "wb")) != NULL) {
		fprintf(stderr, "Unable to open file %s for writing\n", filename);
		exit(-1);
	}

	fprintf(foutput, "P2\n");
	fprintf(foutput, "%d %d\n", M, N);
	fprintf(foutput, "%d\n", 255);

	for (j = 0; j < N; ++j) {
		for (i = 0; i < M; ++i) {
			fprintf(foutput, "%3d ", output_image[M * j + i]);
			if (i % 32 == 31) fprintf(foutput, "\n");
		}
		if (M % 32 != 0) fprintf(foutput, "\n");
	}
	fclose(foutput);


}




void openfile(const char* filename, FILE** finput)
{
	int x0, y0, x , aa;

	if (( err = fopen_s(finput,filename, "rb")) != NULL) {
		fprintf(stderr, "Unable to open file %s for reading\n", filename);
		exit(-1);
	}

	aa = fscanf_s(*finput, "%s", header, 20);

	x0 = getint(*finput);//this is M
	y0 = getint(*finput);//this is N
	printf("\t header is %s, while x=%d,y=%d", header, x0, y0);


	//CRITICAL POINT: AT THIS POINT YOU CAN ASSIGN x0,y0 to M,N 
	// printf("\n Image dimensions are M=%d,N=%d",M,N);
	M = x0;
	N = y0;
	allocateImageArrays(&frame1, &filt, &gradient, M, N);

	x = getint(*finput); /* read and throw away the range info */
	//printf("\n range info is %d",x);

}



//CRITICAL POINT: you can define your routines here that create the arrays dynamically; now, the arrays are defined statically.
	
// Function: allocateImageArrays
// Purpose: Dynamically allocate memory for image processing arrays.
// Parameters:
//   - unsigned char** frame1: Pointer to a pointer for the input image array.
//   - unsigned char** filt: Pointer to a pointer for the filtered image array.
//   - unsigned char** gradient: Pointer to a pointer for the gradient image array.
//   - int M: Width of the image.
//   - int N: Height of the image.
void allocateImageArrays(unsigned char** frame1, unsigned char** filt, unsigned char** gradient, int M, int N) {

	// Size is the product of width (M), height (N), and the size of an unsigned char.
	*frame1 = (unsigned char*)malloc(M * N * sizeof(unsigned char));

	// Allocate memory for the filtered image array.
	*filt = (unsigned char*)malloc(M * N * sizeof(unsigned char));

	// Allocate memory for the gradient image array.
	*gradient = (unsigned char*)malloc(M * N * sizeof(unsigned char));

	// If any of the pointers is NULL, allocation failed.
	if (*frame1 == NULL || *filt == NULL || *gradient == NULL) {
		// Handle memory allocation failure
		fprintf(stderr, "Memory allocation failed\n");
		exit(1); // Exit or handle the error as required
	}
}


int getint(FILE* fp) /* adapted from "xv" source code */
{
	int c, i, firstchar;//, garbage;

	/* note:  if it sees a '#' character, all characters from there to end of
	   line are appended to the comment string */

	   /* skip forward to start of next number */
	c = getc(fp);
	while (1) {
		/* eat comments */
		if (c == '#') {
			/* if we're at a comment, read to end of line */
			char cmt[256], * sp;

			sp = cmt;  firstchar = 1;
			while (1) {
				c = getc(fp);
				if (firstchar && c == ' ') firstchar = 0;  /* lop off 1 sp after # */
				else {
					if (c == '\n' || c == EOF) break;
					if ((sp - cmt) < 250) *sp++ = c;
				}
			}
			*sp++ = '\n';
			*sp = '\0';
		}

		if (c == EOF) return 0;
		if (c >= '0' && c <= '9') break;   /* we've found what we were looking for */

		/* see if we are getting garbage (non-whitespace) */
	   // if (c!=' ' && c!='\t' && c!='\r' && c!='\n' && c!=',')
		//	garbage=1;

		c = getc(fp);
	}

	/* we're at the start of a number, continue until we hit a non-number */
	i = 0;
	while (1) {
		i = (i * 10) + (c - '0');
		c = getc(fp);
		if (c == EOF) return i;
		if (c < '0' || c>'9') break;
	}
	return i;
}








