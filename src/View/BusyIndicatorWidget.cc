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

#include "BusyIndicatorWidget.hh"
#include "MissingFileDialog.hh"
#include "Core/ResourcePath.hh"
#include <QEvent>
#include <QMovie>

BusyIndicatorWidget::BusyIndicatorWidget()
{
    const QString &movieFileName = ResourcePath::mkQString("busy_indicator/busy_indicator.gif");
    movie_ = new QMovie(movieFileName);
    MissingFileDialog::instance().addIfNonExistent(movieFileName);

    // Jump to the first frame, so that when sizeForWidth() is called on the QLabel (remember, we
    // inherit QLabel), it will properly resize. If we don't do this, then the first time the movie
    // plays, the QLabel resizes, which would be a bit ugly.
    movie_->jumpToFrame(0);

    setMovie(movie_);
}

BusyIndicatorWidget::~BusyIndicatorWidget()
{
    // QLabel doesn't take ownership, we have to delete the movie ourself.
    delete movie_;
}

void BusyIndicatorWidget::on()
{
    movie_->start();
    update();
}

void BusyIndicatorWidget::off()
{
    movie_->stop();
    update();
}

bool BusyIndicatorWidget::event(QEvent *ev)
{
    switch (ev->type())
    {
    case QEvent::ToolTip:
        if (movie_->state() == QMovie::Running)
            setToolTip(tr("The chess engine is performing calculations."));
        else
            setToolTip("");
        // FALLTHROUGH
    default:
        return QWidget::event(ev);
    }
}

void BusyIndicatorWidget::paintEvent(QPaintEvent *ev)
{
    if (movie_->state() == QMovie::NotRunning)
    {
        // Movie should not be painted at all.
        return;
    }

    // Paint the movie.
    QLabel::paintEvent(ev);
}
