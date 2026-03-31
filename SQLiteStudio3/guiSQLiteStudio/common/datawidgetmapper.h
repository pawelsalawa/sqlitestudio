#ifndef DATAWIDGETMAPPER_H
#define DATAWIDGETMAPPER_H

#include "guiSQLiteStudio_global.h"
#include <QObject>
#include <QHash>
#include <functional>

class QAbstractItemModel;

/**
 * @brief Binds widget with Qt's item model.
 *
 * DataWidgetMapper is a class that allows to easily map widgets to data in a model.
 * It is used in FormView to map widgets in the form to data in the model, so when current index is changed,
 * widgets are updated with new values from model and when submit() is called, values from widgets are written back to model.
 *
 * Mapping is done by column index in model, so each widget is mapped to one column. When current index is changed,
 * widget's property with name 'propertyName' is set to value from the model for the mapped column and current index row.
 * When submit() is called, value from widget's property with name 'propertyName' is read and written to model for the mapped column and current index row.
 *
 * You can also specify extra properties that should be set when current index is changed. Each extra property is a pair of widget property name and data role in model.
 */
class GUI_API_EXPORT DataWidgetMapper : public QObject
{
        Q_OBJECT
    public:
        typedef std::function<bool(QWidget*)> SubmitFilter;
        typedef QPair<QString,int> PropertyAndRole;

        explicit DataWidgetMapper(QObject *parent = 0);

        QAbstractItemModel* getModel() const;
        void setModel(QAbstractItemModel* value);

        /**
         * @brief Maps a widget to a column in model. When current index is changed, widget's property with name 'propertyName'
         *        will be set to value from the model for the mapped column and current index row.
         * @param widget Widget to be mapped. It will be used for setting value from model and also for getting value when submit() is called.
         * @param modelColumn Column index in model to which the widget should be mapped.
         * @param propertyName Name of the property of the widget which should be set to value from model when current index is changed
         *        and from which value should be read when submit() is called.
         * @param extraProperties List of extra properties that should be set when current index is changed. Each entry in the list is a pair of widget property name and data role.
         */
        void addMapping(QWidget* widget, int modelColumn, const QString& propertyName, const QList<PropertyAndRole>& extraProperties = {});

        /**
         * @brief Adds extra mapping for the widget. This is used when you want to map multiple properties of the same widget to model data with different roles.
         * @param widget Widget to which the extra mapping should be added. It must be already mapped to some column with addMapping() method, otherwise this method does nothing and prints critical debug log.
         * @param propertyName Name of the property of the widget which should be set to value from model when current index is changed and from which value should be read when submit() is called.
         * @param role Data role in model from which the value should be read when current index is changed and to which the value should be written when submit() is called.
         */
        void addMapping(QWidget* widget, const QString& propertyName, int role);

        /**
         * @brief Clears all mappings. After calling this method, no widget is mapped to model and current index is reset to -1. When current index is changed to -1, widgets are not updated with values from model.
         */
        void clearMapping();

        /**
         * @brief Returns current index in model which is mapped to widgets. When current index is changed, widgets are updated with values from model for the new current index.
         * @return Current index in model which is mapped to widgets.
         */
        int getCurrentIndex() const;

        /**
         * @brief Returns column index in model to which the widget is mapped, or -1 if the widget is not mapped to any column.
         * @param widget Widget for which the mapped column index should be returned.
         * @return Column index in model to which the widget is mapped, or -1 if the widget is not mapped to any column.
         */
        int mappedSection(QWidget* widget) const;

        /**
         * @brief Returns submit filter which is used to determine whether the value from the widget should be submitted to model when submit() is called. If submit filter is not set, all widgets are submitted.
         * @return Submit filter which is used to determine whether the value from the widget should be submitted to model when submit() is called.
         */
        SubmitFilter getSubmitFilter() const;
        void setSubmitFilter(const SubmitFilter& value);

    private:
        struct MappingEntry
        {
            QWidget* widget = nullptr;
            int columnIndex = 0;
            QList<PropertyAndRole> properties; // pair of widget property name and data role in model
        };

        void loadFromModel();

        QAbstractItemModel* model = nullptr;
        int currentIndex = -1;
        QHash<QWidget*,MappingEntry*> mappings;
        SubmitFilter submitFilter = nullptr;

    public slots:
        void setCurrentIndex(int rowIndex);
        void toFirst();
        void toLast();
        void toNext();
        void toPrevious();
        void submit();
        void revert();

    signals:
        void currentIndexChanged(int newRowIndex);
};

#endif // DATAWIDGETMAPPER_H
