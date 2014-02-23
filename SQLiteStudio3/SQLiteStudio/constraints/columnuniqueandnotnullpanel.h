#ifndef COLUMNUNIQUEANDNOTNULLPANEL_H
#define COLUMNUNIQUEANDNOTNULLPANEL_H

#include "constraintpanel.h"

namespace Ui {
    class ColumnUniqueAndNotNullPanel;
}

class ColumnUniqueAndNotNullPanel : public ConstraintPanel
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

        Ui::ColumnUniqueAndNotNullPanel *ui;

    private slots:
        void updateState();
};

#endif // COLUMNUNIQUEANDNOTNULLPANEL_H
