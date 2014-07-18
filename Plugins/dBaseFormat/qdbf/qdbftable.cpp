#include "qdbffield.h"

#include "qdbfrecord.h"
#include "qdbftable.h"

#include <QtCore/QDate>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QTextCodec>
#include <QtCore/QVarLengthArray>

#include <QMetaEnum>

namespace QDbf {
namespace Internal {

const qint16 DBC_LENGTH = 263;
const qint16 FIELD_DESCRIPTOR_LENGTH = 32;
const qint16 FIELD_NAME_LENGTH = 11;
const qint16 FIELD_LENGTH_OFFSET = 16;
const qint16 FIELD_PRECISION_OFFSET = 17;
const qint16 HEADER_LENGTH_OFFSET_1 = 8;
const qint16 HEADER_LENGTH_OFFSET_2 = 9;
const qint16 LANGUAGE_DRIVER_OFFSET = 29;
const qint16 RECORD_LENGTH_OFFSET_1 = 10;
const qint16 RECORD_LENGTH_OFFSET_2 = 11;
const qint16 RECORDS_COUNT_OFFSET_1 = 4;
const qint16 RECORDS_COUNT_OFFSET_2 = 5;
const qint16 RECORDS_COUNT_OFFSET_3 = 6;
const qint16 RECORDS_COUNT_OFFSET_4 = 7;
const qint16 TABLE_DESCRIPTOR_LENGTH = 32;
const qint16 TERMINATOR_LENGTH = 1;
const qint16 VERSION_NUMBER_OFFSET = 0;

class QDbfTablePrivate
{
public:
    QDbfTablePrivate();
    QDbfTablePrivate(const QString &dbfFileName);
    QDbfTablePrivate(QIODevice* ioDevice);
    QDbfTablePrivate(const QDbfTablePrivate &other);
    ~QDbfTablePrivate();

    enum QDbfTableType
    {
        SimpleTable,
        TableWithDbc
    };

    enum Location
    {
        BeforeFirstRow = -1,
        FirstRow = 0
    };

    bool open(const QString &fileName, QDbfTable::OpenMode openMode = QDbfTable::ReadOnly);
    bool open(QDbfTable::OpenMode openMode = QDbfTable::ReadOnly);
    void close();

    bool setCodepage(QDbfTable::Codepage m_codepage);
    QDbfTable::Codepage codepage() const;

    bool isOpen() const;
    bool isOpenByMe() const;
    int size() const;
    int at() const;
    bool previous() const;
    bool next() const;
    bool first() const;
    bool last() const;
    bool seek(int index) const;

    QDbfRecord record() const;
    QVariant value(int index) const;
    bool addRecord();
    bool addRecord(const QDbfRecord &record);
    bool updateRecordInTable(const QDbfRecord &record);
    bool removeRecord(int index);

    void setTextCodec();
    QByteArray recordData(const QDbfRecord &record, bool addEndOfFileMark = false) const;

    QAtomicInt ref;
    QString m_fileName;
    mutable QDbfTable::DbfTableError m_error;
    mutable QIODevice* m_file = nullptr;
    QDbfTable::OpenMode m_openMode;
    QTextCodec *m_textCodec;
    QDbfTableType m_type;
    QDbfTable::Codepage m_codepage;
    qint16 m_headerLength;
    qint16 m_recordLength;
    qint16 m_fieldsCount;
    int m_recordsCount;
    mutable int m_currentIndex;
    mutable bool m_bufered;
    mutable bool m_openByMe = false;
    mutable QDbfRecord m_currentRecord;
    QDbfRecord m_record;
};

} // namespace Internal
} // namespace QDbf

using namespace QDbf;
using namespace QDbf::Internal;

