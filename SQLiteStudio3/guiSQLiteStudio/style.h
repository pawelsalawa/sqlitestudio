#ifndef STYLE_H
#define STYLE_H

#include "extendedpalette.h"
#include <QPalette>
#include <QProxyStyle>

class CfgEntry;

class Style : public QProxyStyle
{
    Q_OBJECT

    public:
        static Style* getInstance();
        static bool isDark(const QStyle* style);

        const ExtendedPalette &extendedPalette() const;
        void setStyle(QStyle* style, const QString& styleName);
        QString name() const;
        bool isDark() const;

    protected:
        bool eventFilter(QObject *obj, QEvent *ev) override;

    private:
        static Style* instance;

        Style(QStyle* style);

        ExtendedPalette extPalette;
        QPalette initialPalette;

    signals:
        void paletteChanged();
};

#define STYLE Style::getInstance()

#endif // STYLE_H
