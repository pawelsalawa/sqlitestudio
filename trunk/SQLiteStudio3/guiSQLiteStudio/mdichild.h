#ifndef MDICHILD_H
#define MDICHILD_H

#include "common/extactioncontainer.h"
#include "committable.h"
#include <QWidget>
#include <QVariant>

class MdiWindow;
class Icon;

class GUI_API_EXPORT MdiChild : public QWidget, public ExtActionContainer, public Committable
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
        void updateWindowTitle();
        virtual bool restoreSessionNextTime();
        virtual bool handleInitialFocus();

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
