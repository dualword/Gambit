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

#include "UpdateChecker.hh"
#include "debugf.h"
#include "UpdateCheckResult.hh"
#include "project_info.h"
#include "Utils/Cast.hh"
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkProxyFactory>
#include <QNetworkReply>
#include <QThread>
#include <QTimer>
#include <algorithm>
#include <cassert>
#include <cstdio>
using Utils::Cast::enforce_dynamic_cast;

QEvent::Type UpdateChecker::ProxyAuthentication = QEvent::None;

UpdateChecker::UpdateChecker()
    : workerThread_(0),
      networkAccessManager_(0),
      redirectionCount_(0),
      cancellationTimer_(0),
      authenticationAlreadyTried_(false)
{
    registerEventTypes();

    // Allow Qt::QueuedConnection on our finished() signal.
    qRegisterMetaType<UpdateCheckResult>("UpdateCheckResult");

    // If possible, use HTTP instead of HTTPS in URLs, so the update check works on Windows 2000 as
    // well. Otherwise, we may get QNetworkReply::UnknownNetworkError on Windows 2000.
    urlsToTry_.push_back("http://purl.org/net/gambit/latest");
    urlsToTry_.push_back("http://svn.code.sourceforge.net/p/gambitchess/code/softupdate/latest");

    // Shuffle the URLs, so we are less likely to have a problem if one or more of them aren't
    // working (for whatever length of time).
    std::random_shuffle(urlsToTry_.begin(), urlsToTry_.end());

    workerThread_ = new QThread;
    connect(workerThread_, SIGNAL(started()), this, SLOT(begin()));
    connect(workerThread_, SIGNAL(finished()), workerThread_, SLOT(deleteLater()));
}

UpdateChecker::~UpdateChecker()
{
    debugf("~UpdateChecker()\n");

    cleanup();
}

void UpdateChecker::authenticateToProxy(const QString &user, const QString &password)
{
    // Rather than doing what we want to do, we'll have to post an event to ourself and do it
    // later.
    // _This_ function is meant to be called by the receiver of our proxyAuthenticationRequired()
    // signal, and so we run in the thread of our receiver when this function is called.
    // Thus, we post an event, so we can do what we want to do from our own thread.
    QCoreApplication::postEvent(this, new ProxyAuthenticationEvent(user, password));
}

void UpdateChecker::start()
{
    assert(workerThread_);
    moveToThread(workerThread_);
    workerThread_->start();
}

void UpdateChecker::begin()
{
    // Initialization of the QNetworkAccessManager() can take a little while. Hence we do this in a
    // thread, as we don't want to block the GUI.
    assert(!networkAccessManager_);
    networkAccessManager_ = new QNetworkAccessManager(this);
    connect(
        networkAccessManager_,
        SIGNAL(finished(QNetworkReply *)),
        this,
        SLOT(replyFinished(QNetworkReply *)));

    selectProxy();

    cancellationTimer_ = new QTimer(this);
    cancellationTimer_->setSingleShot(true);
    cancellationTimer_->setInterval(CancellationTimeoutMilliseconds);
    connect(
        cancellationTimer_,
        SIGNAL(timeout()),
        this,
        SLOT(cancel()));
    cancellationTimer_->start();

    redirectionCount_ = 0;
    tryNextUrl();
}

void UpdateChecker::cancel()
{
    sendResult(
        UpdateCheckResult(
            UpdateCheckResult::Error,
            tr("No response received.")));
}

void UpdateChecker::replyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    const QNetworkReply::NetworkError error = reply->error();

    debugf("UpdateChecker::replyFinished(): reply->error() = %d\n", error);

    switch (error)
    {
    case QNetworkReply::NoError:
        // We're fine. Continue processing.
        break;

    case QNetworkReply::ProxyAuthenticationRequiredError:
    {
        // WORKAROUND for a Qt bug.
        // QNetworkAccessManager keeps using the wrong username and password combination when they
        // were invalid the first time.
        // This may or may not happen depending on whether the correct username was supplied the
        // first time.
        // Rather than uselessly asking the user again for credentials, we simply stop.
        if (authenticationAlreadyTried_)
        {
            // It's actually more accurate to say that the proxy server rejected our authentication
            // attempt, which may be due to Qt not supporting the authentication method required by
            // the server (for example NTLM version 2).
            // However, it's simpler to just assume the username and password combination was
            // incorrect.
            sendResult(
                UpdateCheckResult(
                    UpdateCheckResult::Error,
                    tr(
                        "Either the proxy server rejected the username and password combination,"
                        " or the required authentication method is unsupported.")));
            return;
        }

        assert(networkAccessManager_);

        QNetworkProxy proxy = networkAccessManager_->proxy();

        // We don't want to cancel the update check whilst the receiver of our signal is showing
        // the proxy authentication dialog.
        cancellationTimer_->stop();

        emit proxyAuthenticationRequired(proxy);

        return;
    }

    default:
        // Try the next URL (if any), maybe that one works.
        if (!tryNextUrl())
        {
            if (error == QNetworkReply::HostNotFoundError)
            {
                sendResult(
                    UpdateCheckResult(
                        UpdateCheckResult::Error,
                        tr("Could not find the hostname of the update (or proxy) server.")));
            }
            else
            {
                sendUnspecifiedErrorResult();
            }
        }
        return;
    }

    assert(error == QNetworkReply::NoError);

    const QString &possibleRedirectUrl =
        reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();

    if (!possibleRedirectUrl.isEmpty())
    {
        if (redirectionCount_ == RedirectionLimit)
        {
            sendResult(
                UpdateCheckResult(
                    UpdateCheckResult::Error,
                    tr("Too many HTTP redirections.")));
            return;
        }

        redirectionCount_++;
        performRequest(possibleRedirectUrl);
    }
    else
    {
        if (!processResponse(reply->readAll()))
        {
            // This response did not make sense. Try the next URL (if any).
            if (!tryNextUrl())
                sendUnspecifiedErrorResult();
        }
    }
}

