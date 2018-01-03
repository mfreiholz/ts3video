#if defined(OCS_INCLUDE_OPENGL)
#ifndef _OPENGLRENDERTHREAD_HEADER_
#define _OPENGLRENDERTHREAD_HEADER_

#include "QtCore/QThread"
#include "QtCore/QSharedPointer"
#include "QtCore/QWeakPointer"
#include "QtCore/QMutex"

#include "QtGui/QWindow"
#include "QtGui/QOpenGLFunctions"

#include "videolib/yuvframe.h"

#include "openglwindow.h"

class OpenGLWindow;

class OpenGLRenderThread : public QThread
{
	Q_OBJECT
	class Private;
  QScopedPointer<Private> d;

private:
	explicit OpenGLRenderThread();

	struct RenderClient;

	static QWeakPointer<OpenGLRenderThread> _instance;
	static QMutex _instanceMutex;

public:
	~OpenGLRenderThread();

	virtual void run();
	void stop();

	int registerSurface( OpenGLWindow *surface );
	bool removeSurface( int id );

	int subframeId( int id, int posx, int posy );

	static QSharedPointer<OpenGLRenderThread> instance();

public slots:
	void resize( int id, const QSize &size );
	void update( int id );

	void setData( int id, QSharedPointer<YuvFrame> data, int frameId = 0 );
	void setActive( int id, int frameId = 0 );
	void removeSubFrame( int id, int frameId = 0 );
	
	void setRenderMode( int id, OpenGLWindow::RenderMode renderMode );
	void setMirrorEnabled( int id, bool enabled, int frameId = 0 );
	void setDarkEdgeEnabled( int id, bool enabled );
	void setSubframesEnabled( int id, bool enabled );

private slots:
	bool initialize( int id );
	void render( RenderClient *rc );

};

#endif // _OPENGLRENDERTHREAD_HEADER_
#endif