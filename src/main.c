#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applicationParameters.h"
#include "yuvDisplay.h" 
#include "yuvRead.h"	
#include "yuvWrite.h"
#include "stabilization.h"
#include "md5.h"

//#define VERBOSE
#include <stdio.h> //TO DELETE
#ifdef VERBOSE 1
#include <stdio.h>
#endif

int stopThreads = 0;

int main(int argc, char** argv)
{
	// Declarations
	static unsigned char y[HEIGHT*WIDTH], u[HEIGHT*WIDTH / 4], v[HEIGHT*WIDTH / 4];
	static unsigned char yPrevious[HEIGHT*WIDTH];
	static unsigned char yDisp[DISPLAY_W*DISPLAY_H], uDisp[DISPLAY_W*DISPLAY_H / 4], vDisp[DISPLAY_W*DISPLAY_H / 4];
	static coord motionVectors[(HEIGHT / BLOCK_HEIGHT)*(WIDTH / BLOCK_WIDTH)];
	static coordf dominatingMotionVector;
	static coordf accumulatedMotion = { 0.0f, 0.0f };

	// Init display
	yuvDisplayInit(0, DISPLAY_W, DISPLAY_H);

	// Open files
	initReadYUV(WIDTH, HEIGHT);
	initYUVWrite();
	
	//clear MD5 file //TO DELETE
	FILE *f = fopen("MD5_new.txt", "w");
	fprintf(f, "");
	fclose(f);

	unsigned int frameIndex = 1;
	while (!stopThreads)
	{
		// Backup previous frame
		memcpy(yPrevious, y, HEIGHT*WIDTH);

		// Read a frame
		readYUV(WIDTH, HEIGHT, y, u, v); //1 float : FPS

		// Compute motion vectors
		computeBlockMotionVectors(WIDTH, HEIGHT,							//PARALLELIZED
								  BLOCK_WIDTH, BLOCK_HEIGHT,
								  MAX_DELTA_X, MAX_DELTA_Y,
								  y, yPrevious,
								  motionVectors);

		// Find dominating motion vector
		const int nbVectors = (HEIGHT / BLOCK_HEIGHT)*(WIDTH / BLOCK_WIDTH);
		findDominatingMotionVector(nbVectors,								//PARALLELIZED
								   motionVectors, &dominatingMotionVector);	//Tableau de floats
		#ifdef VERBOSE
		// Print motion vector
		printf("Frame %3d: %2.2f, %2.2f\n", frameIndex,
			   dominatingMotionVector.x, dominatingMotionVector.y);
		#endif

		// Accumulate motion
		accumulateMotion(&dominatingMotionVector, &accumulatedMotion, &accumulatedMotion);

		// Render the motion compensated frame	//PARALLELIZED
		renderFrame(WIDTH, HEIGHT, DISPLAY_W, DISPLAY_H, &accumulatedMotion, y, u, v, yDisp, uDisp, vDisp);

		// Display it
		yuvDisplay(0, yDisp, uDisp, vDisp);

		// Compute the MD5 of the rendered frame
		MD5_Update(DISPLAY_H*DISPLAY_W, yDisp);

		// Save it
		yuvWrite(DISPLAY_W, DISPLAY_H, yDisp, uDisp, vDisp);

		// Exit ?
		frameIndex++;
		if (frameIndex == NB_FRAME){//NB_FRAME){
			stopThreads = 1;
			//system("pause");
		}
	}

	#ifdef VERBOSE
	printf("Exit program\n");
	#endif 
	yuvFinalize(0);
	endYUVRead();
	endYUVWrite();

	return 0;
}
