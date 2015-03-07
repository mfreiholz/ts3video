#include "glvideowidget_p.h"

#include <QOpenGLContext>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

#include "humblelogging/api.h"

HUMBLE_LOGGER(HL, "client.opengl");

///////////////////////////////////////////////////////////////////////

GLVideoWidget::GLVideoWidget(QWidget *parent, const QGLWidget *shareWidget, Qt::WindowFlags f) :
  QGLWidget(parent, shareWidget, f),
  d(new GLVideoWidgetPrivate(this))
{
  QSurfaceFormat format;
  format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
  format.setRenderableType(QSurfaceFormat::OpenGL);
  format.setProfile(QSurfaceFormat::CompatibilityProfile);
  setFormat(QGLFormat::fromSurfaceFormat(format));
}

GLVideoWidget::~GLVideoWidget()
{
}

void GLVideoWidget::setFrame(YuvFrameRefPtr frame)
{
  d->frame = frame;
  updateGL();
}

void GLVideoWidget::initializeGL()
{
  // Init shaders.
  d->program = new QOpenGLShaderProgram(this);
  d->program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/opengl/vertex.vsh");
  d->program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/opengl/fragment.fsh");
  d->program->link();

  // Misc stuff.
  glDisable(GL_DEPTH_TEST);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // Init textures.
  glGenTextures(3, d->textureIds);
  for (auto i = 0; i < 3; ++i) {
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, d->textureIds[i]); // Sets as current texture.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }

  // Stuff..
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glEnable(GL_TEXTURE_RECTANGLE_ARB); // Enable texturing
  glShadeModel(GL_FLAT);

  // Status infos.
  auto infoVendor = QString((char*)glGetString(GL_VENDOR));
  auto infoRenderer = QString((char*)glGetString(GL_RENDERER));
  auto infoVersion = QString((char*)glGetString(GL_VERSION));
  auto infoShaderVersion = QString((char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
  HL_DEBUG(HL, QString("OpenGL Video Window Initialized (vendor=%1; renderer=%2; version=%3; shader-version=%4)").arg(infoVendor).arg(infoRenderer).arg(infoVersion).arg(infoShaderVersion).toStdString());
}

void GLVideoWidget::resizeGL(int w, int h)
{
  glLoadIdentity();
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);

  glLoadIdentity();
  glOrtho(0.0, w, 0, h, 0, 1);
  glMatrixMode(GL_MODELVIEW);

  //glLoadIdentity();
}

void GLVideoWidget::paintGL()
{
  auto gl = QOpenGLContext::currentContext()->functions();

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (d->frame.isNull()) {
    return;
  }

  auto frame = d->frame; //YuvFrameRefPtr(_frame->copy());

  // Bind shader.
  glLoadIdentity();
  d->program->bind();
  d->program->setUniformValue("height", (GLfloat)frame->height);

  // Set texture: U plane
  gl->glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, d->textureIds[1]);
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, frame->width >> 1, frame->height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->u);
  d->program->setUniformValue("texU", 1);

  // Set texture: V plane
  gl->glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, d->textureIds[2]);
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, frame->width >> 1, frame->height >> 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->v);
  d->program->setUniformValue("texV", 2);

  // Set texture: Y plane
  gl->glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, d->textureIds[0]);
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, frame->width, frame->height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, frame->y);
  d->program->setUniformValue("texY", 0);

  // Draw canvas.
  int offsetX = 0;
  int offsetY = 0;
  int width = this->width();
  int height = this->height();
  float texleft = 0;
  float texright = (float)frame->width;

  glBegin(GL_QUADS);
  glNormal3f(0.0, 0.0, 1.0); glTexCoord2f(texleft, 0);					glVertex3i(offsetX, offsetY, 0);
  glNormal3f(0.0, 0.0, 1.0); glTexCoord2f(texright, 0);					glVertex3i(offsetX + width, offsetY, 0);
  glNormal3f(0.0, 0.0, 1.0); glTexCoord2f(texright, (float)frame->height);	glVertex3i(offsetX + width, offsetY + height, 0);
  glNormal3f(0.0, 0.0, 1.0); glTexCoord2f(texleft, (float)frame->height);	glVertex3i(offsetX, offsetY + height, 0);
  glEnd();

  // Cleanup.
  d->program->release();
}