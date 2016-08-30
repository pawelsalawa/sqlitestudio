#ifndef SEARCHTEXTLOCATOR_H
#define SEARCHTEXTLOCATOR_H

#include "guiSQLiteStudio_global.h"
#include <QObject>
#include <QTextDocument>

class GUI_API_EXPORT SearchTextLocator : public QObject
{
        Q_OBJECT

    public:
        struct GUI_API_EXPORT Occurrance
        {
            int start;
            int end;
        };

        SearchTextLocator(QTextDocument* document, QObject* parent = nullptr);

        QString getLookupString() const;
        void setLookupString(const QString& value);

        QString getReplaceString() const;
        void setReplaceString(const QString& value);

        bool getCaseSensitive() const;
        void setCaseSensitive(bool value);

        bool getRegularExpression() const;
        void setRegularExpression(bool value);

        bool getSearchBackwards() const;
        void setSearchBackwards(bool value);

        int getStartPosition() const;
        void setStartPosition(int value);

        void reset();

    private:
        QTextDocument::FindFlags getFlags();
        void notFound();
        QTextCursor findInWholeDoc(QTextDocument::FindFlags flags);
        void replaceCurrent();

        QTextDocument* document = nullptr;
        int initialStartPosition;
        int lastMatchStart = -1;
        int lastMatchEnd = -1;
        bool afterDocPositionSwitch = false;
        bool ignoreCursorMovements = false;

        // Config parameters
        QString lookupString;
        QString replaceString;
        bool caseSensitive = false;
        bool regularExpression = false;
        bool searchBackwards = false;
        int startPosition = 0;

    public slots:
        bool find(QTextDocument::FindFlags flags = 0);
        void findNext();
        void findPrev();
        bool replaceAndFind();
        void replaceAll();
        void cursorMoved();

    signals:
        void found(int start, int end);
        void reachedEnd();
        void replaceAvailable(bool available);
};

#endif // SEARCHTEXTLOCATOR_H
