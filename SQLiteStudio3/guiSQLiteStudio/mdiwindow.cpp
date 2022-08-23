#include "mdiwindow.h"
#include "mdichild.h"
#include "mdiarea.h"
#include "iconmanager.h"
#include "mainwindow.h"
#include "services/dbmanager.h"
#include "db/db.h"
#include "uiconfig.h"
#include <QDateTime>
#include <QApplication>
#include <QDebug>
#include <QFocusEvent>
#include <QAction>
#include <QMessageBox>
#include <QAbstractButton>

MdiWindow::MdiWindow(MdiChild* mdiChild, MdiArea *mdiArea, Qt::WindowFlags flags) :
    QMdiSubWindow(mdiArea->viewport(), flags), mdiArea(mdiArea)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWidget(mdiChild);

    connect(DBLIST, SIGNAL(dbAboutToBeDisconnected(Db*,bool&)), this, SLOT(dbAboutToBeDisconnected(Db*,bool&)));
    connect(DBLIST, SIGNAL(dbDisconnected(Db*)), this, SLOT(dbDisconnected(Db*)));
}

MdiWindow::~MdiWindow()
{
    if (SQLITESTUDIO->getImmediateQuit())
        return;

    if (!closeWithoutSessionSaving && !MAINWINDOW->isClosingApp())
        MAINWINDOW->pushClosedWindowSessionValue(saveSession());

    mdiArea->windowDestroyed(this);
}

QVariant MdiWindow::saveSession()
{
    if (!widget())
        return QVariant();

    QHash<QString, QVariant> hash = getMdiChild()->getSessionValue().toHash();
    hash["title"] = windowTitle();
    hash["position"] = pos();
    hash["geometry"] = saveGeometry();
    return hash;
}

bool MdiWindow::restoreSession(const QVariant& sessionValue)
{
    if (!widget())
        return true;

    QHash<QString, QVariant> value = sessionValue.toHash();
    if (value.size() == 0)
        return true;

    if (value.contains("geometry"))
        restoreGeometry(value["geometry"].toByteArray());

    if (value.contains("position"))
        move(value["position"].toPoint());

    if (value.contains("title"))
    {
        QString title = value["title"].toString();
        rename(title);
    }

    return getMdiChild()->applySessionValue(sessionValue);
}

MdiChild* MdiWindow::getMdiChild() const
{
    if (!widget())
        return nullptr;

    return dynamic_cast<MdiChild*>(widget());
}

void MdiWindow::setWidget(MdiChild* value)
{
    QMdiSubWindow::setWidget(value);
    if (value)
        value->setMdiWindow(this);
}

bool MdiWindow::restoreSessionNextTime()
{
    return getMdiChild()->restoreSessionNextTime();
}

void MdiWindow::rename(const QString& title)
{
    setWindowTitle(title);

    QAction* task = mdiArea->getTaskByWindow(this);
    if (task)
        task->setText(title);
}

void MdiWindow::changeEvent(QEvent* event)
{
    if (event->type() != QEvent::WindowStateChange)
    {
        QMdiSubWindow::changeEvent(event);
        return;
    }

    QWindowStateChangeEvent *changeEvent = static_cast<QWindowStateChangeEvent *>(event);

    bool wasActive = changeEvent->oldState().testFlag(Qt::WindowActive);
    bool isActive = windowState().testFlag(Qt::WindowActive);

    /*
     * This is a hack for a bug in Qt: QTBUG-23515. The problem is that when QMdiSubWindow
     * changes its state (because we used shortcut, or the TaskBar to activate another window,
     * or window is maximized or minimized), then the code responsible for hiding old window
     * gives focus to its previous or next child widget (depending on what is the state change),
     * before the new window is shown. This seems to be happening in QWidgetPrivate::hide_helper().
     *
     */
    if (wasActive && isActive)
    {
        // Handle problem with maximize/minimize
        QWidget* w = focusWidget();
        QMdiSubWindow::changeEvent(event);
        if (w)
            w->setFocus();
    }
    else if (wasActive && !isActive)
    {
        // Handle problem with switching between 2 MDI windows - part 1
        lastFocusedWidget = focusWidget();
        QMdiSubWindow::changeEvent(event);
    }
    else if (!wasActive && isActive)
    {
        // Handle problem with switching between 2 MDI windows - part 2
        QMdiSubWindow::changeEvent(event);
        if (!lastFocusedWidget.isNull() && (!focusWidget() || !isAncestorOf(focusWidget())))
            lastFocusedWidget->setFocus();
    }
    else
        QMdiSubWindow::changeEvent(event);

    if (!MAINWINDOW->isClosingApp())
    {
        bool wasMaximized = changeEvent->oldState().testFlag(Qt::WindowMaximized);
        bool isMaximized = windowState().testFlag(Qt::WindowMaximized);
        if (wasMaximized != isMaximized && CFG_UI.General.OpenMaximized.get() != isMaximized)
            CFG_UI.General.OpenMaximized.set(isMaximized);
    }
}

void MdiWindow::closeEvent(QCloseEvent* e)
{
    if (dbBeingClosed || MAINWINDOW->isClosingApp() || !getMdiChild()->isUncommitted())
    {
        QMdiSubWindow::closeEvent(e);
        return;
    }

    if (confirmClose())
        QMdiSubWindow::closeEvent(e);
    else
        e->ignore();
}

void MdiWindow::dbAboutToBeDisconnected(Db* db, bool& deny)
{
    if (!isAssociatedWithDb(db))
        return;

    if (MAINWINDOW->isClosingApp())
        return;

    if (getMdiChild()->isUncommitted() && !confirmClose())
        deny = true;
    else
        dbBeingClosed = true;
}

void MdiWindow::dbDisconnected(Db* db)
{
    if (!isAssociatedWithDb(db))
        return;

    if (MAINWINDOW->isClosingApp())
        return;

    closeWindow();
}

bool MdiWindow::confirmClose()
{
    QMessageBox msgBox(QMessageBox::Question, tr("Uncommitted changes"), getMdiChild()->getQuitUncommittedConfirmMessage(), QMessageBox::Yes|QMessageBox::No, this);
    msgBox.setDefaultButton(QMessageBox::No);

    QAbstractButton* closeBtn = msgBox.button(QMessageBox::Yes);
    QAbstractButton* cancelBtn = msgBox.button(QMessageBox::No);
    closeBtn->setText(tr("Close anyway"));
    closeBtn->setIcon(ICONS.CLOSE);
    cancelBtn->setText(tr("Don't close"));
    cancelBtn->setIcon(ICONS.GO_BACK);

    return (msgBox.exec() == QMessageBox::Yes);
}

void MdiWindow::closeWindow()
{
    getMdiChild()->dbClosedFinalCleanup();
    MDIAREA->enforceCurrentTaskSelectionAfterWindowClose();
    close();
}

bool MdiWindow::isAssociatedWithDb(Db* db)
{
    if (dbBeingClosed)
        return true; // it was already confirmed by dbAboutToBeDisconnected() and any changes to the "db" member afterwards should not inflict decision change here

    return db && getMdiChild()->getAssociatedDb() == db;
}

bool MdiWindow::getCloseWithoutSessionSaving() const
{
    return closeWithoutSessionSaving;
}

void MdiWindow::setCloseWithoutSessionSaving(bool value)
{
    closeWithoutSessionSaving = value;
}

