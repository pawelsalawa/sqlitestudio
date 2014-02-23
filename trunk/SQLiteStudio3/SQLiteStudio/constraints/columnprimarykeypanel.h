#ifndef COLUMNPRIMARYKEYPANEL_H
#define COLUMNPRIMARYKEYPANEL_H

#include "constraintpanel.h"

namespace Ui {
    class ColumnPrimaryKeyPanel;
}

class ColumnPrimaryKeyPanel : public ConstraintPanel
{
        Q_OBJECT

    public:
        explicit ColumnPrimaryKeyPanel(QWidget *parent = 0);
        ~ColumnPrimaryKeyPanel();

        bool validate();

    protected:
        void changeEvent(QEvent *e);
        void constraintAvailable();
        void storeConfiguration();

    private:
        void init();
        void readConstraint();

        Ui::ColumnPrimaryKeyPanel *ui;

    private slots:
        void updateState();
};

#endif // COLUMNPRIMARYKEYPANEL_H
