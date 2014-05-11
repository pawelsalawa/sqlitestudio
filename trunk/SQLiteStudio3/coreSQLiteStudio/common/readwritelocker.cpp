#include "readwritelocker.h"
#include "parser/lexer.h"
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>

ReadWriteLocker::ReadWriteLocker(QReadWriteLock* lock, Mode mode)
{
    init(lock, mode);
}

ReadWriteLocker::ReadWriteLocker(QReadWriteLock* lock, const QString& query, Dialect dialect, bool noLock)
{
    init(lock, getMode(query, dialect, noLock));
}

ReadWriteLocker::~ReadWriteLocker()
{
    if (readLocker)
    {
        delete readLocker;
        readLocker = nullptr;
    }

    if (writeLocker)
    {
        delete writeLocker;
        writeLocker = nullptr;
    }
}

void ReadWriteLocker::init(QReadWriteLock* lock, ReadWriteLocker::Mode mode)
{
    switch (mode)
    {
        case ReadWriteLocker::READ:
            readLocker = new QReadLocker(lock);
            break;
        case ReadWriteLocker::WRITE:
            writeLocker = new QWriteLocker(lock);
            break;
        case ReadWriteLocker::NONE:
            // Nothing to lock.
            break;
    }
}

ReadWriteLocker::Mode ReadWriteLocker::getMode(const QString &query, Dialect dialect, bool noLock)
{
    static QStringList readOnlyCommands = {"ANALYZE", "EXPLAIN", "PRAGMA"};

    if (noLock)
        return ReadWriteLocker::NONE;

    TokenList tokens = Lexer::tokenize(query, dialect);
    int keywordIdx = tokens.indexOf(Token::KEYWORD);

    if (keywordIdx > -1 && readOnlyCommands.contains(tokens[keywordIdx]->value.toUpper()))
        return ReadWriteLocker::READ;

    if (keywordIdx > -1 && tokens[keywordIdx]->value.toUpper() == "WITH")
    {
        bool matched = false;
        bool isSelect = false;
        int depth = 0;
        for (TokenPtr token : tokens)
        {
            switch (token->type)
            {
                case Token::PAR_LEFT:
                    depth++;
                    break;
                case Token::PAR_RIGHT:
                    depth--;
                    break;
                case Token::KEYWORD:
                    if (depth == 0)
                    {
                        QString val = token->value.toUpper();
                        if (val == "SELECT")
                        {
                            matched = true;
                            isSelect = true;
                        }
                        else if (val == "DELETE" || val == "UPDATE" || val == "INSERT")
                        {
                            matched = true;
                        }
                    }
                    break;
                default:
                    break;
            }

            if (matched)
                break;
        }
        if (isSelect)
            return ReadWriteLocker::READ;
    }

    return ReadWriteLocker::WRITE;
}
