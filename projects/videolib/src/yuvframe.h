#ifndef YUVFRAME_HEADER
#define YUVFRAME_HEADER

#include <QSharedPointer>

#include "imageutil.h"

class QImage;

class YuvFrame
{
public:
	YuvFrame();
	~YuvFrame();
	YuvFrame* copy() const;
	QImage toQImage() const;
	void overlayDarkEdge(int posx, int posy, int width, int height);

	static YuvFrame* fromQImage(const QImage& img);
	static YuvFrame* fromRgb(const unsigned char* rgb, uint width, uint height, const ImageFormat& imgFormat = RGB24, uint stride = 0, bool bottomUp = false);
	static YuvFrame* createBlackImage(uint width, uint height);
	static YuvFrame* create(int width, int height);

public:
	uint width;
	uint height;
	unsigned char* y;
	unsigned char* u;
	unsigned char* v;
};
typedef QSharedPointer<YuvFrame> YuvFrameRefPtr;

#endif
