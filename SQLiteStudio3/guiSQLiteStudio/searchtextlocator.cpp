#include "searchtextlocator.h"
#include "common/unused.h"
#include <QTextCursor>
#include <QTextBlock>
#include <QRegularExpression>
#include <QDebug>

SearchTextLocator::SearchTextLocator(QTextDocument* document, QObject* parent) :
    QObject(parent), document(document)
{
}

QString SearchTextLocator::getLookupString() const
{
    return lookupString;
}

void SearchTextLocator::setLookupString(const QString& value)
{
    lookupString = value;
    lastMatchStart = -1;
    lastMatchEnd = -1;
}

QString SearchTextLocator::getReplaceString() const
{
    return replaceString;
}

void SearchTextLocator::setReplaceString(const QString& value)
{
    replaceString = value;
}

bool SearchTextLocator::getCaseSensitive() const
{
    return caseSensitive;
}

void SearchTextLocator::setCaseSensitive(bool value)
{
    caseSensitive = value;
}

bool SearchTextLocator::getRegularExpression() const
{
    return regularExpression;
}

void SearchTextLocator::setRegularExpression(bool value)
{
    regularExpression = value;
}

bool SearchTextLocator::getSearchBackwards() const
{
    return searchBackwards;
}

void SearchTextLocator::setSearchBackwards(bool value)
{
    searchBackwards = value;
}

int SearchTextLocator::getStartPosition() const
{
    return startPosition;
}

void SearchTextLocator::setStartPosition(int value)
{
    if (ignoreCursorMovements)
        return;

    startPosition = value;
    initialStartPosition = value;
    afterDocPositionSwitch = false;
    lastMatchStart = -1;
    lastMatchEnd = -1;
    emit replaceAvailable(false);
}

QTextDocument::FindFlags SearchTextLocator::getFlags()
{
    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;

    if (searchBackwards)
        flags |= QTextDocument::FindBackward;

    return flags;
}

void SearchTextLocator::notFound()
{
    startPosition = initialStartPosition;
    afterDocPositionSwitch = false;
    emit reachedEnd();
    emit replaceAvailable(false);
}

QTextCursor SearchTextLocator::findInWholeDoc(QTextDocument::FindFlags flags)
{
    // Simply find a match
    QTextCursor cursor;
    if (regularExpression)
        cursor = document->find(QRegularExpression(lookupString), startPosition, flags);
    else
        cursor = document->find(lookupString, startPosition, flags);

    // If not matched, see if we can find match at the other part of document (before/after init start position)
    if (cursor.isNull() && !afterDocPositionSwitch)
    {
        afterDocPositionSwitch = true;
        int start = 0;
        if (flags.testFlag(QTextDocument::FindBackward))
            start = document->lastBlock().position() + document->lastBlock().length();

        if (regularExpression)
            cursor = document->find(QRegularExpression(lookupString), start, flags);
        else
            cursor = document->find(lookupString, start, flags);
    }

    // If we found a match after/before start position, but it's already before/after start position, we cannot report it as a match.
    if ((afterDocPositionSwitch && !cursor.isNull()) && // we have a match after switching doc position
        ((!flags.testFlag(QTextDocument::FindBackward) && cursor.selectionStart() >= initialStartPosition) || // it's after init pos
        (flags.testFlag(QTextDocument::FindBackward) && cursor.selectionEnd() <= initialStartPosition))) // it's before init pos
    {
        cursor = QTextCursor(); // exceeded search range
    }

    // If we do have a match, we need to remember its parameters for the next lookup.
    if (!cursor.isNull())
    {
        if (flags.testFlag(QTextDocument::FindBackward))
            startPosition = cursor.selectionStart();
        else
            startPosition = cursor.selectionEnd();

        lastMatchStart = cursor.selectionStart();
        lastMatchEnd = cursor.selectionEnd();
    }

    return cursor;
}

void SearchTextLocator::replaceCurrent()
{
    if (lastMatchStart == -1 || lastMatchEnd == -1)
        return;

    ignoreCursorMovements = true;

    QTextCursor cursor(document);
    cursor.setPosition(lastMatchStart);
    cursor.setPosition(lastMatchEnd, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.insertText(replaceString);

    ignoreCursorMovements = false;

    // Adjust further lookups according to replaced lenght change.
    startPosition += replaceString.length() - lookupString.length();
}

bool SearchTextLocator::find(QTextDocument::FindFlags flags)
{
    if (flags == 0)
        flags = getFlags();

    QTextCursor cursor = findInWholeDoc(flags);
    if (cursor.isNull())
    {
        notFound();
        return false;
    }

    emit found(cursor.selectionStart(), cursor.selectionEnd());
    emit replaceAvailable(true);
    return true;
}

void SearchTextLocator::findNext()
{
    QTextDocument::FindFlags flags = getFlags();
    flags &= !QTextDocument::FindBackward;
    find(flags);
}

void SearchTextLocator::findPrev()
{
    QTextDocument::FindFlags flags = getFlags();
    flags |= QTextDocument::FindBackward;
    find(flags);
}

bool SearchTextLocator::replaceAndFind()
{
    replaceCurrent();
    return find();
}

void SearchTextLocator::replaceAll()
{
    QString origContents = document->toPlainText();
    QString contents = origContents;
    qsizetype replLen = replaceString.length();
    qsizetype diff = 0;
    if (regularExpression)
    {
        QRegularExpression re(lookupString,
                              caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
        contents.replace(re, replaceString);

        qsizetype pos = 0;
        QRegularExpressionMatch match;
        while ((pos = origContents.indexOf(re, pos, &match)) != -1 && pos < startPosition)
        {
            qsizetype len = match.capturedLength();
            pos += len;
            diff += (replLen - len);
        }
    }
    else
    {
        Qt::CaseSensitivity cs = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
        contents.replace(lookupString, replaceString, cs);

        qsizetype len = lookupString.length();
        qsizetype singleDiff = (replLen - len);
        qsizetype pos = 0;
        while ((pos = origContents.indexOf(lookupString, pos, cs)) != -1 && pos < startPosition)
        {
            pos += len;
            diff += singleDiff;
        }
    }
    qsizetype newPos = startPosition + diff; // calculated before replacing contents to use original startPosition

    QTextCursor cursor(document);
    cursor.setPosition(0);
    cursor.setPosition(origContents.length(), QTextCursor::KeepAnchor);
    cursor.insertText(contents);

    emit newCursorPositionAfterAllReplaced(newPos);
}

void SearchTextLocator::cursorMoved()
{
    emit replaceAvailable(false);
}
