#ifndef CLICOMMANDTREE_H
#define CLICOMMANDTREE_H

#include "schemaresolver.h"
#include "cliutils.h"
#include "clicommand.h"

class CliCommandTree : public CliCommand
{
        Q_OBJECT

    public:
        void execute();
        QString shortHelp() const;
        QString fullHelp() const;
        void defineSyntax();

    private:
        enum Opts
        {
            COLUMNS,
            SYSTEM_OBJECTS
        };

        AsciiTree getDatabaseTree(const QString& database, SchemaResolver& resolver, bool printColumns);
        AsciiTree getTableTree(const QString& database, const QString& table, SchemaResolver& resolver, bool printColumns);
        AsciiTree getViewTree(const QString& database, const QString& view, SchemaResolver& resolver);
        AsciiTree getTreeLeaf(const QString& column);

        static const QString metaNodeNameTemplate;
};

#endif // CLICOMMANDTREE_H
