#if defined(OCS_INCLUDE_OPENGL)
#include "yuvvideowindow.h"

#include <QCoreApplication>
#include <QEvent>
#include <QPainter>
#include <QOpenGLShader>
#include <QOpenGLContext>
#include <QOpenGLPaintDevice>
#include <QOpenGLShaderProgram>

#include "humblelogging/api.h"

HUMBLE_LOGGER(HL, "client.opengl");

YuvVideoWindow::YuvVideoWindow(QWindow *parent) :
  QWindow(parent),
  _updatePending(false),
  _context(nullptr),
  _device(nullptr)
{
  setSurfaceType(QWindow::OpenGLSurface);
}

YuvVideoWindow::~YuvVideoWindow()
{
  delete _device;
}

void YuvVideoWindow::initialize()
{
}

void YuvVideoWindow::render()
{
  if (!_device) {
    _device = new QOpenGLPaintDevice();
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  
  _device->setSize(size());

  QPainter painter(_device);
  render(&painter);
}

void YuvVideoWindow::render(QPainter *painter)
{
  painter->setPen(QColor(Qt::white));
  painter->drawText(0, 0, 100, 100, 0, QString("Hallo"));
}

void YuvVideoWindow::renderLater()
{
  if (!_updatePending) {
    _updatePending = true;
    QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
  }
}

void YuvVideoWindow::renderNow()
{
  if (!isExposed()) {
    return;
  }

  bool needsInitialize = false;
  if (!_context) {
    _context = new QOpenGLContext(this);
    _context->setFormat(requestedFormat());
    _context->create();
    needsInitialize = true;
  }
  _context->makeCurrent(this);

  if (needsInitialize) {
    initializeOpenGLFunctions();
    initialize();
  }
  render();
  _context->swapBuffers(this);
  _context->doneCurrent();
}

bool YuvVideoWindow::event(QEvent *ev)
{
  switch (ev->type()) {
  case QEvent::UpdateRequest:
    _updatePending = false;
    renderNow();
    return true;
  }
  return QWindow::event(ev);
}

void YuvVideoWindow::exposeEvent(QExposeEvent *)
{
  if (!isExposed()) {
    return;
  }
  renderNow();
}

///////////////////////////////////////////////////////////////////////

YuvVideoWindowSub::YuvVideoWindowSub(QWindow *parent) :
  YuvVideoWindow(parent),
  _program(nullptr),
  _resize(false)
{
  setSurfaceType(QSurface::OpenGLSurface);

  QSurfaceFormat format;
  format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setProfile(QSurfaceFormat::CompatibilityProfile);
  setFormat(format);
  create();
}

YuvVideoWindowSub::~YuvVideoWindowSub()
{
  delete _program;
}

void YuvVideoWindowSub::setFrame(YuvFrameRefPtr frame)
{
  _frame = frame;
  renderLater();
}

void YuvVideoWindowSub::initialize()
{
  // Init shaders.
  _program = new QOpenGLShaderProgram(this);
  _program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/opengl/vertex.vsh");
  _program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/opengl/fragment.fsh");
  _program->link();

  // Init background.
  //QColor color(Qt::black);
  //glClearColor((float) color.red() / 255.0f, (float) color.green() / 255.0f, (float) color.blue() / 255.0f, 1.0f);

  // Misc stuff.
  glDisable(GL_DEPTH_TEST);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // Init textures.
  glGenTextures(3, _textureIds);
  for (auto i = 0; i < 3; ++i) {
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _textureIds[i]); // Sets as current texture.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }

  // Stuff..
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glEnable(GL_TEXTURE_RECTANGLE_ARB); // Enable texturing
  glShadeModel(GL_FLAT);

  // Status infos.
  auto infoVendor = QString((char*) glGetString(GL_VENDOR));
  auto infoRenderer = QString((char*) glGetString(GL_RENDERER));
  auto infoVersion = QString((char*) glGetString(GL_VERSION));
  auto infoShaderVersion = QString((char*) glGetString(GL_SHADING_LANGUAGE_VERSION));
  HL_DEBUG(HL, QString("OpenGL Video Window Initialized (vendor=%1; renderer=%2; version=%3; shader-version=%4)").arg(infoVendor).arg(infoRenderer).arg(infoVersion).arg(infoShaderVersion).toStdString());
}

void YuvVideoWindowSub::render()
{
  // Resize
  if (_resize) {
    glViewport(0, 0, width(), height());
    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();
    glOrtho(0.0, width(), 0, height(), 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    _resize = false;
  }

  glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (_frame.isNull()) {
    return;
  }

  auto frame = _frame; //YuvFrameRefPtr(_frame->copy());

  // Bind shader.
  glLoadIdentity();
  _program->bind();
  _program->setUniformValue("height", (GLfloat) frame->height);

  // Set texture: U plane
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _textureIds[1]);
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, frame->width >> 1, frame->height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->u);
  _program->setUniformValue("texU", 1);

  // Set texture: V plane
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _textureIds[2]);
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, frame->width >> 1, frame->height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->v);
  _program->setUniformValue("texV", 2);

  // Set texture: Y plane
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _textureIds[0]);
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, frame->width, frame->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->y);
  _program->setUniformValue("texY", 0);

  // Draw canvas.
  int offsetX = 0;
  int offsetY = 0;
  int width = this->width();
  int height = this->height();
  float texleft = 0;
  float texright = (float) frame->width;

  glBegin( GL_QUADS );
    glNormal3f( 0.0, 0.0, 1.0 ); glTexCoord2f( texleft, 0 );					glVertex3i( offsetX, offsetY, 0 );
    glNormal3f( 0.0, 0.0, 1.0 ); glTexCoord2f( texright, 0 );					glVertex3i( offsetX + width, offsetY, 0 );
    glNormal3f( 0.0, 0.0, 1.0 ); glTexCoord2f( texright, (float)frame->height );	glVertex3i( offsetX + width, offsetY + height, 0 );
    glNormal3f( 0.0, 0.0, 1.0 ); glTexCoord2f( texleft, (float)frame->height );	glVertex3i( offsetX, offsetY + height, 0 );
  glEnd();

  // Cleanup.
  _program->release();
}

void YuvVideoWindowSub::resizeEvent(QResizeEvent *ev)
{
  _resize = true;
}
#endif