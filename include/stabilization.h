/**
* Stabilization contains all functions needed to:
* - Find the motion resulting from camera shaking in a video.
* - Render a frame where this motion is compensated.
*
* @file stabilization.h
* @author kdesnos
* @date 2016.09.01
* @version 1.0
* @copyright CECILL-C
*/

#include <assert.h>



#include "applicationParameters.h"
#include "matrix.h"

#ifndef STABILIZATION_H
#define STABILIZATION_H

/**
* Define the color of the black background for rendered frames.
*/
#define BG_BLACK_Y 0
#define BG_BLACK_U 128
#define BG_BLACK_V 128

/**
* MIN and MAX macro to lighten code.
*/
#define MIN(x,y) ((x>y)? y:x)
#define MAX(x,y) ((x>y)? x:y)


/**
* Render a motion compensated frame in for a display of configurable size.
*
* This method generates an new YUV frame of pixels of size dispWidth*dispHeight
* where the input frame is placed in front of a black background. The position
* of the frame is centered with a displacement of deltaX and deltaY pixels on
* the abscissa and ordinate coordinates respectively.
*
* @param frameWidth
*		 width of the input frame in pixels.
* @param frameHeight
*		 height of the input frame in pixels.
* @param dispWidth
*		 width of the rendered frame in pixels.
* @param dispHeight
*		 height of the rendered frame in pixels.
* @param delta
*		 displacement of the input frame.
* @param yIn
*        the Y component of the frame to render.
* @param uIn
*        the U component of the frame to render.
* @param vIn
*        the V component of the frame to render.
* @param yOut
*        the Y component of the rendered frame.
* @param uOut
*        the U component of the rendered frame
* @param vOut
*        the V component of the rendered frame
*/
void renderFrame(const int frameWidth, const int frameHeight,
				 const int dispWidth, const int dispHeight,
				 const coordf * const delta,
				 const unsigned char * const yIn, const unsigned char * const uIn, const unsigned char * const vIn,
				 unsigned char * const yOut, unsigned char * const uOut, unsigned char * const vOut);

/**
* Computes the motion vectors for the frame decomposed into blocks of
* configurable size.
*
* The method first divides the input frame into blocks of blockWidth*
* blockPixels (ignoring incomplete blocks). Then, the position of each block
* in the previousFrame is searched by computing the minimum mean square error
* (MMSE) for all possible positions in the previousFrame. Possible positions
* in the previous frame are determined relatively to the block position in the
* frame and a neighborhood limited by the maxDeltaX and maxDeltaY parameters.
* Results are returned as two arrays containing the abscissa and ordinates of
* computed vectors respectively.
*
* @param width
*		 width of the input frame in pixels.
* @param height
*		 height of the input frame in pixels.
* @param blockWidth
*		 width of matched blocks in pixels.
* @param blockheight
*		 height of matched blocks in pixels.
* @param maxDeltaX
*		 maximum displacement of blocks on the abscissa.
* @param maxDeltaY
*		 maximum displacement of blocks on the ordinates.
* @param frame
*		 input frame whose blocks will be matched.
* @param previousFrame
*		 frame in which blocks will be matched.
* @param vectors
*		 motion vectors of block in raster scan order.
*/
void computeBlockMotionVectors(const int width, const int height,
							   const int blockWidth, const int blockHeight,
							   const int maxDeltaX, const int maxDeltaY,
							   const unsigned char * const frame, const unsigned char * const previousFrame,
							   coord * const vectors);

/**
* Computes the motion vectors for the given block.
*
* The position of the block in the previousFrame is searched by computing the
* minimum mean square error (MMSE) for all possible positions in the
* previousFrame. Possible positions in the previous frame are determined
* relatively to the block position in the frame and a neighborhood limited by
* the maxDeltaX and maxDeltaY parameters. Results are returned as vector
* containing the abscissa and ordinates of the computed vector respectively.
*
* @param width
*		 width of the input frame in pixels.
* @param height
*		 height of the input frame in pixels.
* @param blockWidth
*		 width of matched blocks in pixels.
* @param blockheight
*		 height of matched blocks in pixels.
* @param maxDeltaX
*		 maximum displacement of blocks on the abscissa.
* @param maxDeltaY
*		 maximum displacement of blocks on the ordinates.
* @param b
*		 matched block.
* @param previousFrame
*		 frame in which b will be matched.
* @param vector
*		 motion vector of b.
*/
void computeBlockMotionVector(const int width, const int height,
							  const int blockWidth, const int blockHeight,
							  const int maxDeltaX, const int maxDeltaY,
							  const coord * const blockCoord, const unsigned char * const blockData, 
							  const unsigned char * const previousFrame,
							  coord * const vector);

/**
* The method divides the input frame into blocks of blockWidth*
* blockPixels (ignoring incomplete blocks).
*
* @param width
*		 width of the input frame in pixels.
* @param height
*		 height of the input frame in pixels.
* @param blockWidth
*		 width of matched blocks in pixels.
* @param blockheight
*		 height of matched blocks in pixels.
* @param frame
*		 input frame whose blocks will be matched.
* @param blocks
*		 computed blocks in the raster scan order.
*/
void divideBlocks(const int width, const int height,
				  const int blockWidth, const int blockHeight,
				  const unsigned char * const frame,
				  coord * const blocksCoord,
				  unsigned char * const blocksData);

/**
* Computes the Mean Squared Error for the given block at the given position.
*
* If at the given position in the reference frame, less than half of the pixels
* of the block are matched within the frame, the cost is not computed.
*
* @param width
*		 width of the input frame in pixels.
* @param height
*		 height of the input frame in pixels.
* @param blockWidth
*		 width of matched blocks in pixels.
* @param blockheight
*		 height of matched blocks in pixels.
* @param deltaX
*		 abscissa of the first matched pixels in the previousFrame
* @param deltaY
*		 ordinate of the first matched pixels in the previousFrame
* @param b
*		 matched block.
* @param previousFrame
*		 frame in which b will be matched.
* @return
*        the computed Mean Squared Error.
*/
unsigned int computeMeanSquaredError(const int width, const int height,
									 const int blockWidth, const int blockHeight,
									 const int deltaX, const int deltaY,
									 const coord * blockCoort,
									 const unsigned char * const blockData, 
									 const unsigned char * const previousFrame);


/**
* Finds the dominating vector in an array of vectors.
*
* The dominating vector is computed by first removing from the array of vectors
* all the vectors that are too far from the mean vector from the multivariate
* gaussian perspective. Then, the dominating vector is simply the average of
* remaining vectors.
*
* There probably exist simpler way of finding the dominating vector (median ?),
* but this one has been kept for its good results.
*
* @param nbVectors
*		 number of vectors in vectors array.
* @param vector
*		 array of vectors.
* @param dominatingVector
*		 vector resulting from the computation
*/
void findDominatingMotionVector(const int nbVectors,
								const coord * const vectors, coordf * const dominatingVector);


/**
* Accumulated the given motionVector in the given accumulatedMotion.
* 
* If a non-zero HIGH_PASS_FILTER_TAP is defined, the function applies it.
* 
* @param motionVector
*        new 2D motion to accumulate.
* @param accumulatedMotionIn
*        accumulated 2D motion vector to update.
** @param accumulatedMotionOut
*        updated accumulated 2D motion vector.
*/
void accumulateMotion(const coordf * const motionVector, const coordf * const accumulatedMotionIn, coordf * const accumulatedMotionOut);


#endif