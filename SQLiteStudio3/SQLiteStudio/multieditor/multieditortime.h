#ifndef MULTIEDITORTIME_H
#define MULTIEDITORTIME_H

#include "multieditordatetime.h"

class MultiEditorTime : public MultiEditorDateTime
{
    public:
        explicit MultiEditorTime(QWidget *parent = 0);

        static void staticInit();

    protected:
        QStringList getParsingFormats();

    private:
        static QStringList formats;
};

#endif // MULTIEDITORTIME_H
