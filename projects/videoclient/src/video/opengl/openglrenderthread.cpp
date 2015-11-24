#include "openglrenderthread.h"

#include "QtCore/QCoreApplication"
#include "QtCore/QList"
#include "QtCore/QHash"
#include "QtCore/QMutex"
#include "QtCore/QWaitCondition"
#include "QtCore/QAtomicInt"
#include "QtCore/QScopedPointer"
#include "QtCore/QQueue"
#include "QtCore/QRect"

#include "QtGui/QOpenGLContext"
#include "QtGui/QOpenGLShaderProgram"
#include "QtGui/QColor"

#include "openglwindow.h"

#include "humblelogging/api.h"

HUMBLE_LOGGER( logger, "client.gui.opengl.openglrenderthread" );

QWeakPointer<OpenGLRenderThread> OpenGLRenderThread::_instance;
QMutex OpenGLRenderThread::_instanceMutex;

QSharedPointer<OpenGLRenderThread> OpenGLRenderThread::instance()
{
	QMutexLocker l( &_instanceMutex );

	QSharedPointer<OpenGLRenderThread> shared = _instance.toStrongRef();
	if( shared.isNull() ) {
		shared = QSharedPointer<OpenGLRenderThread>( new OpenGLRenderThread() );
		_instance = shared.toWeakRef();
	}

	if( !shared->isRunning() )
		shared->start();

	return shared;
}

struct OpenGLRenderThread::RenderClient
{
	struct RenderFrame {
		QSharedPointer<YuvFrame> frame;
		bool mirrored;

		RenderFrame() {
			mirrored = false;
		}
	};

	OpenGLWindow *surface;
	QScopedPointer<QOpenGLContext> context;
	QScopedPointer<QOpenGLFunctions> functions;
	QScopedPointer<QOpenGLShaderProgram> shaderProgram;

	QList<int> ids;
	QHash<int,RenderFrame*> frames;
	int selectedId;

	OpenGLWindow::RenderMode renderMode;
	bool renderSubframes;
	bool renderDarkEdge;
	int width, height, frameCounter;

	bool doResize;
	bool forceDraw;
	unsigned int textureIds[3];

	QHash<int,QRect> subframeAreas;

	QMutex mutex;

	RenderClient() {
		surface = nullptr;
		selectedId = -1;
		renderMode = OpenGLWindow::RM_CROPPED;
		renderSubframes = false;
		renderDarkEdge = false;
		width = height = frameCounter = 0;
		doResize = forceDraw = true;
	}

	~RenderClient() {
		QList<int> keys = frames.keys();
		foreach( const int &id, keys ) {
			RenderFrame *rf = frames.value( id );
			if( rf ) {
				frames.remove( id );
				delete rf;
			}
		}
	}

	RenderFrame* getOrCreateRenderFrame( int id ) {
		RenderFrame *rf = frames.value( id );
		if( !rf ) {
			rf = new RenderFrame;
			frames.insert( id, rf );
			if( !ids.contains( id ) )
				ids.append( id );
		}
		return rf;
	}

	bool removeRenderFrame( int id ) {
		ids.removeAll( id );
		RenderFrame *rf = frames.value( id );
		if( !rf )
			return false;
		frames.remove( id );
		delete rf;
		return true;
	}
};

class OpenGLRenderThread::Private
{
public:
  Private(OpenGLRenderThread *owner) : _pOwner(owner) { init(); }

	void init()
  {
		rendering = false;
		ids.store( 0 );
	}

  OpenGLRenderThread *_pOwner;

	bool rendering;

	QAtomicInt ids;
	QHash<int,RenderClient*> clients;
	QQueue<int> queue;

	QMutex dataMutex;
	QMutex queueMutex;
	QWaitCondition queueWaitCondition;

