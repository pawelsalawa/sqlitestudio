#ifndef COMPLETERITEMDELEGATE_H
#define COMPLETERITEMDELEGATE_H

#include "guiSQLiteStudio_global.h"
#include "expectedtoken.h"
#include <QStyledItemDelegate>

class QIcon;

class GUI_API_EXPORT CompleterItemDelegate : public QStyledItemDelegate
{
        Q_OBJECT
    public:
        explicit CompleterItemDelegate(QObject *parent = 0);

        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

    private:
        QIcon* getIcon(ExpectedToken::Type type) const;
        void paintIcon(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        void paintText(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        void paintPrefix(QPainter* painter, const QFontMetrics& metrics, int& x, int y, const QString& text) const;
        void paintValue(QPainter* painter, const QFontMetrics& metrics, int& x, int y, const QString& text) const;
        void paintLabel(QPainter* painter, int& x, int y, const QString& text, bool emptyValue) const;

        const static int spacer = 1;
};

#endif // COMPLETERITEMDELEGATE_H
