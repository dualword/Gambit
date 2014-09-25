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

#include "ProxyAuthenticationDialog.hh"
#include "UI.hh"

ProxyAuthenticationDialog::ProxyAuthenticationDialog(
    const QString &infoText,
    QWidget *_parent /* = 0 */,
    Qt::WindowFlags f /* = 0 */)
    : QDialog(_parent, f)
{
    setupUi(infoText);
}

QString ProxyAuthenticationDialog::password() const
{
    return password_;
}

QString ProxyAuthenticationDialog::user() const
{
    return user_;
}

void ProxyAuthenticationDialog::done(int r)
{
    QDialog::done(r);

    if (r == QDialog::Accepted)
    {
        user_ = usernameLineEdit->text();
        password_ = passwordLineEdit->text();
    }
}

void ProxyAuthenticationDialog::on_showPasswordCheckBox_stateChanged(int state)
{
    passwordLineEdit->setEchoMode(
        state == Qt::Unchecked ? QLineEdit::Password : QLineEdit::Normal);
}

void ProxyAuthenticationDialog::retranslateUi()
{
    Ui::ProxyAuthenticationDialog::retranslateUi(this);
}

void ProxyAuthenticationDialog::setupUi(const QString &infoText)
{
    Ui::ProxyAuthenticationDialog::setupUi(this);

    setWindowTitle(UI::makeTitle(windowTitle()));

    infoLabel->setText(infoText);

    // WORKAROUND
    // Due to either a bug in Qt or an issue in X11, when word wrapping is used, the minimum sizes
    // of widgets are not respected, and all widgets become squeezed, and the window can be made
    // way too small by resizing.
    // A workaround is to use a fixed height for the widget that uses word wrapping.
#ifdef Q_WS_X11
    infoLabel->setFixedHeight(
        infoLabel->fontMetrics().lineSpacing() * 3);
#endif
}