QDbfTablePrivate::QDbfTablePrivate() :
    ref(1),
    m_error(QDbfTable::NoError),
    m_openMode(QDbfTable::ReadOnly),
    m_textCodec(QTextCodec::codecForLocale()),
    m_type(QDbfTablePrivate::SimpleTable),
    m_codepage(QDbfTable::CodepageNotSet),
    m_headerLength(-1),
    m_recordLength(-1),
    m_fieldsCount(-1),
    m_recordsCount(-1),
    m_currentIndex(-1),
    m_bufered(false)
{
}

QDbfTablePrivate::QDbfTablePrivate(const QString &dbfFileName) :
    ref(1),
    m_fileName(dbfFileName),
    m_error(QDbfTable::NoError),
    m_openMode(QDbfTable::ReadOnly),
    m_textCodec(QTextCodec::codecForLocale()),
    m_type(QDbfTablePrivate::SimpleTable),
    m_codepage(QDbfTable::CodepageNotSet),
    m_headerLength(-1),
    m_recordLength(-1),
    m_fieldsCount(-1),
    m_recordsCount(-1),
    m_currentIndex(-1),
    m_bufered(false)
{
}

QDbfTablePrivate::QDbfTablePrivate(QIODevice* ioDevice) :
    ref(1),
    m_error(QDbfTable::NoError),
    m_file(ioDevice),
    m_openMode(QDbfTable::ReadOnly),
    m_textCodec(QTextCodec::codecForLocale()),
    m_type(QDbfTablePrivate::SimpleTable),
    m_codepage(QDbfTable::CodepageNotSet),
    m_headerLength(-1),
    m_recordLength(-1),
    m_fieldsCount(-1),
    m_recordsCount(-1),
    m_currentIndex(-1),
    m_bufered(false)
{

}

QDbfTablePrivate::QDbfTablePrivate(const QDbfTablePrivate &other) :
    ref(1),
    m_fileName(other.m_fileName),
    m_error(other.m_error),
    m_file(other.m_file),
    m_openMode(other.m_openMode),
    m_textCodec(other.m_textCodec),
    m_type(other.m_type),
    m_codepage(other.m_codepage),
    m_headerLength(other.m_headerLength),
    m_recordLength(other.m_recordLength),
    m_fieldsCount(other.m_fieldsCount),
    m_recordsCount(other.m_recordsCount),
    m_currentIndex(other.m_currentIndex),
    m_bufered(other.m_bufered),
    m_currentRecord(other.m_currentRecord),
    m_record(other.m_record)
{
}

QDbfTablePrivate::~QDbfTablePrivate()
{
    if (isOpen()) {
        m_file->close();
    }
}

bool QDbfTablePrivate::open(const QString &fileName, QDbfTable::OpenMode openMode)
{
    m_fileName = fileName;
    return open(openMode);
}

