#ifndef EXTTABLEWIDGET_H
#define EXTTABLEWIDGET_H

#include "guiSQLiteStudio_global.h"
#include <QTableWidget>

/**
 * @brief Extended QTableWidget with some additional features.
 *
 * Currently it only emits doubleClicked(QModelIndex) signal when user double clicks on the item,
 * but in the future it may be extended with some more features, so it's better to have it as separate class.
 */
class GUI_API_EXPORT ExtTableWidget : public QTableWidget
{
    public:
        explicit ExtTableWidget(QWidget* parent = nullptr);

    protected:
        void mouseDoubleClickEvent(QMouseEvent* e);
};

#endif // EXTTABLEWIDGET_H
