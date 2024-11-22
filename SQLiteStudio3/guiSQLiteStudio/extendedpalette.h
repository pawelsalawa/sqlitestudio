#ifndef EXTENDEDPALETTE_H
#define EXTENDEDPALETTE_H

#include <QBrush>
#include <QPalette>
#include <QVariant>

class QStyle;

class ExtendedPalette
{
    public:
        ExtendedPalette();

        QBrush editorString() const;
        void setEditorString(const QBrush &value);

        QBrush editorLineBase() const;
        void setEditorLineBase(const QBrush &value);

        bool styleChanged(QStyle* style, const QString& themeName);

        QBrush mdiAreaBase() const;
        void setMdiAreaBase(const QBrush& value);

        QBrush editorExpression() const;
        void setEditorExpression(const QBrush& value);

        const QBrush& editorCurrentQueryBase() const;
        void setEditorCurrentQueryBase(const QBrush& newEditorCurrentQueryBrush);

        const QBrush& editorLineNumberBase() const;
        void setEditorLineNumberBaseBrush(const QBrush& newEditorLineNumberBaseBrush);

    private:
        QBrush editorStringBrush;
        QBrush editorExpressionBrush;
        QBrush editorLineBaseBrush;
        QBrush editorLineNumberBaseBrush;
        QBrush editorCurrentQueryBrush;
        QBrush mdiAreaBaseBrush;

        QVariant initializedForPalette;
};

#endif // EXTENDEDPALETTE_H
