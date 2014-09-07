#ifndef QNUMERICSPINBOX_H
#define QNUMERICSPINBOX_H

#include "guiSQLiteStudio_global.h"
#include <QAbstractSpinBox>

/**
 * @brief The NumericSpinBox class
 * This class implements a spinbox for numeric SQLite types.
 * This includes integers, as well as decimals.
 * User is also allowed to type in any text value (unless "strict" property is set),
 * but once he uses "step up" or "step down", the text value
 * gets replaced with 0.
 * If strict propery is set, also the allowEmpty property starts to matter.
 * Otherwise allowEmpty is ignored.
 */
class GUI_API_EXPORT NumericSpinBox : public QAbstractSpinBox
{
        Q_OBJECT
    public:
        explicit NumericSpinBox(QWidget *parent = 0);

        void stepBy(int steps);
        QValidator::State validate(QString& input, int& pos) const;

        QVariant getValue() const;
        void setValue(const QVariant& newValue, bool nullAsZero = true);

        bool isStrict() const;
        void setStrict(bool value, bool allowEmpty = true);

        bool getAllowEmpty() const;
        void setAllowEmpty(bool value);

    protected:
        StepEnabled	stepEnabled() const;

    private:
        QVariant getFixedVariant(const QVariant& value);
        void setValueInternal(const QVariant& newValue);
        void stepIntBy(int steps);
        void stepDoubleBy(int steps);
        void updateText();
        QValidator::State validateStrict(QString &input, int &pos) const;

        QVariant value;
        bool strict = false;
        bool allowEmpty = true;

    private slots:
        void valueEdited(const QString& value);

    signals:
        void modified() const;
};

#endif // QNUMERICSPINBOX_H