	void cleanup() {
		QList<int> keys = clients.keys();
		foreach( const int &id, keys ) {
			RenderClient *rc = clients.value( id );
			if( !rc )
				continue;

			clients.remove( id );
			delete rc;
		}
	}

};

OpenGLRenderThread::OpenGLRenderThread()
	: QThread(), d( new Private(this) )
{
	//HL_DEBUG( logger, QString( "OpenGLRenderThread created" ).toStdString() );
}

OpenGLRenderThread::~OpenGLRenderThread()
{
	stop();
	wait();

	d->cleanup();
	//HL_DEBUG( logger, QString( "OpenGLRenderThread destroyed" ).toStdString() );
}

void OpenGLRenderThread::stop()
{
	d->rendering = false;
	d->queueWaitCondition.wakeAll();
}

int OpenGLRenderThread::registerSurface( OpenGLWindow *surface )
{
	QMutexLocker l( &d->dataMutex );
	int id = d->ids.fetchAndAddRelaxed( 1 );

	RenderClient *rc = new RenderClient;
	rc->surface = surface;

	d->clients.insert( id, rc );
	//HL_DEBUG( logger, QString( "New surface registered: %1" ).arg( id ).toStdString() );
	return id;
}

bool OpenGLRenderThread::removeSurface( int id )
{
	QMutexLocker l( &d->dataMutex );

	RenderClient *rc = d->clients.value( id, nullptr );
	if( !rc )
		return false;

	d->clients.remove( id );
	l.unlock();

	rc->mutex.lock();
	rc->surface = nullptr;
	rc->mutex.unlock();
	delete rc;

	return true;
}

int OpenGLRenderThread::subframeId( int id, int posx, int posy )
{
	QMutexLocker l( &d->dataMutex );
	RenderClient *rc = d->clients.value( id );
	if( !rc )
		return -1;

	if( !rc->renderSubframes )
		return -1;

	QList<int> keys = rc->subframeAreas.keys();
	foreach( const int &id, keys ) {
		QRect area = rc->subframeAreas.value( id, QRect() );
		if( area.isNull() )
			continue;
		if( area.contains( posx, posy, false ) )
			return id;
	}

	return -1;
}

bool OpenGLRenderThread::initialize( int id )
{
	RenderClient *rc = d->clients.value( id, nullptr );
	if( !rc || !rc->surface )
		return false;

	if( !rc->context ) {
		//HL_DEBUG( logger, QString( "Initialize context: %1" ).arg( id ).toStdString() );

		// Create context.
		rc->context.reset( new QOpenGLContext() );
		rc->context->setFormat( rc->surface->requestedFormat() );
		rc->context->create();

		// Initialize GL functions.
		rc->context->makeCurrent( rc->surface );
		rc->functions.reset( new QOpenGLFunctions() );
		rc->functions->initializeOpenGLFunctions();


		// Create shader program.
		rc->shaderProgram.reset( new QOpenGLShaderProgram() );

		// Initialize shaders.
		QString vertex( ":/opengl/vertex.vsh" );
		QString fragment( ":/opengl/fragment.fsh" );
		rc->shaderProgram->addShaderFromSourceFile( QOpenGLShader::Vertex, vertex );
		rc->shaderProgram->addShaderFromSourceFile( QOpenGLShader::Fragment, fragment );
		rc->shaderProgram->link();

		QColor backgroundColor = rc->surface->backgroundColor();
		glClearColor(
			(float)backgroundColor.red() / 255.0f,
			(float)backgroundColor.green() / 255.0f,
			(float)backgroundColor.blue() / 255.0f,
			1.0f
		);

		glDisable( GL_DEPTH_TEST );
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		// Setup variables.

		glGenTextures( 3, rc->textureIds ); // Obtain an id for the texture
		for( int i = 0; i < 3; i++ ) {
			glBindTexture( GL_TEXTURE_RECTANGLE_ARB, rc->textureIds[i] ); // Set as the current texture
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		}

		// Set some more render settings.
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		glEnable( GL_TEXTURE_RECTANGLE_ARB ); // Enable texturing
		glShadeModel( GL_FLAT );

		const GLubyte* pGPU = glGetString( GL_RENDERER );
		const GLubyte* pVersion = glGetString( GL_VERSION );
		const GLubyte* pShaderVersion = glGetString( GL_SHADING_LANGUAGE_VERSION );

		qDebug() << "GLFrame initialized - GPU" << QString( (char*)pGPU )
			<< ", OpenGL" << QString( (char*)pVersion )
			<< ", GLSL" << QString( (char*)pShaderVersion );
	}

	return true;
}

