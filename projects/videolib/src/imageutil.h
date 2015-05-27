#ifndef IMAGEUTILITY_H
#define IMAGEUTILITY_H

#include <QImage>

enum ImageFormat
{
	RGB24, ARGB32, BGA24, BGRA32
};

ImageFormat imageFormat( QImage::Format format );

// Fixed point scale & round result of matrix multiplication to 0..255.
#define SCALEYUV(v) (((v)+128000)/256000)

/*
 * The kernel size affects quality of the filtered result. Beyond 8,
 * there are diminishing returns but 4 is noticeably inferior to 8.
 * With large kernels such as 32 or even 256, objectionable ringing
 * artefacts are more evident (as well as being more costly to compute).
 */
#define KERNEL_SIZE 4

void resizeHq4ch( unsigned char* src, int w1, int h1,
					unsigned char* dest, int w2, int h2,
					volatile bool* pQuitFlag );

int rcoeff(int y, int u, int v);
int gcoeff(int y, int u, int v);
int bcoeff(int y, int u, int v);
int clamp( int vv );

void lanczos_interp2(unsigned char *in, unsigned char *out,
					int in_stride, int out_w, int out_h);

void rgbToYV12( unsigned char *pRGBData,
					int nFrameWidth, int nFrameHeight, void *pFullYPlane,
					void *pDownsampledUPlane, void *pDownsampledVPlane, ImageFormat format = BGA24 );

#endif
