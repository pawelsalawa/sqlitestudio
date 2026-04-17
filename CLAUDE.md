# Project Guidelines

## Team Identity
- Team: SQLiteStudio Contributors
- Project: SQLiteStudio3 - SQLite Database Manager
- Language: C++ with Qt framework

## Core Principles
- All changes must include tests
- Single responsibility per function (<= 50 lines)
- No loose typing or silent exception handling
- Handle errors explicitly
- Use Qt conventions (Q_OBJECT, signals/slots, QString)

## Forbidden Patterns
- Hard-coded secrets or credentials
- Unparameterized SQL (use bindings)
- Network calls inside tight loops
- Using STL containers instead of Qt equivalents (QString, QList, QHash, QVariant)

## Architecture
- **Framework**: Qt 6.x (Widgets, Core)
- **Build**: CMake with Ninja
- **Testing**: Qt Test framework
- **Language**: C++17

## Project Structure
- `SQLiteStudio3/coreSQLiteStudio/` - Core library (database, parser, services)
- `SQLiteStudio3/guiSQLiteStudio/` - Qt GUI application
- `SQLiteStudio3/sqlitestudio/` - Main application entry point
- `SQLiteStudio3/sqlitestudiocli/` - CLI interface
- `SQLiteStudio3/Tests/` - Unit tests
- `Plugins/` - Plugin system

## CI Requirements
- All PRs must pass `ctest` unit tests
- Build uses CMake with Qt6

## Spec Loading
This project uses the code-flow two-tier spec system.

**Two-tier architecture**:
- **Tier 0 `_map.md`（导航地图）**：项目结构、关键文件、数据流。
- **Tier 1 约束规范**：编码规则、模式、反模式。

**Your responsibility**:
1. Determine domain from file path:
   - **frontend**: guiSQLiteStudio components (.cpp/.h with Qt widgets)
   - **backend**: coreSQLiteStudio services, db, parser (.cpp/.h)
2. Read `.code-flow/specs/<domain>/_map.md` for navigation context
3. Constraint specs are auto-injected by PreToolUse Hook when you edit code
4. If question spans multiple domains, read all matching `_map.md` files

Do NOT ask the user which specs to load — the system handles constraint injection automatically.
