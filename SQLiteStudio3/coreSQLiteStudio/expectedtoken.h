#ifndef EXPECTEDTOKEN_H
#define EXPECTEDTOKEN_H

#include "coreSQLiteStudio_global.h"
#include <QString>
#include <QSharedPointer>

struct API_EXPORT ExpectedToken
{
    /**
     * @brief The expected token type
     * Order of this enum matters, because it defines the order
     * of sorting a list of expected tokens by CompletionComparer.
     * The NO_VALUE value means that there is no prepared value,
     * but some context information and/or label is available.
     */
    enum Type
    {
        COLUMN,
        TABLE,
        INDEX,
        TRIGGER,
        VIEW,
        DATABASE,
        NO_VALUE,
        KEYWORD,
        FUNCTION,
        OPERATOR,
        COLLATION,
        PRAGMA,
        STRING,
        NUMBER,
        BLOB,
        OTHER
    };

    /**
     * @brief type Token type.
     */
    Type type;

    /**
     * @brief value Token value.
     * The actual value to be inserted in expected position.
     */
    QString value;

    /**
     * @brief contextInfo Context information.
     * Context of proposed token. Context is for example table, that proposed column belongs to,
     * or it's database, that proposed table belongs to. This is used when completer sorts results.
     */
    QString contextInfo;

    /**
     * @brief label Token explanation label.
     * This is a context info to be displayed in completer. It may provide info about actual object under the alias,
     * or human readable info about the proposed token (like "New column name").
     */
    QString label;

    /**
     * @brief prefix Token prefix.
     * Necessary prefix for token to be valid. It should be concatenated with the value using a dot character.
     */
    QString prefix;

    /**
     * @brief Token priority on completion list.
     *
     * Some tokens require manual control on where they appear in the completion list. If this value is any positive
     * number, than it's considered as a priority. The higher value, the higher priority. 0 and negative numbers are ignored.
     * The priority has a precedence before any other sorting rules.
     */
    int priority = 0;

    /**
     * @brief needsWrapping Tells if the token requires wrapping.
     * @return true if the token is of type that might require wrapping (in case the value contains forbidden characters or keyword as a name).
     */
    bool needsWrapping() const;

    int operator==(const ExpectedToken& other) const;
    QString toString() const;
};

/**
 * @brief The ExpectedTokenPtr class
 * This class would normally be just a typedef, but we need qHash() functionality
 * being applied for this type, so simple typedef wouldn't overwrite the type matching for qHash() arguments.
 * The qHash() is necessary for QSet and we need that for getting rid of duplicates.
 */
class API_EXPORT ExpectedTokenPtr : public QSharedPointer<ExpectedToken>
{
    public:
        ExpectedTokenPtr();
        explicit ExpectedTokenPtr(ExpectedToken* ptr);
        explicit ExpectedTokenPtr(const QSharedPointer<ExpectedToken> & other);
        explicit ExpectedTokenPtr(const QWeakPointer<ExpectedToken> & other);
};

int operator==(const ExpectedTokenPtr& ptr1, const ExpectedTokenPtr& ptr2);

TYPE_OF_QHASH qHash(const ExpectedToken& token);
TYPE_OF_QHASH qHash(const ExpectedTokenPtr &ptr);

#endif // EXPECTEDTOKEN_H
