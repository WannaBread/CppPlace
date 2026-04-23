#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QLabel;

namespace cppplace::client {

/// Modal login/register dialog. Reports the user's choice via signals — it
/// doesn't talk to the network itself.
class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget* parent = nullptr);

    void setBusy(bool busy);
    void setError(const QString& message);

signals:
    void loginRequested(const QString& username, const QString& password);
    void registerRequested(const QString& username, const QString& password);

private:
    QLineEdit* user_   = nullptr;
    QLineEdit* pass_   = nullptr;
    QLabel*    status_ = nullptr;
};

} // namespace cppplace::client
