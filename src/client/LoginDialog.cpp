#include "client/LoginDialog.hpp"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

namespace cppplace::client {

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("CppPlace — Sign in");
    setModal(true);
    setMinimumWidth(340);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 16);
    root->setSpacing(12);

    auto* title = new QLabel("CppPlace", this);
    title->setObjectName("loginTitle");
    title->setAlignment(Qt::AlignHCenter);
    root->addWidget(title);

    auto* subtitle = new QLabel("Place a pixel. Wait. Repeat.", this);
    subtitle->setObjectName("loginSubtitle");
    subtitle->setAlignment(Qt::AlignHCenter);
    root->addWidget(subtitle);
    root->addSpacing(8);

    user_ = new QLineEdit(this);
    user_->setPlaceholderText("username");
    pass_ = new QLineEdit(this);
    pass_->setPlaceholderText("password");
    pass_->setEchoMode(QLineEdit::Password);

    auto* form = new QFormLayout;
    form->setSpacing(8);
    form->addRow("User", user_);
    form->addRow("Pass", pass_);
    root->addLayout(form);

    status_ = new QLabel(this);
    status_->setObjectName("loginStatus");
    status_->setWordWrap(true);
    root->addWidget(status_);

    auto* btnLogin = new QPushButton("Sign in", this);
    btnLogin->setObjectName("primaryButton");
    btnLogin->setDefault(true);
    auto* btnReg   = new QPushButton("Register", this);
    btnReg->setObjectName("secondaryButton");

    auto* row = new QHBoxLayout;
    row->addWidget(btnReg);
    row->addStretch();
    row->addWidget(btnLogin);
    root->addLayout(row);

    connect(btnLogin, &QPushButton::clicked, this, [this]() {
        emit loginRequested(user_->text(), pass_->text());
    });
    connect(btnReg, &QPushButton::clicked, this, [this]() {
        emit registerRequested(user_->text(), pass_->text());
    });
}

void LoginDialog::setBusy(bool busy) {
    user_->setEnabled(!busy);
    pass_->setEnabled(!busy);
    if (busy) status_->setText("…");
}

void LoginDialog::setError(const QString& message) {
    status_->setText(message);
    user_->setEnabled(true);
    pass_->setEnabled(true);
}

} // namespace cppplace::client
