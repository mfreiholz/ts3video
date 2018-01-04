#if defined(OCS_INCLUDE_OPENGL)
#include "videowidget_opengl.h"

#include <QGLFormat>
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include "libapp/elws.h"

class YuvVideoOpenGLWidget::Private
{
public:
	YuvFrameRefPtr yuvFrame;
	QImage rgbImage;

	QOpenGLShaderProgram* program;
	unsigned int textureIds[3];
};

YuvVideoOpenGLWidget::YuvVideoOpenGLWidget(QWidget* parent, Qt::WindowFlags f):
	QOpenGLWidget(parent, f),
	VideoWidgetI(),
	d(new Private())
{
	QSurfaceFormat format;
	format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	format.setRenderableType(QSurfaceFormat::OpenGL);
	format.setProfile(QSurfaceFormat::CompatibilityProfile);
	setFormat(format);
	//setAttribute(Qt::WA_OpaquePaintEvent);
}

YuvVideoOpenGLWidget::~YuvVideoOpenGLWidget()
{
	makeCurrent();

	delete d->program;
	d->program = 0;

	doneCurrent();
}

void
YuvVideoOpenGLWidget::setFrame(YuvFrameRefPtr frame)
{
	d->yuvFrame = frame;
	update();
}

void
YuvVideoOpenGLWidget::setFrame(const QImage& frame)
{
	d->rgbImage = frame;
	update();
}

void
YuvVideoOpenGLWidget::initializeGL()
{
	d->program = new QOpenGLShaderProgram(this);
	d->program->addShaderFromSourceFile(QOpenGLShader::Vertex,
										":/opengl/vertex.vsh");
	d->program->addShaderFromSourceFile(QOpenGLShader::Fragment,
										":/opengl/fragment-win.fsh");
	d->program->link();


	QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();

	// Misc stuff.
	gl->glDisable(GL_DEPTH_TEST);
	gl->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Init textures.
	gl->glGenTextures(3, d->textureIds);
	for (auto i = 0; i < 3; ++i)
	{
		gl->glBindTexture(GL_TEXTURE_RECTANGLE_ARB, d->textureIds[i]);
		gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	// Stuff..
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	gl->glEnable(GL_TEXTURE_RECTANGLE_ARB); // Enable texturing
	glShadeModel(GL_FLAT);

	// Status infos.
	//auto infoVendor = QString((char*)glGetString(GL_VENDOR));
	//auto infoRenderer = QString((char*)glGetString(GL_RENDERER));
	//auto infoVersion = QString((char*)glGetString(GL_VERSION));
	//auto infoShaderVersion = QString((char*)glGetString(
	//									 GL_SHADING_LANGUAGE_VERSION));
	//HL_DEBUG(HL,
	//		 QString("OpenGL Video Window Initialized (vendor=%1; renderer=%2; version=%3; shader-version=%4)").arg(
	//			 infoVendor).arg(infoRenderer).arg(infoVersion).arg(
	//			 infoShaderVersion).toStdString());
}

void
YuvVideoOpenGLWidget::resizeGL(int w, int h)
{
	glLoadIdentity();
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	glOrtho(0.0, w, 0, h, 0, 1);
	glMatrixMode(GL_MODELVIEW);

	//glLoadIdentity();
}

void
YuvVideoOpenGLWidget::paintGL()
{
	auto gl = QOpenGLContext::currentContext()->functions();

	gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto frame = d->yuvFrame; //YuvFrameRefPtr(_frame->copy());
	if (!frame)
		return;

	// Bind shader.
	glLoadIdentity();
	d->program->bind();
	d->program->setUniformValue("height", (GLfloat)frame->height);

	// Set texture: U plane
	gl->glActiveTexture(GL_TEXTURE1);
	gl->glBindTexture(GL_TEXTURE_RECTANGLE_ARB, d->textureIds[1]);
	gl->glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, frame->width >> 1,
					 frame->height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->u);
	d->program->setUniformValue("texU", 1);

	// Set texture: V plane
	gl->glActiveTexture(GL_TEXTURE2);
	gl->glBindTexture(GL_TEXTURE_RECTANGLE_ARB, d->textureIds[2]);
	gl->glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, frame->width >> 1,
					 frame->height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->v);
	d->program->setUniformValue("texV", 2);

	// Set texture: Y plane
	gl->glActiveTexture(GL_TEXTURE0);
	gl->glBindTexture(GL_TEXTURE_RECTANGLE_ARB, d->textureIds[0]);
	gl->glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, frame->width,
					 frame->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->y);
	d->program->setUniformValue("texY", 0);

	// Draw canvas.
	int offsetX = 0;
	int offsetY = 0;
	int width = this->width();
	int height = this->height();
	float texleft = 0;
	float texright = (float)frame->width;

	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);
	glTexCoord2f(texleft, 0);
	glVertex3i(offsetX, offsetY, 0);
	glNormal3f(0.0, 0.0, 1.0);
	glTexCoord2f(texright, 0);
	glVertex3i(offsetX + width, offsetY, 0);
	glNormal3f(0.0, 0.0, 1.0);
	glTexCoord2f(texright, (float)frame->height);
	glVertex3i(offsetX + width, offsetY + height, 0);
	glNormal3f(0.0, 0.0, 1.0);
	glTexCoord2f(texleft, (float)frame->height);
	glVertex3i(offsetX, offsetY + height, 0);
	glEnd();

	// Cleanup.
	glFinish();
	d->program->release();
}

#endif