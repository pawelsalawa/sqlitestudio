#ifndef DEBUGCONSOLE_H
#define DEBUGCONSOLE_H

#include "guiSQLiteStudio_global.h"
#include <QDialog>
#include <QTextCharFormat>

namespace Ui {
class DebugConsole;
}

class GUI_API_EXPORT DebugConsole : public QDialog
{
    Q_OBJECT

    public:
        explicit DebugConsole(QWidget *parent = 0);
        ~DebugConsole();

    protected:
        void showEvent(QShowEvent*);

    private:
        void initFormats();
        void message(const QString& msg, const QTextCharFormat& format);

        Ui::DebugConsole *ui = nullptr;
        QTextCharFormat dbgFormat;
        QTextCharFormat wrnFormat;
        QTextCharFormat criFormat;
        QTextCharFormat fatFormat;
        QTextBlockFormat blockFormat;

    private slots:
        void reset();

    public slots:
        void debug(const QString& msg);
        void warning(const QString& msg);
        void critical(const QString& msg);
        void fatal(const QString& msg);
};

#endif // DEBUGCONSOLE_H
