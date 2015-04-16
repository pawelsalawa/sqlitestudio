#ifndef IPVALIDATOR_H
#define IPVALIDATOR_H

#include "guiSQLiteStudio_global.h"
#include <QValidator>

class GUI_API_EXPORT IpValidator : public QValidator
{
    public:
        IpValidator(QObject* parent = 0);
        ~IpValidator();

        State validate(QString& input, int&) const;

        bool getAcceptWhiteSpaces() const;
        void setAcceptWhiteSpaces(bool value);

        bool getAcceptEmptyParts() const;
        void setAcceptEmptyParts(bool value);

        QChar getWhitespaceCharacter() const;
        void setWhitespaceCharacter(const QChar& value);

        static bool check(const QString& input, bool acceptWhiteSpaces = false);

    private:
        static QString getPattern(bool acceptWhiteSpaces, bool requireFull, QChar whitespaceCharacter);

        bool acceptWhiteSpaces = false;
        bool acceptEmptyParts = false;
        QChar whitespaceCharacter = ' ';

        static QString reStr;
};

#endif // IPVALIDATOR_H
