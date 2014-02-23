#ifndef COLUMNCOLLATEPANEL_H
#define COLUMNCOLLATEPANEL_H

#include "constraintpanel.h"

namespace Ui {
    class ColumnCollatePanel;
}

class QStringListModel;

class ColumnCollatePanel : public ConstraintPanel
{
        Q_OBJECT

    public:
        explicit ColumnCollatePanel(QWidget *parent = 0);
        ~ColumnCollatePanel();

        bool validate();

    protected:
        void changeEvent(QEvent *e);
        void constraintAvailable();
        void storeConfiguration();

    private:
        void init();
        void readConstraint();
        void readCollations();

        QStringListModel* collationModel;
        Ui::ColumnCollatePanel *ui;

    private slots:
        void updateState();
};

#endif // COLUMNCOLLATEPANEL_H
