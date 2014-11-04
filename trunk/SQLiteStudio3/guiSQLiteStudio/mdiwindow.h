#ifndef MDIWINDOW_H
#define MDIWINDOW_H

#include "guiSQLiteStudio_global.h"
#include <QMdiSubWindow>
#include <QPointer>

class MdiChild;
class MdiArea;
class Db;

class GUI_API_EXPORT MdiWindow : public QMdiSubWindow
{
    Q_OBJECT

    public:
        MdiWindow(MdiChild* mdiChild, MdiArea *mdiArea, Qt::WindowFlags flags = 0);
        virtual ~MdiWindow();

        virtual QVariant saveSession();
        virtual bool restoreSession(const QVariant& sessionValue);

        MdiChild* getMdiChild() const;
        void setWidget(MdiChild* value);
        bool restoreSessionNextTime();
        void rename(const QString& title);

        void changeEvent(QEvent *event);
        void closeEvent(QCloseEvent* e);

    private:
        bool confirmClose();

        QPointer<QWidget> lastFocusedWidget;
        MdiArea* mdiArea;
        bool dbBeingClosed = false;

    private slots:
        void dbAboutToBeDisconnected(Db* db, bool& deny);
        void dbDisconnected(Db* db);
};

#endif // MDIWINDOW_H
