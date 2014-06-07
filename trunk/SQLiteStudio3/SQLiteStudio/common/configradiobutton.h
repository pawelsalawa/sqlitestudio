#ifndef CONFIGRADIOBUTTON_H
#define CONFIGRADIOBUTTON_H

#include <QRadioButton>
#include <QVariant>

/**
 * @brief Config-oriented radio button.
 *
 * It's just like a usual QRadioButton, except it has a value assigned to it
 * and when the radio is toggled on, the signal is emitted to inform about it.
 * To inform about the button being toggled off a different signal is emitted.
 * It also has a slot to be called when the associated property in the application
 * has changed and needs to be reflected in the button - the button checks
 * if the value of the property reflects the button's assigned value
 * and toggles on or off approprietly. In that case the signals are not emitted.
 */
class ConfigRadioButton : public QRadioButton
{
        Q_OBJECT

        Q_PROPERTY(QVariant assignedValue READ getAssignedValue WRITE setAssignedValue)

    public:
        explicit ConfigRadioButton(QWidget *parent = 0);

        QVariant getAssignedValue() const;
        void setAssignedValue(const QVariant& value);

    private:
        QVariant assignedValue;
        bool handlingSlot = false;

    signals:
        void toggledOn(const QVariant& assignedBalue);
        void toggledOff(const QVariant& assignedBalue);

    private slots:
        void handleToggled(bool checked);

    public slots:
        void alignToValue(const QVariant& value);
};

#endif // CONFIGRADIOBUTTON_H
