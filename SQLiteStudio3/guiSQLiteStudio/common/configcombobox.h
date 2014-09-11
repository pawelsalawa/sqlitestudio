#ifndef CONFIGCOMBOBOX_H
#define CONFIGCOMBOBOX_H

#include "guiSQLiteStudio_global.h"
#include <QComboBox>

/**
 * @brief Config-oriented combo box.
 *
 * It's just like a regular QComboBox, except it honors additional Qt dynamic property
 * called "modelName". The "modelName" property should name a CfgEntry key (together with its category,
 * just like "cfg" properties for CfgEntry linked widgets), that is of QStringList type.
 * The QStringList is used as a data model for QComboBox. Every time that the CfgEntry
 * with QStringList changes, the combo box data entries are updated.
 */
class GUI_API_EXPORT ConfigComboBox : public QComboBox
{
        Q_OBJECT

        Q_PROPERTY(QVariant modelName READ getModelName WRITE setModelName)

    public:
        explicit ConfigComboBox(QWidget* parent = 0);

        QVariant getModelName() const;

    public slots:
        void setModelName(QVariant arg);

    private:
        QVariant modelName;
};

#endif // CONFIGCOMBOBOX_H