void OpenGLRenderThread::run()
{
	d->rendering = true;

	while( d->rendering ) {

		// Check queue for next client to render.
		QMutexLocker queueLocker( &d->queueMutex );

		if( d->queue.isEmpty() ) {
			//HL_DEBUG( logger, QString( "Queue going to sleep" ).toStdString() );

			// Wait for more data.
			d->queueWaitCondition.wait( &d->queueMutex );

			//HL_DEBUG( logger, QString( "Queue woken up" ).toStdString() );

			// In case there is still no data, restart loop.
			if( d->queue.isEmpty() ) {
				queueLocker.unlock();
				continue;
			}
		}

		int id = d->queue.dequeue();
		queueLocker.unlock();

		// Lock data mutex.
		QMutexLocker dataLocker( &d->dataMutex );

		// Get client and check for validity.
		RenderClient *rc = d->clients.value( id, nullptr );
		if( !rc || !rc->surface ) {
			continue;
		}

		// Check surface exposure.
		if( !rc->surface->isExposed() ) {
			//HL_DEBUG( logger, QString( "Surface not exposed: %1" ).arg( id ).toStdString() );
			continue;
		}

		// Check initialization of context, OpenGL functions and shaders.
		if( !initialize( id ) ) {
			//HL_DEBUG( logger, QString( "Initialization failed: %1" ).arg( id ).toStdString() );
			continue;
		}

		// Release lock.
		dataLocker.unlock();

		// Lock down client.
		if( rc->mutex.tryLock( 1 ) ) {
			// Last safety check.
			if( !rc || !rc->surface )
				continue;
			
			//HL_DEBUG( logger, QString( "Rendering: %1" ).arg( id ).toStdString() );

			// Everything checks out. Start render routine.
			render( rc );

			rc->mutex.unlock();
		} else {
			//HL_DEBUG( logger, QString( "Could not lock client: %1" ).arg( id ).toStdString() );
			update( id );
		}
	}
}

