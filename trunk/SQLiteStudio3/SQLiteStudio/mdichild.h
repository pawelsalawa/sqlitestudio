#ifndef MDICHILD_H
#define MDICHILD_H

#include <QWidget>
#include <QVariant>

class MdiWindow;
class Icon;

class MdiChild : public QWidget
{
        Q_OBJECT
    public:
        explicit MdiChild(QWidget* parent = 0);
        ~MdiChild();

        QVariant getSessionValue();
        bool applySessionValue(const QVariant& sessionValue);

        MdiWindow* getMdiWindow() const;
        void setMdiWindow(MdiWindow* value);

        bool isInvalid() const;
        virtual bool restoreSessionNextTime();
        void updateWindowTitle();

    protected:
        virtual QVariant saveSession() = 0;
        virtual bool restoreSession(const QVariant& sessionValue) = 0;

        virtual Icon* getIconNameForMdiWindow() = 0;
        virtual QString getTitleForMdiWindow() = 0;

        bool invalid = false;

    private:
        MdiWindow* mdiWindow;
};

#endif // MDICHILD_H
