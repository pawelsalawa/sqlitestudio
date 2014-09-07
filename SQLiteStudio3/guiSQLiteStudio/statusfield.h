#ifndef STATUSFIELD_H
#define STATUSFIELD_H

#include "guiSQLiteStudio_global.h"
#include <QVariant>
#include <QDockWidget>

class QMenu;

namespace Ui {
    class StatusField;
}

class GUI_API_EXPORT StatusField : public QDockWidget
{
        Q_OBJECT

    public:
        explicit StatusField(QWidget *parent = 0);
        ~StatusField();

        bool hasMessages() const;

    protected:
        void changeEvent(QEvent *e);

    private:
        void addEntry(const QIcon& icon, const QString& text, const QColor &color);
        void setupMenu();
        void readRecentMessages();

        Ui::StatusField *ui;
        QMenu* menu;
        QAction* copyAction;
        QAction* clearAction;
        static const int timeStampColumnWidth = 70;
        static constexpr const char* timeStampFormat = "hh:mm:ss";

    private slots:
        void customContextMenuRequested(const QPoint& pos);
        void info(const QString& text);
        void warn(const QString& text);
        void error(const QString& text);
        void reset();
        void fontChanged(const QVariant& variant);

    signals:
        void linkActivated(const QString& link);
};

#endif // STATUSFIELD_H
