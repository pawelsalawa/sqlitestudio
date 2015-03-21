#ifndef IPVALIDATOR_H
#define IPVALIDATOR_H

#include <QValidator>

class IpValidator : public QValidator
{
    public:
        IpValidator(QObject* parent = 0);
        ~IpValidator();

        State validate(QString&input, int&) const;

        bool getAcceptWhiteSpaces() const;
        void setAcceptWhiteSpaces(bool value);

        bool getAcceptEmptyParts() const;
        void setAcceptEmptyParts(bool value);

        QChar getWhitespaceCharacter() const;
        void setWhitespaceCharacter(const QChar& value);

    private:
        bool acceptWhiteSpaces = false;
        bool acceptEmptyParts = false;
        QChar whitespaceCharacter = ' ';
};

#endif // IPVALIDATOR_H
