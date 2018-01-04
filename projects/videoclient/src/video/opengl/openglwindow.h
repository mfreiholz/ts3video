#if defined(OCS_INCLUDE_OPENGL)
#ifndef _OPENGLWINDOW_HEADER_
#define _OPENGLWINDOW_HEADER_

#include "QtCore/QObject"
#include "QtCore/QSharedPointer"
#include "QtGui/QWindow"
#include "QtGui/QOpenGLFunctions"

#include "libapp/yuvframe.h"

class QWidget;
class QResizeEvent;

class OpenGLWindow : public QWindow
{
	Q_OBJECT
	class Private;
	QScopedPointer<Private> d;

public:
	enum RenderMode { RM_CENTERED, RM_CROPPED };

	explicit OpenGLWindow(QWindow* parent = nullptr);
	~OpenGLWindow();

	QWidget* widget();

public slots:
	void setData(QSharedPointer<YuvFrame> data, int id = 0);
	void setActive(int id = 0);

	void setRenderMode(RenderMode renderMode);
	void setMirrorEnabled(bool enabled, int id = 0);
	void setDarkEdgeEnabled(bool enabled);
	void setSubframesEnabled(bool enabled);

	QColor backgroundColor() const;
	void setBackgroundColor(const QColor& color);

protected:
	bool event(QEvent* event);
	void exposeEvent(QExposeEvent* event);
	void resizeEvent(QResizeEvent* ev);
	void mousePressEvent(QMouseEvent* ev);

signals:
	void mouseClicked();
	void subframeClicked(int id);

};

#endif // _OPENGLWINDOW_HEADER_
#endif