void UpdateChecker::cleanup()
{
    destroyNetworkAccessManager();

    assert(cancellationTimer_);
    cancellationTimer_->stop();

    assert(workerThread_);
    workerThread_->quit();
}

void UpdateChecker::destroyNetworkAccessManager()
{
    if (networkAccessManager_)
    {
        // It's safest to destroy the object later, rather than now, as we may have been called
        // directly by the object (by it emitting a signal, and our slot calling _this_ function).
        networkAccessManager_->deleteLater();
        networkAccessManager_ = 0;
    }
}

bool UpdateChecker::event(QEvent *ev)
{
    QEvent::Type t = ev->type();

    if (t == ProxyAuthentication)
    {
        ProxyAuthenticationEvent *o = enforce_dynamic_cast<ProxyAuthenticationEvent *>(ev);

        assert(networkAccessManager_);

        QNetworkProxy proxy = networkAccessManager_->proxy();
        proxy.setUser(o->user);
        proxy.setPassword(o->password);
        networkAccessManager_->setProxy(proxy);
        authenticationAlreadyTried_ = true;

        assert(cancellationTimer_->interval() == CancellationTimeoutMilliseconds);
        cancellationTimer_->start();

        // Retry the request, now with proxy credentials.
        performRequest(currentUrl_);

        return true;
    }
    else
    {
        return QObject::event(ev);
    }
}

void UpdateChecker::performRequest(const QString &url)
{
    assert(networkAccessManager_);
    networkAccessManager_->get(QNetworkRequest(QUrl(url)));
}

// Returns 'false' if and only if the response made no sense.
bool UpdateChecker::processResponse(const QString &response)
{
    if (response.isEmpty())
        return false;

    bool ok = false;
    const int latestAvailableRevision = response.toUInt(&ok);
    if (!ok)
        return false;

    int resultCode = UpdateCheckResult::NoUpdateAvailable;

    if (latestAvailableRevision > APP_INTERNAL_REVISION)
        resultCode = UpdateCheckResult::UpdateAvailable;

    sendResult(UpdateCheckResult(resultCode));

    return true;
}

void UpdateChecker::selectProxy()
{
    assert(networkAccessManager_);
    assert(!urlsToTry_.empty());

    QUrl url(urlsToTry_.front());

    QList<QNetworkProxy> proxies =
        QNetworkProxyFactory::systemProxyForQuery(
            QNetworkProxyQuery(url));

    if (proxies.size() != 0)
        networkAccessManager_->setProxy(proxies.first());
}

void UpdateChecker::sendResult(const UpdateCheckResult &result)
{
    // Destroy the QNetworkAccessManager, not only because we need to do so anyway at some point,
    // but also because it disconnects us from its signals.
    // We don't want to get signals from it, in case the receiver of our signal, for instance,
    // shows a message box and deletes us afterwards (rather than before, which would also
    // disconnect us, since we get destroyed). Showing a message box would continue the event loop,
    // and thus we would still receive signals.
    cleanup(); // Destroys the QNetworkAccessManager, among others.

    emit finished(result);
}

void UpdateChecker::sendUnspecifiedErrorResult()
{
    sendResult(
        UpdateCheckResult(
            UpdateCheckResult::Error,
            tr("An unspecified error occurred.")));
}

bool UpdateChecker::tryNextUrl()
{
    if (urlsToTry_.empty())
    {
        currentUrl_.clear();
        return false;
    }

    currentUrl_ = urlsToTry_.front();

    urlsToTry_.erase(urlsToTry_.begin());

    performRequest(currentUrl_);

    return true;
}

void UpdateChecker::registerEventTypes()
{
    if (ProxyAuthentication == QEvent::None)
        ProxyAuthentication = static_cast<QEvent::Type>(QEvent::registerEventType());
}

UpdateChecker::ProxyAuthenticationEvent::ProxyAuthenticationEvent(
    const QString &_user,
    const QString &_password)
    : QEvent(ProxyAuthentication),
      user(_user),
      password(_password)
{
    assert(ProxyAuthentication != QEvent::None);
}
