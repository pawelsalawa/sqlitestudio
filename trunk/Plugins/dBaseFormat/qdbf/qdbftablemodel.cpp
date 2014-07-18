#include "qdbftablemodel.h"

#include "qdbffield.h"
#include "qdbfrecord.h"

#include <QtCore/QDebug>

#define DBF_PREFETCH 255

namespace QDbf {
namespace Internal {

class QDbfTableModelPrivate
{
public:
    QDbfTableModelPrivate();
    QDbfTableModelPrivate(const QString &filePath);
    ~QDbfTableModelPrivate();

    bool open(const QString &filePath, bool readOnly = false);
    bool open(bool readOnly = false);

    int rowCount(const QModelIndex &index = QModelIndex()) const;
    int columnCount(const QModelIndex &index = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant data(const QModelIndex &index, int role) const;

    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::DisplayRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool canFetchMore(const QModelIndex &index = QModelIndex()) const;
    void fetchMore(const QModelIndex &index = QModelIndex());

    QDbfTableModel *q;
    QString m_filePath;
    bool m_readOnly;
    QDbfTable *const m_dbfTable;
    QDbfRecord m_record;
    QVector<QDbfRecord> m_records;
    QVector<QHash<int, QVariant> > m_headers;
    int m_deletedRecordsCount;
    int m_lastRecordIndex;
};

} // namespace Internal
} // namespace QDbf

using namespace QDbf;
using namespace QDbf::Internal;

QDbfTableModelPrivate::QDbfTableModelPrivate() :
    q(0),
    m_filePath(QString::null),
    m_readOnly(false),
    m_dbfTable(new QDbfTable()),
    m_deletedRecordsCount(0),
    m_lastRecordIndex(-1)
{
}

QDbfTableModelPrivate::QDbfTableModelPrivate(const QString &filePath) :
    q(0),
    m_filePath(filePath),
    m_readOnly(false),
    m_dbfTable(new QDbfTable()),
    m_deletedRecordsCount(0),
    m_lastRecordIndex(-1)
{
}

QDbfTableModelPrivate::~QDbfTableModelPrivate()
{
    m_dbfTable->close();
    delete m_dbfTable;
}

bool QDbfTableModelPrivate::open(const QString &filePath, bool readOnly)
{
    m_filePath = filePath;
    return open(readOnly);
}

bool QDbfTableModelPrivate::open(bool readOnly)
{
    m_readOnly = readOnly;
    m_record = QDbfRecord();
    m_records.clear();
    m_headers.clear();
    m_deletedRecordsCount = 0;
    m_lastRecordIndex = -1;

    const QDbfTable::OpenMode &openMode = m_readOnly
            ? QDbfTable::ReadOnly
            : QDbfTable::ReadWrite;

    if (!m_dbfTable->open(m_filePath, openMode)) {
        return false;
    }

    m_record = m_dbfTable->record();

    if (canFetchMore()) {
        fetchMore();
    }

    return true;
}

int QDbfTableModelPrivate::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return m_records.count();
}

int QDbfTableModelPrivate::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return m_record.count();
}

Qt::ItemFlags QDbfTableModelPrivate::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (!index.isValid()) {
        return flags;
    }

    QVariant value = m_records.at(index.row()).value(index.column());

    if (value.type() == QVariant::Bool) {
        flags |= Qt::ItemIsTristate;
    }

    if (!m_readOnly) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool QDbfTableModelPrivate::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value);

    if (!m_dbfTable->isOpen()) {
        return false;
    }

    if (index.isValid() && role == Qt::EditRole) {
        QVariant oldValue = m_records.at(index.row()).value(index.column());
        m_records[index.row()].setValue(index.column(), value);

        if (!m_dbfTable->updateRecordInTable(m_records.at(index.row()))) {
            m_records[index.row()].setValue(index.column(), oldValue);
            return false;
        }

        emit q->dataChanged(index, index);

        return true;
    }

    return false;
}

