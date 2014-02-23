#ifndef MULTIEDITORDATE_H
#define MULTIEDITORDATE_H

#include "multieditordatetime.h"

class MultiEditorDate : public MultiEditorDateTime
{
    public:
        explicit MultiEditorDate(QWidget *parent = 0);

        static void staticInit();

    protected:
        QStringList getParsingFormats();

    private:
        static QStringList formats;
};

#endif // MULTIEDITORDATE_H
