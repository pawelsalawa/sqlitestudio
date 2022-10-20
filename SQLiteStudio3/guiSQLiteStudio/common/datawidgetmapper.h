#ifndef DATAWIDGETMAPPER_H
#define DATAWIDGETMAPPER_H

#include <QObject>
#include <QHash>
#include <functional>

class QAbstractItemModel;

class DataWidgetMapper : public QObject
{
        Q_OBJECT
    public:
        typedef std::function<bool(QWidget*)> SubmitFilter;

        explicit DataWidgetMapper(QObject *parent = 0);

        QAbstractItemModel* getModel() const;
        void setModel(QAbstractItemModel* value);
        void addMapping(QWidget* widget, int modelColumn, const QString& propertyName);
        void clearMapping();
        int getCurrentIndex() const;
        int mappedSection(QWidget* widget) const;
        SubmitFilter getSubmitFilter() const;
        void setSubmitFilter(const SubmitFilter& value);

    private:
        struct MappingEntry
        {
            QWidget* widget = nullptr;
            int columnIndex = 0;
            QString propertyName;
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