bool QDbfTablePrivate::open(QDbfTable::OpenMode openMode)
{
    m_openMode = openMode;
    m_error = QDbfTable::NoError;
    m_headerLength = -1;
    m_recordLength = -1;
    m_fieldsCount = -1;
    m_recordsCount = -1;
    m_currentIndex = -1;
    m_bufered = false;
    m_record = QDbfRecord();
    m_currentRecord = QDbfRecord();

    if (isOpen()) {
        m_file->seek(0);
        m_openByMe = false;
    } else {
        if (!m_file) {
            m_file = new QFile(m_fileName);
        }

        if (!m_file->open(openMode == QDbfTable::ReadWrite ? QIODevice::ReadWrite : QIODevice::ReadOnly)) {
            m_error = QDbfTable::OpenError;
            return false;
        }
        m_openByMe = true;
    }

    QByteArray headerData = m_file->read(TABLE_DESCRIPTOR_LENGTH);
    if (headerData.length() != TABLE_DESCRIPTOR_LENGTH) {
        return false;
    }

    const quint8 versionNumber = static_cast<quint8>(headerData.at(VERSION_NUMBER_OFFSET));

    // QDbfTableType
    switch(versionNumber) {
    case 2:
    case 3:
    case 4:
    case 5:
    case 7:
        m_type = QDbfTablePrivate::SimpleTable;
        break;
    case 48:
    case 49:
        m_type = QDbfTablePrivate::TableWithDbc;
        break;
    default:
        return false;
    }

    m_recordsCount = static_cast<int>(headerData.at(RECORDS_COUNT_OFFSET_1) & 0xFF);
    m_recordsCount += static_cast<int>(((headerData.at(RECORDS_COUNT_OFFSET_2) & 0xFF) << 8));
    m_recordsCount += static_cast<int>(((headerData.at(RECORDS_COUNT_OFFSET_3) & 0xFF) << 16));
    m_recordsCount += static_cast<int>(((headerData.at(RECORDS_COUNT_OFFSET_4) & 0xFF) << 24));

    m_headerLength = static_cast<qint16>(headerData.at(HEADER_LENGTH_OFFSET_1) & 0xFF);
    m_headerLength += static_cast<qint16>(((headerData.at(HEADER_LENGTH_OFFSET_2) & 0xFF) << 8));

    m_recordLength = static_cast<qint16>(headerData.at(RECORD_LENGTH_OFFSET_1) & 0xFF);
    m_recordLength += static_cast<qint16>(((headerData.at(RECORD_LENGTH_OFFSET_2) & 0xFF) << 8));

    int fieldDescriptorsLength = m_headerLength - TABLE_DESCRIPTOR_LENGTH - TERMINATOR_LENGTH;

    if (m_type == QDbfTablePrivate::TableWithDbc) {
        fieldDescriptorsLength -= DBC_LENGTH;
    }

    m_fieldsCount  = fieldDescriptorsLength / FIELD_DESCRIPTOR_LENGTH;

    // Codepage
    const quint8 codepage = static_cast<quint8>(headerData.at(LANGUAGE_DRIVER_OFFSET));
    m_codepage = QDbfTable::getCodepage(codepage);

    // set text codec
    setTextCodec();

    QVarLengthArray<char> fieldDescriptorsData(fieldDescriptorsLength);

    if (m_file->read(fieldDescriptorsData.data(),
                    static_cast<qint64>(fieldDescriptorsLength)) != fieldDescriptorsLength) {
        return false;
    }

    int offset = 1;
    for (int i = 0; i < fieldDescriptorsLength; i += FIELD_DESCRIPTOR_LENGTH) {
        QString fieldName;
        for (int j = 0; j < FIELD_NAME_LENGTH; j++) {
            const char fieldNameChar = fieldDescriptorsData.at(i + j) & 0xFF;
            if (fieldNameChar == 0) {
                continue;
            }
            fieldName.append(m_textCodec->toUnicode(&fieldNameChar, 1));
        }

        QVariant::Type fieldType = QVariant::Invalid;
        QDbfField::QDbfType fieldQDbfType = QDbfField::UnknownDataType;
        quint8 fieldTypeChar = static_cast<quint8>(fieldDescriptorsData.at(i + FIELD_NAME_LENGTH) & 0xFF);
        switch (fieldTypeChar) {
        case 67: // C
            fieldType = QVariant::String;
            fieldQDbfType = QDbfField::Character;
            break;
        case 68: // D
            fieldType = QVariant::Date;
            fieldQDbfType = QDbfField::Date;
            break;
        case 70: // F
            fieldType = QVariant::Double;
            fieldQDbfType = QDbfField::FloatingPoint;
            break;
        case 76: // L
            fieldType = QVariant::Bool;
            fieldQDbfType = QDbfField::Logical;
            break;
        /*case 77: // M
            fieldType = QVariant::String;
            break;*/
        case 78: // N
            fieldType = QVariant::Double;
            fieldQDbfType = QDbfField::Number;
            break;
        }

        const int fieldLength = static_cast<int>(fieldDescriptorsData.at(i + FIELD_LENGTH_OFFSET) & 0xFF);
        const int fieldPrecision = static_cast<int>(fieldDescriptorsData.at(i + FIELD_PRECISION_OFFSET) & 0xFF);

        QDbfField field(fieldName, fieldType);
        field.setQDbfType(fieldQDbfType);
        field.setLength(fieldLength);
        field.setPrecision(fieldPrecision);
        field.setOffset(offset);
        m_record.append(field);

        offset += fieldLength;
    }

    return true;
}

