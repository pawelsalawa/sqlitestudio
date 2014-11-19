#ifndef COLUMNUNIQUEANDNOTNULLPANEL_H
#define COLUMNUNIQUEANDNOTNULLPANEL_H

#include "constraintpanel.h"
#include "guiSQLiteStudio_global.h"

namespace Ui {
    class ColumnUniqueAndNotNullPanel;
}

class GUI_API_EXPORT ColumnUniqueAndNotNullPanel : public ConstraintPanel
{
        Q_OBJECT

    public:
        explicit ColumnUniqueAndNotNullPanel(QWidget *parent = 0);
        ~ColumnUniqueAndNotNullPanel();

        bool validate();

    protected:
        void changeEvent(QEvent *e);
        void constraintAvailable();
        void storeConfiguration();
        virtual void storeType() = 0;

    private:
        void init();
        void readConstraint();

        Ui::ColumnUniqueAndNotNullPanel *ui = nullptr;

    private slots:
        void updateState();
};

#endif // COLUMNUNIQUEANDNOTNULLPANEL_H