void OpenGLRenderThread::render( RenderClient *rc )
{
	rc->context->makeCurrent( rc->surface );

	// Lock mutex.
	QMutexLocker l( &d->dataMutex );

	// Copy widget dimensions.
	int widgetWidth = rc->width;
	int widgetHeight = rc->height;

	// Resize context if necessary.
	if( rc->doResize ) {
		glViewport( 0, 0, widgetWidth, widgetHeight );
		glMatrixMode( GL_PROJECTION );

		glLoadIdentity();
		glOrtho( 0, widgetWidth, 0, widgetHeight, 0, 1 );

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		rc->doResize = false;
	}

	// Clear render surface.
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();

	if( widgetWidth <= 0 || widgetHeight <= 0 ) {
		rc->context->swapBuffers( rc->surface );
		rc->context->doneCurrent();
		rc->frameCounter++;
		return;
	}

	// Declare/initialize variables for basic render calculation.
	bool mirrored = false;
	float widgetRatio = (float)widgetWidth / (float)widgetHeight;
	float imgRatio, scale, texleft, texright;
	int offsetX = 0, offsetY= 0, width = 0, height = 0;

	QScopedPointer<YuvFrame> data;
	RenderClient::RenderFrame *rf = nullptr;

	// Check for available primary frame.
	if( rc->selectedId >= 0 ) {
		rf = rc->frames.value( rc->selectedId );
		if( rf && !rf->frame.isNull() ) {
			data.reset( rf->frame->copy() );
			mirrored = rf->mirrored;
		}
	}
	l.unlock();

	rc->shaderProgram->bind();

	if( data ) {
		// Calculate texture position to keep aspect ratio of image.
		widgetRatio = (float)widgetWidth / (float)widgetHeight;
		imgRatio = (float)data->width / (float)data->height;
		scale = 1.0f;
		offsetX = 0;
		offsetY = 0;
		width = rc->width;
		height = widgetHeight;

		int visibleX = 0;
		int visibleY = 0;
		int visibleWidth = data->width;
		int visibleHeight = data->height;

		if( rc->renderMode == OpenGLWindow::RM_CENTERED ) {
			if( widgetRatio < imgRatio ) { // Center Y. 4:3 < 16:9
				scale = (float)widgetWidth / (float)data->width;
				height = data->height * scale;
				offsetY = (widgetHeight - height) / 2;
			} else { // Center X. 16:9 > 4:3
				scale = (float)widgetHeight / (float)data->height;
				width = data->width * scale;
				offsetX = (widgetWidth - width) / 2;
			}
		} else {
			if( widgetRatio < imgRatio ) { // Crop X. 4:3 < 16:9
				scale = (float)widgetHeight / (float)data->height;
				width = data->width * scale;
				offsetX = (widgetWidth - width) / 2;

				visibleWidth = data->height * widgetRatio;
				visibleX = (data->width - visibleWidth) / 2;
			} else { // Crop Y. 16:9 > 4:3
				scale = (float)widgetWidth / (float)data->width;
				height = data->height * scale;
				offsetY = (widgetHeight - height) / 2;

				visibleHeight = data->width / widgetRatio;
				visibleY = (data->height - visibleHeight) / 2;
			}
		}
		
		// Calculate dark edge overlay.
		if( rc->renderDarkEdge )
			data->overlayDarkEdge( visibleX, visibleY, visibleWidth, visibleHeight );

		// Set additional values for shader.
		rc->shaderProgram->setUniformValue( "height", (GLfloat)data->height );

		// Set textures.
		rc->functions->glActiveTexture( GL_TEXTURE1 ); // U plane.
		glBindTexture( GL_TEXTURE_RECTANGLE_ARB, rc->textureIds[1] );
		glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, data->width >> 1, data->height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data->u );
		rc->shaderProgram->setUniformValue( "texU", 1 );

		rc->functions->glActiveTexture( GL_TEXTURE2 ); // V plane.
		glBindTexture( GL_TEXTURE_RECTANGLE_ARB, rc->textureIds[2] );
		glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, data->width >> 1, data->height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data->v );
		rc->shaderProgram->setUniformValue( "texV", 2 );

		rc->functions->glActiveTexture( GL_TEXTURE0 ); // Y plane.
		glBindTexture( GL_TEXTURE_RECTANGLE_ARB, rc->textureIds[0] );
		glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, data->width, data->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data->y );
		rc->shaderProgram->setUniformValue( "texY", 0 );

		// Setup mirror.
		if( mirrored ) {
			texleft = (float)data->width;
			texright = 0;
		} else {
			texleft = 0;
			texright = (float)data->width;
		}

		// Draw canvas.
		glBegin( GL_QUADS );
			glNormal3f( 0.0, 0.0, 1.0 ); glTexCoord2f( texleft, 0 );					glVertex3i( offsetX, offsetY, 0 );
			glNormal3f( 0.0, 0.0, 1.0 ); glTexCoord2f( texright, 0 );					glVertex3i( offsetX + width, offsetY, 0 );
			glNormal3f( 0.0, 0.0, 1.0 ); glTexCoord2f( texright, (float)data->height );	glVertex3i( offsetX + width, offsetY + height, 0 );
			glNormal3f( 0.0, 0.0, 1.0 ); glTexCoord2f( texleft, (float)data->height );	glVertex3i( offsetX, offsetY + height, 0 );
		glEnd();
	}

	// Relock
	l.relock();

	// Clear sub-frame area memory.
	rc->subframeAreas.clear();

	// Render secondary frames.
	int numFrames = rc->ids.size();
	if( rc->renderSubframes && numFrames > 0 ) {
		// Determine sub-frame height and maximum width.
		height = widgetHeight / 4;
		int maxWidth = (widgetWidth - 3) / numFrames - 3;

		// Calculate individual width.
		QHash<int,int> widths;
		QHash<int,int> crop;
		int sumWidth = 0;

		foreach( const int &id, rc->ids ) {
			rf = rc->frames.value( id );
			if( !rf || rf->frame.isNull() )
				continue;

			scale = (float)height / (float)rf->frame->height;
			width = rf->frame->width * scale;

			int imgOffset = 0;
			if( width > maxWidth ) {
				scale = (float)maxWidth / (float)width;
				int scaledWidth = rf->frame->width * scale;
				imgOffset = (rf->frame->width - scaledWidth) / 2;

				width = maxWidth;
			}

			widths.insert( id, width );
			sumWidth += width + 3;
			crop.insert( id, imgOffset);
		}

		// Calculate positions.
		sumWidth -= 3;
		offsetX = (((widgetWidth - 6) - sumWidth) / 2) + 3;
		offsetY = 3;

		// Draw sub-frames.
		foreach( const int &id, rc->ids ) {
			width = widths.value( id );

			// Relock for coming iteration.
			l.relock();
			rf = rc->frames.value( id );

			// Create sub-frame copy.
			if( !rf || rf->frame.isNull() ) // Just to be safe.
				data.reset( YuvFrame::createBlackImage( width, height ) );
			else
				data.reset( rf->frame->copy() );

			// Remember sub-frame area.
			QRect area( offsetX, widgetHeight - height - offsetY, width, height );
			rc->subframeAreas.insert( id, area );
			l.unlock();

			/* Render actual frame. */
			// Set textures.
			rc->functions->glActiveTexture( GL_TEXTURE1 ); // U plane.
			glBindTexture( GL_TEXTURE_RECTANGLE_ARB, rc->textureIds[1] );
			glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, data->width >> 1, data->height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data->u );
			rc->shaderProgram->setUniformValue( "texU", 1 );

			rc->functions->glActiveTexture( GL_TEXTURE2 ); // V plane.
			glBindTexture( GL_TEXTURE_RECTANGLE_ARB, rc->textureIds[2] );
			glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, data->width >> 1, data->height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data->v );
			rc->shaderProgram->setUniformValue( "texV", 2 );

			rc->functions->glActiveTexture( GL_TEXTURE0 ); // Y plane.
			glBindTexture( GL_TEXTURE_RECTANGLE_ARB, rc->textureIds[0] );
			glTexImage2D( GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, data->width, data->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data->y );
			rc->shaderProgram->setUniformValue( "texY", 0 );

			// Set additional values for shader.
			rc->shaderProgram->setUniformValue( "height", (GLfloat)data->height );

			int imgOffset = crop.value( id );
			if( rf->mirrored ) {
				texleft = (float)(data->width - imgOffset);
				texright = (float)(0 + imgOffset);
			} else {
				texleft = (float)(0 + imgOffset);
				texright = (float)(data->width - imgOffset);
			}

			// Draw canvas.
			glBegin( GL_QUADS );
				glNormal3f( 0.0, 0.0, 1.0 ); glTexCoord2f( texleft, 0 );					glVertex3i( offsetX, offsetY, 0 );
				glNormal3f( 0.0, 0.0, 1.0 ); glTexCoord2f( texright, 0 );					glVertex3i( offsetX + width, offsetY, 0 );
				glNormal3f( 0.0, 0.0, 1.0 ); glTexCoord2f( texright, (float)data->height );	glVertex3i( offsetX + width, offsetY + height, 0 );
				glNormal3f( 0.0, 0.0, 1.0 ); glTexCoord2f( texleft, (float)data->height );	glVertex3i( offsetX, offsetY + height, 0 );
			glEnd();

			// Move offset forward.
			offsetX += width + 3;
		}
	}

	l.unlock();

	// Clean-up.
	rc->shaderProgram->release();

	rc->context->swapBuffers( rc->surface );
	rc->context->doneCurrent();
	rc->frameCounter++;
}