void QDbfTablePrivate::close()
{
    if (isOpenByMe()) {
        m_file->close();
    }
}

bool QDbfTablePrivate::setCodepage(QDbfTable::Codepage codepage)
{
    if (!isOpen()) {
        qWarning("QDbfTablePrivate::setCodepage(): IODevice is not open");
        return false;
    }

    if (!m_file || !m_file->isWritable()) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    m_file->seek(LANGUAGE_DRIVER_OFFSET);
    quint8 byte;
    switch(codepage) {
        case QDbfTable::UnspecifiedCodepage:
            return false;
        default:
            byte = static_cast<int>(codepage);
    }

    if (m_file->write(reinterpret_cast<char *>(&byte), 1) != 1) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    m_codepage = codepage;
    setTextCodec();

    m_error = QDbfTable::NoError;

    return true;
}

QDbfTable::Codepage QDbfTablePrivate::codepage() const
{
    return m_codepage;
}

bool QDbfTablePrivate::isOpen() const
{
    return m_file && m_file->isOpen();
}

bool QDbfTablePrivate::isOpenByMe() const
{
    return isOpen() && m_openByMe;
}

int QDbfTablePrivate::size() const
{
    return m_recordsCount;
}

int QDbfTablePrivate::at() const
{
    return m_currentIndex;
}

bool QDbfTablePrivate::previous() const
{
    if (at() <= QDbfTablePrivate::FirstRow) {
        return false;
    }

    if (at() > (size() - 1)) {
        return last();
    }

    return seek(at() - 1);
}

bool QDbfTablePrivate::next() const
{
    if (at() < QDbfTablePrivate::FirstRow) {
        return first();
    }

    if (at() >= (size() - 1)) {
        return false;
    }

    return seek(at() + 1);
}

bool QDbfTablePrivate::first() const
{
    return seek(QDbfTablePrivate::FirstRow);
}

bool QDbfTablePrivate::last() const
{
    return seek(size() - 1);
}

bool QDbfTablePrivate::seek(int index) const
{
    const int previousIndex = m_currentIndex;

    if (index < QDbfTablePrivate::FirstRow) {
        m_currentIndex = QDbfTablePrivate::BeforeFirstRow;
    } else if (index > (size() - 1)) {
        m_currentIndex = size() - 1;
    } else {
        m_currentIndex = index;
    }

    if (previousIndex != m_currentIndex) {
        m_bufered = false;
    }

    return true;
}

QDbfRecord QDbfTablePrivate::record() const
{
    if (m_bufered) {
        return m_currentRecord;
    }

    m_currentRecord = m_record;
    m_bufered = true;

    if (m_currentIndex < QDbfTablePrivate::FirstRow) {
        return m_currentRecord;
    }

    if (!isOpen()) {
        qWarning("QDbfTablePrivate::record(): IODevice is not open");
        return m_currentRecord;
    }

    if (!m_file || !m_file->isReadable()) {
        m_error = QDbfTable::ReadError;
        return m_currentRecord;
    }

    const qint64 position = m_headerLength + m_recordLength * m_currentIndex;

    if (!m_file->seek(position)) {
        m_error = QDbfTable::ReadError;
        return m_currentRecord;
    }

    m_currentRecord.setRecordIndex(m_currentIndex);

    const QByteArray recordData = m_file->read(m_recordLength);

    if (recordData.count() == 0) {
        m_error = QDbfTable::UnspecifiedError;
        return m_currentRecord;
    }

    m_currentRecord.setDeleted(recordData.at(0) == '*' ? true : false);

    for (int i = 0; i < m_currentRecord.count(); ++i) {
        const QByteArray byteArray = recordData.mid(m_currentRecord.field(i).offset(),
                                                    m_currentRecord.field(i).length());
        QVariant value;
        switch (m_currentRecord.field(i).type()) {
        case QVariant::String:
            value = m_textCodec->toUnicode(byteArray);
            break;
        case QVariant::Date:
            value = QVariant(QDate(byteArray.mid(0, 4).toInt(),
                                   byteArray.mid(4, 2).toInt(),
                                   byteArray.mid(6, 2).toInt()));
            break;
        case QVariant::Double:
            value = byteArray.toDouble();
            break;
        case QVariant::Bool: {
            QString val = QString::fromLatin1(byteArray.toUpper());
            if (val == QLatin1String("T") ||
                val == QLatin1String("Y")) {
                value = true;
            } else {
                value = false;
            }
            break; }
        default:
            value = QVariant::Invalid;
        }

        m_currentRecord.setValue(i, value);
    }

    m_error = QDbfTable::NoError;

    return m_currentRecord;
}

