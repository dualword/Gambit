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

#ifndef PROXY_AUTHENTICATION_DIALOG_HH
#define PROXY_AUTHENTICATION_DIALOG_HH

#include "../../ui_ProxyAuthenticationDialog.h"

class ProxyAuthenticationDialog : public QDialog, private Ui::ProxyAuthenticationDialog
{
    Q_OBJECT

public:
    ProxyAuthenticationDialog(const QString &infoText, QWidget *_parent = 0, Qt::WindowFlags f = 0);

    QString password() const;
    QString user() const;

public slots:
    void done(int r);

private slots:
    void on_showPasswordCheckBox_stateChanged(int state);

private:
    void retranslateUi();
    void setupUi(const QString &infoText);

    QString password_;
    QString user_;
};

#endif