void OpenGLRenderThread::resize( int id, const QSize &size )
{
	QMutexLocker dl( &d->dataMutex );
	RenderClient *rc = d->clients.value( id );
	if( !rc )
		return;

	rc->width = size.width();
	rc->height = size.height();
	rc->doResize = true;
	dl.unlock();

	update( id );
}

void OpenGLRenderThread::update( int id )
{
	QMutexLocker ql( &d->queueMutex );
	if( !d->queue.contains( id ) ) {
		d->queue.enqueue( id );
		d->queueWaitCondition.wakeAll();
	}
}

void OpenGLRenderThread::setData( int id, QSharedPointer<YuvFrame> data, int frameId )
{
	QMutexLocker dl( &d->dataMutex );
	RenderClient *rc = d->clients.value( id );
	if( !rc )
		return;

	RenderClient::RenderFrame *rf = rc->getOrCreateRenderFrame( frameId );
	rf->frame = data;
	if( !rc->renderSubframes || rc->selectedId < 0 )
			rc->selectedId = frameId;
	dl.unlock();

	QMutexLocker ql( &d->queueMutex );
	if( !d->queue.contains( id ) ) {
		d->queue.enqueue( id );
		d->queueWaitCondition.wakeAll();
	}
}

void OpenGLRenderThread::setMirrorEnabled( int id, bool enabled, int frameId )
{
	QMutexLocker dl( &d->dataMutex );
	RenderClient *rc = d->clients.value( id );
	if( !rc )
		return;

	RenderClient::RenderFrame *rf = rc->getOrCreateRenderFrame( frameId );
	rf->mirrored = enabled;
	dl.unlock();

	update( id );
}