QVariant QDbfTablePrivate::value(int index) const
{
    return record().value(index);
}

bool QDbfTablePrivate::addRecord()
{
    QDbfRecord newRecord(record());
    newRecord.clearValues();
    newRecord.setDeleted(false);
    return addRecord(newRecord);
}

bool QDbfTablePrivate::addRecord(const QDbfRecord &record)
{
    if (!isOpen()) {
        qWarning("QDbfTablePrivate::addRecord(): IODevice is not open");
        return false;
    }

    if (!m_file || !m_file->isWritable()) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    QByteArray data = recordData(record, true);

    const qint64 position = m_headerLength + m_recordLength * m_recordsCount;

    if (!m_file->seek(position)) {
        m_error = QDbfTable::ReadError;
        return false;
    }

    if (m_file->write(data) != static_cast<qint64>(m_recordLength) + 1) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    int recordsCount = m_recordsCount + 1;

    unsigned char recordsCountChars[4];
    int shift = 0;
    for (int i = 0; i < 4; ++i) {
        recordsCountChars[i] = recordsCount >> shift;
        shift += 8;
    }

    if (!m_file->seek(4)) {
        m_error = QDbfTable::ReadError;
        return false;
    }

    if (m_file->write((const char *) recordsCountChars, 4) != 4) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    m_recordsCount++;

    m_error = QDbfTable::NoError;

    return true;
}

bool QDbfTablePrivate::updateRecordInTable(const QDbfRecord &record)
{
    if (!isOpen()) {
        qWarning("QDbfTablePrivate::addRecord(): IODevice is not open");
        return false;
    }

    if (!m_file || !m_file->isWritable()) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    QByteArray data = recordData(record);

    const qint64 position = m_headerLength + m_recordLength * record.recordIndex();

    if (!m_file->seek(position)) {
        m_error = QDbfTable::ReadError;
        return false;
    }

    if (m_file->write(data) != static_cast<qint64>(m_recordLength)) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    m_error = QDbfTable::NoError;

    return true;
}

bool QDbfTablePrivate::removeRecord(int index)
{
    if (!isOpen()) {
        qWarning("QDbfTablePrivate::removeRecord(): IODevice is not open");
        return false;
    }

    if (!m_file || !m_file->isWritable()) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    if (index < QDbfTablePrivate::FirstRow ||
        index > (size() - 1)) {
        m_error = QDbfTable::UnspecifiedError;
        return false;
    }

    const qint64 position = m_headerLength + m_recordLength * index;

    if (!m_file->seek(position)) {
        m_error = QDbfTable::ReadError;
        return false;
    }

    const QByteArray recordData = m_file->read(m_recordLength);

    if (recordData.count() != m_recordLength) {
        m_error = QDbfTable::UnspecifiedError;
        return false;
    }

    quint8 byte = '*';

    if (m_file->write(reinterpret_cast<char *>(&byte), 1) != 1) {
        m_error = QDbfTable::WriteError;
        return false;
    }

    m_error = QDbfTable::NoError;

    return true;
}

