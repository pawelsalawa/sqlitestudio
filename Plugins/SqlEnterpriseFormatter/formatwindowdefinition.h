#ifndef FORMATWINDOWDEFINITION_H
#define FORMATWINDOWDEFINITION_H

#include "formatstatement.h"
#include "parser/ast/sqlitewindowdefinition.h"

class FormatWindowDefinition : public FormatStatement
{
    public:
        FormatWindowDefinition(SqliteWindowDefinition* windowDef);

    protected:
        void formatInternal();

    private:
        SqliteWindowDefinition* windowDef = nullptr;
};

class FormatWindowDefinitionWindow : public FormatStatement
{
    public:
        FormatWindowDefinitionWindow(SqliteWindowDefinition::Window* window);

    protected:
        void formatInternal();

    private:
        SqliteWindowDefinition::Window* window = nullptr;
};

class FormatWindowDefinitionWindowFrame : public FormatStatement
{
    public:
        FormatWindowDefinitionWindowFrame(SqliteWindowDefinition::Window::Frame* frame);

    protected:
        void formatInternal();

    private:
        SqliteWindowDefinition::Window::Frame* frame = nullptr;
};

class FormatWindowDefinitionWindowFrameBound : public FormatStatement
{
    public:
        FormatWindowDefinitionWindowFrameBound(SqliteWindowDefinition::Window::Frame::Bound* bound);

    protected:
        void formatInternal();

    private:
        SqliteWindowDefinition::Window::Frame::Bound* bound = nullptr;
};

#endif // FORMATWINDOWDEFINITION_H
