#include "openglwindow.h"

#include "QtCore/QCoreApplication"
#include "QtCore/QSharedPointer"

#include "QtGui/QPainter"
#include "QtGui/QResizeEvent"
#include "QtGui/QColor"

#include "QtWidgets/QWidget"

#include "openglrenderthread.h"

#include "humblelogging/api.h"

HUMBLE_LOGGER(logger, "client.gui.opengl.openglwindow");

class OpenGLWindow::Private
{
public:
  OpenGLWindow *_pOwner;
  Private (OpenGLWindow *owner) : _pOwner(owner) { init(); }

	void init() {
		renderId = -1;
		containerWidget = nullptr;
		//backgroundColor = QColor( 0, 0, 0 );
		backgroundColor = QColor( 242, 147, 213 ); // Pink
	}

	QSharedPointer<OpenGLRenderThread> thread;
	int renderId;

	QWidget *containerWidget;
	QColor backgroundColor;
};

OpenGLWindow::OpenGLWindow( QWindow *parent )
	: QWindow( parent ), d( new Private(this) )
{
	setSurfaceType( QWindow::OpenGLSurface );

	QSurfaceFormat format;
	//format.setDepthBufferSize( 24 );
	//format.setSamples( 4 );
	//format.setMajorVersion( 3 );
	//format.setMinorVersion( 3 );
	format.setSwapBehavior( QSurfaceFormat::DoubleBuffer );
	format.setRenderableType( QSurfaceFormat::OpenGL );
	format.setProfile( QSurfaceFormat::CompatibilityProfile );
	setFormat( format );
	create();

	d->thread = OpenGLRenderThread::instance();
	d->renderId = d->thread->registerSurface( this );
	d->thread->update( d->renderId );
}

OpenGLWindow::~OpenGLWindow()
{
	if( d->thread ) {
		d->thread->removeSurface( d->renderId );
	}

	if( d->containerWidget ) {
		d->containerWidget->deleteLater();
	}
}

bool OpenGLWindow::event( QEvent *event )
{
	switch( event->type() ) {
	case QEvent::UpdateRequest:
		d->thread->update( d->renderId );
		return true;
	default:
		return QWindow::event( event );
	}
}

void OpenGLWindow::exposeEvent( QExposeEvent *event )
{
	Q_UNUSED( event );

	if( isExposed() ) {
		d->thread->update( d->renderId );
	}
}

void OpenGLWindow::resizeEvent( QResizeEvent *ev )
{
	if( !d->thread )
		return;
	QSize size = ev->size();
	d->thread->resize( d->renderId, size );
}

void OpenGLWindow::mousePressEvent( QMouseEvent *ev )
{
	if( ev->button() == Qt::LeftButton ) {
		emit mouseClicked();

		if( d->thread ) {
			QPoint pos = ev->pos();
			int id = d->thread->subframeId( d->renderId, pos.x(), pos.y() );
			if( id >= 0 ) {
				d->thread->setActive( d->renderId, id );
				emit subframeClicked( id );
			}
		}
	}
}

void OpenGLWindow::setData( QSharedPointer<YuvFrame> data , int id )
{
	if( !d->thread )
		return;
	d->thread->setData( d->renderId, data, id );
}

void OpenGLWindow::setActive( int id )
{
	if( !d->thread )
		return;
	d->thread->setActive( d->renderId, id );
}

void OpenGLWindow::setRenderMode( RenderMode renderMode )
{
	if( !d->thread )
		return;
	d->thread->setRenderMode( d->renderId, renderMode );
}

void OpenGLWindow::setMirrorEnabled( bool enabled , int id )
{
	if( !d->thread )
		return;
	d->thread->setMirrorEnabled( d->renderId, enabled, id );
}

void OpenGLWindow::setDarkEdgeEnabled( bool enabled )
{
	if( !d->thread )
		return;
	d->thread->setDarkEdgeEnabled( d->renderId, enabled );
}

void OpenGLWindow::setSubframesEnabled( bool enabled )
{
	if( !d->thread )
		return;
	d->thread->setSubframesEnabled( d->renderId, enabled );
}

QWidget* OpenGLWindow::widget()
{
	if( !d->containerWidget )
		d->containerWidget = QWidget::createWindowContainer( this );
	return d->containerWidget;
}

QColor OpenGLWindow::backgroundColor() const
{
	return d->backgroundColor;
}

void OpenGLWindow::setBackgroundColor(const QColor &color)
{
	d->backgroundColor = color;
}
