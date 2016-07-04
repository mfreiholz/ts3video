#include "imageutil.h"

#include <QtMath>

#define min(a,b) (std::min<int>(a,b))


ImageFormat imageFormat(QImage::Format format)
{
	switch (format)
	{
	case QImage::Format_ARGB32:
	case QImage::Format_RGB32:
		return ImageFormat::ARGB32;
	case QImage::Format_RGB888:
		return ImageFormat::RGB24;
	}
	return (ImageFormat) - 1;
}

/*
    http://www.geisswerks.com/ryan/FAQS/resize.html
*/
void resizeHq4ch(unsigned char* src, int w1, int h1,
				 unsigned char* dest, int w2, int h2,
				 volatile bool* pQuitFlag)
{
	// Both buffers must be in ARGB format, and a scanline should be w*4 bytes.

	// If pQuitFlag is non-NULL, then at the end of each scanline, it will check
	//    the value at *pQuitFlag; if it's set to 'true', this function will abort.
	// (This is handy if you're background-loading an image, and decide to cancel.)

	// NOTE: THIS WILL OVERFLOW for really major downsizing (2800x2800 to 1x1 or more)
	// (2800 ~ sqrt(2^23)) - for a lazy fix, just call this in two passes.

	int* g_px1a    = NULL;
	int  g_px1a_w  = 0;
	int* g_px1ab   = NULL;
	int  g_px1ab_w = 0;

	unsigned int* dsrc  = (unsigned int*)src;
	unsigned int* ddest = (unsigned int*)dest;

	bool bUpsampleX = (w1 < w2);
	bool bUpsampleY = (h1 < h2);

	// If too many input pixels map to one output pixel, our 32-bit accumulation values
	// could overflow - so, if we have huge mappings like that, cut down the weights:
	//    256 max color value
	//   *256 weight_x
	//   *256 weight_y
	//   *256 (16*16) maximum # of input pixels (x,y) - unless we cut the weights down...
	int weight_shift = 0;
	float source_texels_per_out_pixel = ((w1 / (float)w2 + 1) * (h1 / (float)h2 + 1));
	float weight_per_pixel = source_texels_per_out_pixel * 256 * 256;  //weight_x * weight_y
	float accum_per_pixel = weight_per_pixel * 256; //color value is 0-255
	float weight_div = accum_per_pixel / 4294967000.0f;
	if (weight_div > 1)
		weight_shift = (int)ceilf(logf((float)weight_div) / logf(2.0f));
	weight_shift = min(15, weight_shift);  // this could go to 15 and still be ok.

	float fh = 256 * h1 / (float)h2;
	float fw = 256 * w1 / (float)w2;

	if (bUpsampleX && bUpsampleY)
	{
		// faster to just do 2x2 bilinear interp here

		// cache x1a, x1b for all the columns:
		// ...and your OS better have garbage collection on process exit :)
		if (g_px1a_w < w2)
		{
			if (g_px1a)
				delete [] g_px1a;
			g_px1a = new int[w2 * 2 * 1];
			g_px1a_w = w2 * 2;
		}
		for (int x2 = 0; x2 < w2; x2++)
		{
			// find the x-range of input pixels that will contribute:
			int x1a = (int)(x2 * fw);
			x1a = min(x1a, 256 * (w1 - 1) - 1);
			g_px1a[x2] = x1a;
		}

		// FOR EVERY OUTPUT PIXEL
		for (int y2 = 0; y2 < h2; y2++)
		{
			// find the y-range of input pixels that will contribute:
			int y1a = (int)(y2 * fh);
			y1a = min(y1a, 256 * (h1 - 1) - 1);
			int y1c = y1a >> 8;

			unsigned int* ddest = &((unsigned int*)dest)[y2 * w2 + 0];

			for (int x2 = 0; x2 < w2; x2++)
			{
				// find the x-range of input pixels that will contribute:
				int x1a = g_px1a[x2];//(int)(x2*fw);
				int x1c = x1a >> 8;

				unsigned int* dsrc2 = &dsrc[y1c * w1 + x1c];

				// PERFORM BILINEAR INTERPOLATION on 2x2 pixels
				unsigned int r = 0, g = 0, b = 0, a = 0;
				unsigned int weight_x = 256 - (x1a & 0xFF);
				unsigned int weight_y = 256 - (y1a & 0xFF);
				for (int y = 0; y < 2; y++)
				{
					for (int x = 0; x < 2; x++)
					{
						unsigned int c = dsrc2[x + y * w1];
						unsigned int r_src = (c) & 0xFF;
						unsigned int g_src = (c >> 8) & 0xFF;
						unsigned int b_src = (c >> 16) & 0xFF;
						unsigned int w = (weight_x * weight_y) >> weight_shift;
						r += r_src * w;
						g += g_src * w;
						b += b_src * w;
						weight_x = 256 - weight_x;
					}
					weight_y = 256 - weight_y;
				}

				unsigned int c = ((r >> 16)) | ((g >> 8) & 0xFF00) | (b & 0xFF0000);
				*ddest++ = c;//ddest[y2*w2 + x2] = c;
			}
		}
	}
	else
	{
		// cache x1a, x1b for all the columns:
		// ...and your OS better have garbage collection on process exit :)
		if (g_px1ab_w < w2)
		{
			if (g_px1ab)
				delete [] g_px1ab;
			g_px1ab = new int[w2 * 2 * 2];
			g_px1ab_w = w2 * 2;
		}
		for (int x2 = 0; x2 < w2; x2++)
		{
			// find the x-range of input pixels that will contribute:
			int x1a = (int)((x2) * fw);
			int x1b = (int)((x2 + 1) * fw);
			if (bUpsampleX) // map to same pixel -> we want to interpolate between two pixels!
				x1b = x1a + 256;
			x1b = min(x1b, 256 * w1 - 1);
			g_px1ab[x2 * 2 + 0] = x1a;
			g_px1ab[x2 * 2 + 1] = x1b;
		}

		// FOR EVERY OUTPUT PIXEL
		for (int y2 = 0; y2 < h2; y2++)
		{
			// find the y-range of input pixels that will contribute:
			int y1a = (int)((y2) * fh);
			int y1b = (int)((y2 + 1) * fh);
			if (bUpsampleY) // map to same pixel -> we want to interpolate between two pixels!
				y1b = y1a + 256;
			y1b = min(y1b, 256 * h1 - 1);
			int y1c = y1a >> 8;
			int y1d = y1b >> 8;

			for (int x2 = 0; x2 < w2; x2++)
			{
				// find the x-range of input pixels that will contribute:
				int x1a = g_px1ab[x2 * 2 + 0]; // (computed earlier)
				int x1b = g_px1ab[x2 * 2 + 1]; // (computed earlier)
				int x1c = x1a >> 8;
				int x1d = x1b >> 8;

				// ADD UP ALL INPUT PIXELS CONTRIBUTING TO THIS OUTPUT PIXEL:
				unsigned int r = 0, g = 0, b = 0, a = 0;
				for (int y = y1c; y <= y1d; y++)
				{
					unsigned int weight_y = 256;
					if (y1c != y1d)
					{
						if (y == y1c)
							weight_y = 256 - (y1a & 0xFF);
						else if (y == y1d)
							weight_y = (y1b & 0xFF);
					}

					unsigned int* dsrc2 = &dsrc[y * w1 + x1c];
					for (int x = x1c; x <= x1d; x++)
					{
						unsigned int weight_x = 256;
						if (x1c != x1d)
						{
							if (x == x1c)
								weight_x = 256 - (x1a & 0xFF);
							else if (x == x1d)
								weight_x = (x1b & 0xFF);
						}

						unsigned int c = *dsrc2++;//dsrc[y*w1 + x];
						unsigned int r_src = (c) & 0xFF;
						unsigned int g_src = (c >> 8) & 0xFF;
						unsigned int b_src = (c >> 16) & 0xFF;
						unsigned int w = (weight_x * weight_y) >> weight_shift;
						r += r_src * w;
						g += g_src * w;
						b += b_src * w;
						a += w;
					}
				}

				// write results
				unsigned int c = ((r / a)) | ((g / a) << 8) | ((b / a) << 16);
				*ddest++ = c;//ddest[y2*w2 + x2] = c;
			}

			if (pQuitFlag && *pQuitFlag)
				break;
		}
	}

	if (g_px1a)
		delete [] g_px1a;
	if (g_px1ab)
		delete [] g_px1ab;
}


