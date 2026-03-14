#ifndef MDICHILD_H
#define MDICHILD_H

#include "common/extactioncontainer.h"
#include "committable.h"
#include <QWidget>
#include <QVariant>

class MdiWindow;
class Icon;
class Db;

class GUI_API_EXPORT MdiChild : public QWidget, public ExtActionContainer, public Committable
{
        Q_OBJECT
    public:
        explicit MdiChild(QWidget* parent = 0);
        ~MdiChild();

        QVariant getSessionValue();
        bool applySessionValue(const QVariant& sessionValue);

        virtual bool shouldReuseForArgs(int argCount, ...);

        MdiWindow* getMdiWindow() const;
        virtual void setMdiWindow(MdiWindow* value);
        bool isInvalid() const;
        void updateWindowTitle();
        virtual bool restoreSessionNextTime();
        virtual bool handleInitialFocus();

        /**
         * @brief Provides associated database for this window. It is used to determine which windows should be closed when database is closed.
         * @return Database used by this MDI child.
         *
         * It should return the database pointer here only if the window strongly depends on the database.
         * For example EditorWindow will not return a pointer here, because it should not be closed with database,
         * as the editor can just switch to another database.
         * But TableWindow will return a pointer, because it doesn't make sense to keep it open without database.
         */
        virtual Db* getAssociatedDb() const;

        /**
         * @brief Provides associated database for this window, together with the name of the database object that is currently used in the window (if applicable).
         * @return Pair of database pointer and database object (table/view) name. Database object name can be empty if the window doesn't work with specific database object, but just with the database in general.
         *
         * This method provides a soft association, meaning that the window will not be closed, even if it uses pointed database & object.
         * It is used to allow MdiArea->DbTree selection linking.
         */
        virtual QPair<Db*, QString> getSoftDbObjectAssociation() const;

        virtual void dbClosedFinalCleanup();
        virtual bool isWindowClosingBlocked() const;

    protected:

        virtual QVariant saveSession() = 0;
        virtual bool restoreSession(const QVariant& sessionValue) = 0;

        virtual Icon* getIconNameForMdiWindow() = 0;
        virtual QString getTitleForMdiWindow() = 0;

        bool invalid = false;

    private:
        MdiWindow* mdiWindow = nullptr;

    signals:
        void sessionValueChanged();
};


#endif // MDICHILD_H
