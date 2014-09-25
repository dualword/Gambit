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

#ifndef UPDATE_CHECKER_HH
#define UPDATE_CHECKER_HH

#include <QEvent>
#include <QObject>
#include <vector>

class QNetworkAccessManager;
class QNetworkProxy;
class QNetworkReply;
class QThread;
class QTimer;
class UpdateCheckResult;

class UpdateChecker : public QObject
{
    Q_OBJECT

public:
    UpdateChecker();
    ~UpdateChecker();

    void start();
    void authenticateToProxy(const QString &user, const QString &password);

signals:
    void proxyAuthenticationRequired(const QNetworkProxy &proxy);
    void finished(const UpdateCheckResult &result);

private slots:
    void begin();
    void cancel();
    void replyFinished(QNetworkReply *reply);

private:
    struct ProxyAuthenticationEvent : public QEvent
    {
        ProxyAuthenticationEvent(const QString &_user, const QString &_password);

        QString user;
        QString password;
    };

    void cleanup();
    void destroyNetworkAccessManager();
    bool event(QEvent *ev);
    void performRequest(const QString &url);
    bool processResponse(const QString &response);
    void selectProxy();
    void sendResult(const UpdateCheckResult &result);
    void sendUnspecifiedErrorResult();
    bool tryNextUrl();

    static void registerEventTypes();

    enum
    {
        CancellationTimeoutMilliseconds = 15000,
        RedirectionLimit = 10
    };

    static QEvent::Type ProxyAuthentication;

    QThread *workerThread_;
    std::vector<QString> urlsToTry_;
    QString currentUrl_;
    QNetworkAccessManager *networkAccessManager_;
    int redirectionCount_;
    QTimer *cancellationTimer_;
    bool authenticationAlreadyTried_;
};

#endif
