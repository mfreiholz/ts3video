#ifndef GLVIDEOWIDGETPRIVATE_H
#define GLVIDEOWIDGETPRIVATE_H

#include "glvideowidget.h"

class GLVideoWidgetPrivate
{
public:
  GLVideoWidgetPrivate(GLVideoWidget *o) : owner(o)
  {}

public:
  GLVideoWidget *owner;
  class QOpenGLShaderProgram *program;
  unsigned int textureIds[3];
  YuvFrameRefPtr frame;
};


#endif