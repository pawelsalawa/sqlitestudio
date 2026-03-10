#ifndef EXTENDEDPALETTE_H
#define EXTENDEDPALETTE_H

#include "guiSQLiteStudio_global.h"
#include <QBrush>
#include <QPalette>
#include <QVariant>

class QStyle;

class GUI_API_EXPORT ExtendedPalette
{
    public:
        ExtendedPalette();

        bool styleChanged(QStyle* style, const QString& themeName);

        QBrush editorString() const;
        QBrush editorLineBase() const;
        QBrush mdiAreaBase() const;
        QBrush editorExpression() const;
        const QBrush& editorCurrentQueryBase() const;
        const QBrush& editorLineNumberBase() const;
        QBrush editorComment() const;

    private:
        QBrush editorStringBrush;
        QBrush editorExpressionBrush;
        QBrush editorLineBaseBrush;
        QBrush editorLineNumberBaseBrush;
        QBrush editorCurrentQueryBrush;
        QBrush editorCommentBrush;
        QBrush mdiAreaBaseBrush;

        QVariant initializedForPalette;
};

#endif // EXTENDEDPALETTE_H
