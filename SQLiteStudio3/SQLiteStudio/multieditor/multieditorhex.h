#ifndef MULTIEDITORHEX_H
#define MULTIEDITORHEX_H

#include "multieditorwidget.h"
#include <QVariant>
#include <QSharedPointer>

class QHexEdit;
class QBuffer;

class MultiEditorHex : public MultiEditorWidget
{
        Q_OBJECT
    public:
        explicit MultiEditorHex();
        ~MultiEditorHex();

        void setValue(const QVariant& value);
        QVariant getValue();
        void setReadOnly(bool value);

        QList<QWidget*> getNoScrollWidgets();

    private:
        QHexEdit* hexEdit;

    private slots:
        void modificationChanged();
};

#endif // MULTIEDITORHEX_H
