#ifndef STYLE_H
#define STYLE_H

#include "extendedpalette.h"

#include <QProxyStyle>


class Style : public QProxyStyle
{
    public:
        static Style* getInstance();

        const ExtendedPalette &extendedPalette() const;
        void setStyle(QStyle* style, const QString& styleName);

    private:
        static Style* instance;

        Style(QStyle* style);

        ExtendedPalette extPalette;
};

#define STYLE Style::getInstance()

#endif // STYLE_H