void QDbfTablePrivate::setTextCodec()
{
    switch (m_codepage) {
        case QDbfTable::cp437:
            m_textCodec = QTextCodec::codecForName("cp437");
            break;
        case QDbfTable::cp850:
            m_textCodec = QTextCodec::codecForName("cp850");
            break;
        case QDbfTable::cp1252:
            m_textCodec = QTextCodec::codecForName("cp1252");
            break;
        case QDbfTable::macRoman:
            m_textCodec = QTextCodec::codecForName("macroman");
            break;
        case QDbfTable::cp852:
            m_textCodec = QTextCodec::codecForName("cp852");
            break;
        case QDbfTable::cp865:
            m_textCodec = QTextCodec::codecForName("cp865");
            break;
        case QDbfTable::cp866:
            m_textCodec = QTextCodec::codecForName("cp866");
            break;
        case QDbfTable::cp861:
            m_textCodec = QTextCodec::codecForName("cp861");
            break;
        case QDbfTable::cp857:
            m_textCodec = QTextCodec::codecForName("cp857");
            break;
        case QDbfTable::cp950:
            m_textCodec = QTextCodec::codecForName("windows-950");
            break;
        case QDbfTable::cp936:
            m_textCodec = QTextCodec::codecForName("windows-936");
            break;
        case QDbfTable::cp1255:
            m_textCodec = QTextCodec::codecForName("cp1255");
            break;
        case QDbfTable::cp1256:
            m_textCodec = QTextCodec::codecForName("cp1256");
            break;
        case QDbfTable::cp932:
            m_textCodec = QTextCodec::codecForName("cp932");
            break;
        case QDbfTable::macCyrillic:
            m_textCodec = QTextCodec::codecForName("mac-cyrillic");
            break;
        case QDbfTable::macGreek:
            m_textCodec = QTextCodec::codecForName("macgr");
            break;
        case QDbfTable::cp1250:
            m_textCodec = QTextCodec::codecForName("cp1250");
            break;
        case QDbfTable::cp1251:
            m_textCodec = QTextCodec::codecForName("Windows-1251");
            break;
        case QDbfTable::cp1254:
            m_textCodec = QTextCodec::codecForName("cp1254");
            break;
        case QDbfTable::cp1253:
            m_textCodec = QTextCodec::codecForName("cp1253");
            break;
        default:
            m_textCodec = QTextCodec::codecForLocale();
    }

    if (!m_textCodec) // for safety
        m_textCodec = QTextCodec::codecForLocale();
}

QByteArray QDbfTablePrivate::recordData(const QDbfRecord &record, bool addEndOfFileMark) const
{
    QByteArray data;
    // delete flag
    data.append(record.isDeleted() ? '*' : ' ');
    // field
    for (int i = 0; i < m_record.count(); ++i) {
        if (m_record.field(i).d != record.field(i).d) {
            m_error = QDbfTable::UnspecifiedError;
            return data;
        }
        switch (record.field(i).dbfType()) {
        case QDbfField::Character:
            data.append(m_textCodec->fromUnicode(record.field(i).value().toString().leftJustified(record.field(i).length(), QLatin1Char(' '), true)));
            break;
        case QDbfField::Date:
            data.append(record.field(i).value().toDate().toString(QString(QLatin1String("yyyyMMdd"))).leftJustified(record.field(i).length(), QLatin1Char(' '), true).toLatin1());
            break;
        case QDbfField::FloatingPoint:
        case QDbfField::Number:
            data.append(QString(QLatin1String("%1")).arg(record.field(i).value().toDouble(), 0, 'f', record.field(i).precision()).rightJustified(record.field(i).length(), QLatin1Char(' '), true).toLatin1());
            break;
        case QDbfField::Logical:
            data.append(record.field(i).value().toBool() ? 'T' : 'F');
            break;
        default:
            data.append(QString(QLatin1String("")).leftJustified(record.field(i).length(), QLatin1Char(' '), true).toLatin1());
        }
    }

    if (addEndOfFileMark) {
        data.append(QChar(26).toLatin1());
    }

    return data;
}

