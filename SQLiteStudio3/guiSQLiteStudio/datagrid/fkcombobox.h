#ifndef FKCOMBOBOX_H
#define FKCOMBOBOX_H

#include <QComboBox>

class SqlQueryModelColumn;
class SqlQueryModel;
class SqlQueryView;
class Db;

class FkComboBox : public QComboBox
{
    Q_OBJECT

    public:
        FkComboBox(QWidget *parent, int dropDownViewMinWidth = -1);

        static QString getSqlForFkEditor(Db* db, SqlQueryModelColumn* columnModel, const QVariant& currentValue);
        static qlonglong getRowCountForFkEditor(Db* db, const QString& query, bool* isError);

        static const qlonglong MAX_ROWS_FOR_FK = 10000L;
        static const int FK_CELL_LENGTH_LIMIT = 30;

        void init(Db* db, SqlQueryModelColumn* columnModel);
        void setValue(const QVariant& value);
        QVariant getValue(bool* manualValueUsed = nullptr, bool* ok = nullptr) const;

    private:
        class FkComboShowFilter : public QObject
        {
            public:
                explicit FkComboShowFilter(FkComboBox* parentCombo);
                bool eventFilter(QObject *obj, QEvent *event);
        };

        void init();
        void updateComboViewGeometry(bool initial) const;
        void updateCurrentItemIndex(const QString& value = QString());
        int getFkViewHeaderWidth(bool includeScrollBar) const;
        QString getSql() const;

        int dropDownViewMinWidth;
        SqlQueryView* comboView = nullptr;
        SqlQueryModel* comboModel = nullptr;
        SqlQueryModelColumn* columnModel = nullptr;
        QString beforeLoadValue;
        QVariant sourceValue;
        bool disableValueChangeNotifications = false;
        QString oldValue;

    private slots:
        void fkDataAboutToLoad();
        void fkDataReady();
        void fkDataFailed(const QString& errorText);
        void notifyValueModified();

    signals:
        void valueModified();
};

#endif // FKCOMBOBOX_H