void OpenGLRenderThread::setRenderMode( int id, OpenGLWindow::RenderMode renderMode )
{
	QMutexLocker dl( &d->dataMutex );
	RenderClient *rc = d->clients.value( id );
	if( !rc )
		return;

	rc->renderMode = renderMode;
	dl.unlock();

	update( id );
}

void OpenGLRenderThread::setDarkEdgeEnabled( int id, bool enabled )
{
	QMutexLocker dl( &d->dataMutex );
	RenderClient *rc = d->clients.value( id );
	if( !rc )
		return;

	rc->renderDarkEdge = enabled;
	dl.unlock();

	update( id );
}

void OpenGLRenderThread::setSubframesEnabled( int id, bool enabled )
{
	QMutexLocker dl( &d->dataMutex );
	RenderClient *rc = d->clients.value( id );
	if( !rc )
		return;

	rc->renderSubframes = enabled;
	dl.unlock();

	update( id );
}

void OpenGLRenderThread::setActive( int id, int frameId )
{
	QMutexLocker dl( &d->dataMutex );
	RenderClient *rc = d->clients.value( id );
	if( !rc )
		return;

	rc->selectedId = frameId;
	dl.unlock();

	update( id );
}

void OpenGLRenderThread::removeSubFrame( int id, int frameId )
{
	QMutexLocker dl( &d->dataMutex );
	RenderClient *rc = d->clients.value( id );
	if( !rc )
		return;

	rc->removeRenderFrame( frameId );
	dl.unlock();

	update( id );
}