QDbfTable::QDbfTable() :
    d(new QDbfTablePrivate())
{
}

QDbfTable::QDbfTable(const QString &dbfFileName) :
    d(new QDbfTablePrivate(dbfFileName))
{
}

QDbfTable::QDbfTable(QIODevice* ioDevice) :
    d(new QDbfTablePrivate(ioDevice))
{
}

QDbfTable &QDbfTable::operator=(const QDbfTable &other)
{
    if (this == &other) {
        return *this;
    }
    qAtomicAssign(d, other.d);
    return *this;
}

bool QDbfTable::operator==(const QDbfTable &other) const
{
    return (d->m_file == other.d->m_file &&
            d->m_type == other.d->m_type &&
            d->m_codepage == other.d->m_codepage &&
            d->m_headerLength == other.d->m_headerLength &&
            d->m_recordLength == other.d->m_recordLength &&
            d->m_fieldsCount == other.d->m_fieldsCount &&
            d->m_recordsCount == other.d->m_recordsCount);
}

QDbfTable::~QDbfTable()
{
    if (!d->ref.deref()) {
        delete d;
    }
}

QString QDbfTable::fileName() const
{
    if (dynamic_cast<QFile*>(d->m_file))
        return dynamic_cast<QFile*>(d->m_file)->fileName();
    else
        return QString();
}

QDbfTable::OpenMode QDbfTable::openMode() const
{
    return d->m_openMode;
}

QDbfTable::DbfTableError QDbfTable::error() const
{
    return d->m_error;
}

bool QDbfTable::open(const QString &fileName, OpenMode openMode)
{
    return d->open(fileName, openMode);
}

void QDbfTable::close()
{
    d->close();
}

bool QDbfTable::open(OpenMode openMode)
{
    return d->open(openMode);
}

bool QDbfTable::setCodepage(QDbfTable::Codepage codepage)
{
    return d->setCodepage(codepage);
}

QDbfTable::Codepage QDbfTable::codepage() const
{
    return d->codepage();
}

bool QDbfTable::isOpen() const
{
    return d->isOpen();
}

int QDbfTable::size() const
{
    return d->size();
}

int QDbfTable::at() const
{
    return d->at();
}

bool QDbfTable::previous() const
{
    return d->previous();
}

bool QDbfTable::next() const
{
    return d->next();
}

bool QDbfTable::first() const
{
    return d->first();
}

bool QDbfTable::last() const
{
    return d->last();
}

bool QDbfTable::seek(int index) const
{
    return d->seek(index);
}

QDbfRecord QDbfTable::record() const
{
    return d->record();
}

QVariant QDbfTable::value(int index) const
{
    return d->value(index);
}

bool QDbfTable::addRecord()
{
    return d->addRecord();
}

bool QDbfTable::addRecord(const QDbfRecord &record)
{
    return d->addRecord(record);
}

bool QDbfTable::updateRecordInTable(const QDbfRecord &record)
{
    return d->updateRecordInTable(record);
}

bool QDbfTable::removeRecord(int index)
{
    return d->removeRecord(index);
}

QDbfTable::Codepage QDbfTable::getCodepage(qint8 value)
{
    QMetaEnum cpEnum = QDbfTable::staticMetaObject.enumerator(0);
    const char* enumName = cpEnum.valueToKey(value);
    if (!enumName)
        return UnspecifiedCodepage;

    return static_cast<Codepage>(value);
}

QDebug operator<<(QDebug debug, const QDbfTable &table)
{
    debug.nospace() << "QDbfTable("
                    << table.fileName() << ", "
                    << "size: " << table.record().count()
                    << " x " << table.size() << ')';

    return debug.space();
}
