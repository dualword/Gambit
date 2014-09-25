/*
    Written by Jelle Geerts (jellegeerts@gmail.com).

    To the extent possible under law, the author(s) have dedicated all
    copyright and related and neighboring rights to this software to
    the public domain worldwide. This software is distributed without
    any warranty.

    You should have received a copy of the CC0 Public Domain Dedication
    along with this software.
    If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include "GraphicsView.hh"
#include "Core/debugf.h"
#include "Core/Preferences.hh"

#ifdef CONFIG_QT_OPENGL
# include <QGLWidget>
#endif /* defined(CONFIG_QT_OPENGL) */
#include <QResizeEvent>
#include <cassert>
#include <cstdio>

GraphicsView::GraphicsView(QGraphicsScene *_scene, Preferences &preferences)
    : usingOpenGL_(false)
{
    setFrameShape(QFrame::NoFrame);

#ifdef CONFIG_QT_OPENGL
    if (preferences.graphicsUseHardwareAcceleration() &&
        QGLFormat::hasOpenGL())
    {
        // Hope that hardware acceleration is available for OpenGL.
        setViewport(new QGLWidget);
        //setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
        usingOpenGL_ = true;
    }
#endif /* defined(CONFIG_QT_OPENGL) */

    setScene(_scene);
}

GraphicsView::~GraphicsView()
{
    debugf("~GraphicsView()\n");
    assert(scene());
    delete scene();
}

bool GraphicsView::usingOpenGL() const
{
    return usingOpenGL_;
}

void GraphicsView::resizeEvent(QResizeEvent *ev)
{
    // Not only do we want to resize the scene, it's also more efficient to tell the scene what
    // its size should be, as that way it doesn't have to find out using more expensive methods.
    scene()->setSceneRect(
        QRect(QPoint(0, 0), ev->size()));

    QGraphicsView::resizeEvent(ev);
}