QVariant QDbfTableModelPrivate::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= rowCount() ||
        index.column() >= columnCount()) {
        return QVariant();
    }

    QVariant value = m_records.at(index.row()).value(index.column());

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (value.type()) {
        case QVariant::String:
            return value.toString().trimmed();
        default:
            return value;
        }
     case Qt::CheckStateRole:
        switch (value.type()) {
        case QVariant::Bool:
            return value.toBool() ? Qt::Checked : Qt::Unchecked;
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
}

bool QDbfTableModelPrivate::setHeaderData(int section, Qt::Orientation orientation,
                                          const QVariant &value, int role)
{
    if (orientation != Qt::Horizontal || section < 0 || columnCount() <= section) {
        return false;
    }

    if (m_headers.size() <= section) {
        m_headers.resize(qMax(section + 1, 16));
    }

    m_headers[section][role] = value;

    emit q->headerDataChanged(orientation, section, section);

    return true;
}

QVariant QDbfTableModelPrivate::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        QVariant value = m_headers.value(section).value(role);

        if (role == Qt::DisplayRole && !value.isValid()) {
            value = m_headers.value(section).value(Qt::EditRole);
        }

        if (value.isValid()) {
            return value;
        }

        if (role == Qt::DisplayRole && m_record.count() > section) {
            return m_record.fieldName(section);
        }
    }

    if (role == Qt::DisplayRole) {
        return section + 1;
    }

    return QVariant();
}

bool QDbfTableModelPrivate::canFetchMore(const QModelIndex &index) const
{
    if (!index.isValid() && m_dbfTable->isOpen() &&
        (m_records.size() + m_deletedRecordsCount < m_dbfTable->size())) {
        return true;
    }

    return false;
}

void QDbfTableModelPrivate::fetchMore(const QModelIndex &index)
{
    if (index.isValid()) {
        return;
    }

    if (!m_dbfTable->seek(m_lastRecordIndex)) {
        return;
    }

    const int fetchSize = qMin(m_dbfTable->size() - m_records.count() -
                               m_deletedRecordsCount, DBF_PREFETCH);

    q->beginInsertRows(index, m_records.size() + 1, m_records.size() + fetchSize);

    int fetchedRecordsCount = 0;
    while (m_dbfTable->next()) {
        const QDbfRecord record(m_dbfTable->record());
        if (record.isDeleted()) {
            ++m_deletedRecordsCount;
            continue;
        }
        m_records.append(record);
        m_lastRecordIndex = m_dbfTable->at();
        if (++fetchedRecordsCount >= fetchSize) {
            break;
        }
    }

    q->endInsertRows();
}

QDbfTableModel::QDbfTableModel(QObject *parent) :
    QAbstractTableModel(parent),
    d(new QDbfTableModelPrivate())
{
    d->q = this;
}

QDbfTableModel::QDbfTableModel(const QString &filePath, QObject *parent) :
    QAbstractTableModel(parent),
    d(new QDbfTableModelPrivate(filePath))
{
    d->q = this;
}

QDbfTableModel::~QDbfTableModel()
{
    delete d;
}

bool QDbfTableModel::open(const QString &filePath, bool readOnly)
{
    return d->open(filePath, readOnly);
}

bool QDbfTableModel::open(bool readOnly)
{
    return d->open(readOnly);
}

bool QDbfTableModel::readOnly() const
{
    return d->m_readOnly;
}

QDbfTable::DbfTableError QDbfTableModel::error() const
{
    return d->m_dbfTable->error();
}

int QDbfTableModel::rowCount(const QModelIndex &index) const
{
    return d->rowCount(index);
}

int QDbfTableModel::columnCount(const QModelIndex &index) const
{
    return d->columnCount(index);
}

QVariant QDbfTableModel::data(const QModelIndex &index, int role) const
{
    return d->data(index, role);
}

bool QDbfTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    return d->setHeaderData(section, orientation, value, role);
}

QVariant QDbfTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return d->headerData(section, orientation, role);
}

Qt::ItemFlags QDbfTableModel::flags(const QModelIndex &index) const
{
    return d->flags(index);
}

bool QDbfTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return d->setData(index, value, role);
}

bool QDbfTableModel::canFetchMore(const QModelIndex &index) const
{
    return d->canFetchMore(index);
}

void QDbfTableModel::fetchMore(const QModelIndex &index)
{
    d->fetchMore(index);
}
