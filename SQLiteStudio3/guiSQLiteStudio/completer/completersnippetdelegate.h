#ifndef COMPLETERSNIPPETDELEFATE_H
#define COMPLETERSNIPPETDELEGATE_H

#include "guiSQLiteStudio_global.h"
#include <QStyledItemDelegate>

class GUI_API_EXPORT CompleterSnippetDelegate : public QStyledItemDelegate
{
    public:
        explicit CompleterSnippetDelegate(QObject *parent = 0);

        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // COMPLETERSNIPPETDELEGATE_H
