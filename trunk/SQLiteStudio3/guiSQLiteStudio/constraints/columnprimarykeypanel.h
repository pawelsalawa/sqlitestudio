#ifndef COLUMNPRIMARYKEYPANEL_H
#define COLUMNPRIMARYKEYPANEL_H

#include "constraintpanel.h"
#include "guiSQLiteStudio_global.h"

namespace Ui {
    class ColumnPrimaryKeyPanel;
}

class GUI_API_EXPORT ColumnPrimaryKeyPanel : public ConstraintPanel
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

        Ui::ColumnPrimaryKeyPanel *ui = nullptr;

    private slots:
        void updateState();
};

#endif // COLUMNPRIMARYKEYPANEL_H