/*****************************************************************************/
/* Static functions                                                          */
/*****************************************************************************/


// Conversion per http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html#RTFToC30
int rcoeff(int y, int u, int v)
{
	return 298082 * y +      0 * u + 408583 * v;
}
int gcoeff(int y, int u, int v)
{
	return 298082 * y - 100291 * u - 208120 * v;
}
int bcoeff(int y, int u, int v)
{
	return 298082 * y + 516411 * u +      0 * v;
}


int clamp(int vv)
{
	if (vv < 0)
		return 0;
	else if (vv > 255)
		return 255;
	return vv;
}


static double sinc(double x)
{
	return sin(x) / x;
}


/**
	Build 1-D kernel
	Conv: array of 2n+1 floats (-n .. 0 .. +n)
*/
void lanczos_1d(double* conv, unsigned n, double fac)
{
	unsigned i;

	conv[n] = 1.;
	for (i = 1; i <= n; ++i)
	{
		double xpi = i * M_PI * fac;
		conv[n + i] = conv[n - i] = sinc(xpi) * sinc(xpi / n);
	}
}

/**
	\ref http://www.telegraphics.com.au/svn/webpformat/trunk/decode.c
*/
void lanczos_interp2(unsigned char* in, unsigned char* out, int in_stride, int out_w, int out_h)
{
	double conv[2 * KERNEL_SIZE + 1], *knl, sum, weight;
	float* t_buf, *t_row;
	int i, j, d, k, x, y;
	int in_w = (out_w + 1) / 2;
	int in_h = (out_h + 1) / 2;
	unsigned char* in_row, *out_row;

	lanczos_1d(conv, KERNEL_SIZE, 0.5);
	knl = conv + KERNEL_SIZE; // zeroth index

	// Apply horizontal convolution, interpolating input to output 1:2.
	// Store this in a temporary buffer, with same row count as input.
	// (This code is completely un-optimised, apart from precomputing the kernel, above.)

	t_buf = (float*)malloc(sizeof(*t_buf) * in_h * out_w);

	for (j = 0, in_row = in, t_row = t_buf; j < in_h; ++j, in_row += in_stride, t_row += out_w)
	{
		for (i = 0; i < out_w; ++i)
		{
			d = i % 2;

			// Find involved input pixels.
			// Multiply each by corresponding kernel coefficient.
			sum		= d ? 0. : knl[0] * in_row[i / 2];
			weight	= d ? 0. : knl[0];

			// Step out from the zero point, in relative output ordinates.
			for (k = 2 - d; k <= KERNEL_SIZE; k += 2)
			{
				x = (i - k) / 2;
				if (x >= 0)
				{
					sum += knl[k] * in_row[x];
					weight += knl[k];
				}

				x = (i + k) / 2;
				if (x < in_w)
				{
					sum += knl[k] * in_row[x];
					weight += knl[k];
				}
			}
			t_row[i] = sum / weight;
		}
	}

	// Interpolate vertically.
	for (i = 0; i < out_w; ++i)
	{
		t_row = t_buf + i;
		out_row = out + i;
		for (j = 0; j < out_h; ++j)
		{
			d = j % 2;

			// Find involved input pixels.
			// Multiply each by corresponding kernel coefficient.
			sum		= d ? 0. : knl[0] * t_row[out_w * j / 2];
			weight	= d ? 0. : knl[0];

			// Step out from the zero point, in relative output ordinates.
			for (k = 2 - d; k <= KERNEL_SIZE; k += 2)
			{
				y = (j - k) / 2;
				if (y >= 0)
				{
					sum += knl[k] * t_row[out_w * y];
					weight += knl[k];
				}
				y = (j + k) / 2;
				if (y < in_h)
				{
					sum += knl[k] * t_row[out_w * y];
					weight += knl[k];
				}
			}
			out_row[0] = clamp(sum / weight + 0.5);

			// Step down one row.
			out_row += out_w;
		}
	}

	delete t_buf;
}

