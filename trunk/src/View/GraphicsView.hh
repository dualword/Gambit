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

#ifndef GRAPHICS_VIEW_HH
#define GRAPHICS_VIEW_HH

#include <QGraphicsView>

class Preferences;
class QGraphicsScene;

// The GraphicsView class exists and is used for having an easy and reliable way to make use of
// available hardware acceleration (by using a QGLWidget for our viewport).
//
// Features:
//     - Resizes the QGraphicsScene when resizeEvent() is called.
class GraphicsView : public QGraphicsView
{
public:
    GraphicsView(QGraphicsScene *_scene, Preferences &preferences);
    ~GraphicsView();

    bool usingOpenGL() const;

protected:
    void resizeEvent(QResizeEvent *ev);

    bool usingOpenGL_;
};

#endif
