#include "passwordtogglehelper.h"
#include "iconmanager.h"

PasswordToggleHelper::PasswordToggleHelper(QLineEdit *lineEdit)
    : QObject(lineEdit)
    , lineEdit(lineEdit)
{
    lineEdit->setEchoMode(QLineEdit::Password);

    toggleAction = lineEdit->addAction(ICONS.EYE_CLOSED, QLineEdit::TrailingPosition);
    toggleAction->setToolTip(tr("Show / hide password"));

    connect(toggleAction, &QAction::triggered, this, &PasswordToggleHelper::toggleEchoMode);
}

void PasswordToggleHelper::toggleEchoMode()
{
    visible = !visible;
    lineEdit->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
    updateIcon();
}

void PasswordToggleHelper::updateIcon()
{
    toggleAction->setIcon(visible ? ICONS.EYE_OPEN : ICONS.EYE_CLOSED);
}