/**
	Function for converting a RGB encoded image into a YV12 encoded image.
	\ref http://stackoverflow.com/questions/4765436/need-to-create-a-webm-video-from-rgb-frames
*/
void rgbToYV12(unsigned char* pRGBData, int nFrameWidth, int nFrameHeight, void* pFullYPlane, void* pDownsampledUPlane, void* pDownsampledVPlane, ImageFormat format)
{
	int bytesPerPixel;
	if (format == ImageFormat::ARGB32 || format == ImageFormat::BGRA32)
		bytesPerPixel = 4;
	else
		bytesPerPixel = 3;
	int nRGBBytes = nFrameWidth * nFrameHeight * bytesPerPixel;

	// Convert RGB -> YV12. We do this in-place to avoid allocating any more memory.
	unsigned char* pYPlaneOut = (unsigned char*)pFullYPlane;
	int nYPlaneOut = 0;

	int offset = (format == ImageFormat::BGRA32) ? 1 : 0;
	unsigned char R, G, B;
	for (int i = 0; i < nRGBBytes; i += bytesPerPixel)
	{
		if (format == ImageFormat::ARGB32 || format == ImageFormat::RGB24)
		{
			B = pRGBData[i + 0 + offset];
			G = pRGBData[i + 1 + offset];
			R = pRGBData[i + 2 + offset];
		}
		else
		{
			R = pRGBData[i + 0 + offset];
			G = pRGBData[i + 1 + offset];
			B = pRGBData[i + 2 + offset];
		}

		float y = (float)(R * 66 +  G * 129 + B * 25  + 128) / 256 + 16;
		float u = (float)(R * -38 + G * -74 + B * 112 + 128) / 256 + 128;
		float v = (float)(R * 112 + G * -94 + B * -18 + 128) / 256 + 128;

		// NOTE: We're converting pRGBData to YUV in-place here as well as writing out YUV to pFullYPlane/pDownsampledUPlane/pDownsampledVPlane.
		pRGBData[i + 0 + offset] = (unsigned char)y;
		pRGBData[i + 1 + offset] = (unsigned char)u;
		pRGBData[i + 2 + offset] = (unsigned char)v;

		// Write out the Y plane directly here rather than in another loop.
		pYPlaneOut[nYPlaneOut++] = pRGBData[i + 0 + offset];
	}

	// Downsample to U and V.
	int halfWidth = nFrameWidth >> 1;
	int halfHeight = nFrameHeight >> 1;

	unsigned char* pUPlaneOut = (unsigned char*)pDownsampledUPlane;
	unsigned char* pVPlaneOut = (unsigned char*)pDownsampledVPlane;

	for (int yPixel = 0; yPixel < halfHeight; yPixel++)
	{
		int iBaseSrc = ((yPixel * 2) * nFrameWidth * bytesPerPixel);

		for (int xPixel = 0; xPixel < halfWidth; xPixel++)
		{
			pUPlaneOut[yPixel * halfWidth + xPixel] = pRGBData[iBaseSrc + 1 + offset];
			pVPlaneOut[yPixel * halfWidth + xPixel] = pRGBData[iBaseSrc + 2 + offset];

			if (format == ImageFormat::ARGB32 || format == ImageFormat::BGRA32)
				iBaseSrc += 8;
			else
				iBaseSrc += 6;
		}
	}
}
