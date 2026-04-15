#ifndef DIALOGSIZEHANDLER_H
#define DIALOGSIZEHANDLER_H

#include "guiSQLiteStudio_global.h"
#include <QObject>
#include <QRect>

class QTimer;

class GUI_API_EXPORT DialogSizeHandler : public QObject
{
    Q_OBJECT

    public:
        enum Mode
        {
            BOTH,
            HORIZONTAL,
            VERTICAL
        };

        virtual ~DialogSizeHandler();

        static void applyFor(QObject *parent, Mode mode = BOTH);
        static void applyFor(const QString& key, QObject *parent, Mode mode = BOTH);

    protected:
        DialogSizeHandler(const QString& key, QObject *parent, Mode mode);

        bool eventFilter(QObject *obj, QEvent *event) override;

    private:
        static const constexpr char* CONFIG_GROUP = "DialogDimensions";

        QString configKey;
        QTimer* saveTimer = nullptr;
        QRect recentGeometry;
        Mode mode = BOTH;

    public slots:
        void doSave();
};

#endif // DIALOGSIZEHANDLER_H
