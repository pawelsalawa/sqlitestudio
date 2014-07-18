/*******************************************************************
 * Qt dbf library (https://code.google.com/p/qdbf/)
 * by arial79@gmail.com
 *
 * Version updated for Qt5 and removed symbol exports,
 * cause those are not used by the SQLiteStudio plugin.
 *
 * Also some enhancements were introduced:
 * 1. Codepages got enhanced. Now they support way
 * more cases and adding/removing codepage requires
 * only one additional sourcecode change (for QTextCodec name),
 * instead of multiple adjustments.
 * 2. Support for reading/writting from/to QIODevice*,
 * not only a filename.
 *******************************************************************/

#ifndef QDBFTABLE_H
#define QDBFTABLE_H

class QVariant;
#include "qdbf_global.h"
#include <QString>
#include <QDebug>

namespace QDbf {
namespace Internal {

class QDbfTablePrivate;

} // namespace Internal

class QDbfRecord;

class QDBF_EXPORT QDbfTable : public QObject
{
        Q_OBJECT

public:
    enum Codepage {
        CodepageNotSet = 0x0,
        cp437 = 0x01,			// DOS USA
        cp850 = 0x02,			// DOS Multilingual
        cp1252 = 0x03,			// Windows ANSI
        macRoman = 0x04,		// Standard Macintosh
        cp852 = 0x64,			// EE MS-DOS
        cp865 = 0x65,			// Nordic MS-DOS
        cp866 = 0x66,			// Russian MS-DOS
        cp861 = 0x67,			// Icelandic MS-DOS
        cp857 = 0x6B,			// Turkish MS-DOS
        cp950 = 0x78,			// Chinese Windows
        cp936 = 0x7A,			// Chinese Windows
        cp1255 = 0x7D,			// Herbew Windows
        cp1256 = 0x7E,			// Arabic Windows
        cp932 = 0x8B,			// Japanese Windows
        macCyrillic = 0x96,    	// Russian Macintosh
        macGreek = 0x98,	    // Greek Macintosh
        cp1250 = 0xC8,			// Windows EE
        cp1251 = 0xC9,			// Russian Windows
        cp1254 = 0xCA,			// Turkish Windows
        cp1253 = 0xCB,			// Greek Windows
        UnspecifiedCodepage = 0xFF
    };

    Q_ENUMS(Codepage)

    enum OpenMode {
        ReadOnly = 0,
        ReadWrite
    };

    enum DbfTableError {
        NoError = 0,
        OpenError,
        ReadError,
        WriteError,
        PermissionsError,
        UnspecifiedError
    };

    QDbfTable();
    explicit QDbfTable(const QString &dbfFileName);
    explicit QDbfTable(QIODevice* ioDevice);
    bool operator==(const QDbfTable &other) const;
    inline bool operator!=(const QDbfTable &other) const { return !operator==(other); }
    QDbfTable &operator=(const QDbfTable &other);
    virtual ~QDbfTable();

    bool open(const QString &fileName, OpenMode openMode = QDbfTable::ReadOnly);
    bool open(OpenMode openMode = QDbfTable::ReadOnly);

    void close();

    QString fileName() const;

    QDbfTable::OpenMode openMode() const;

    DbfTableError error() const;

    bool setCodepage(QDbfTable::Codepage codepage);
    QDbfTable::Codepage codepage() const;

    bool isOpen() const;
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

    static Codepage getCodepage(qint8 value);

private:
    Internal::QDbfTablePrivate *d;
};

} // namespace QDbf

QDebug operator<<(QDebug, const QDbf::QDbfTable&);

#endif // QDBFTABLE_H
