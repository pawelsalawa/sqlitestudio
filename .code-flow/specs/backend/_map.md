# Backend Retrieval Map

> AI 导航地图：帮助快速定位核心业务逻辑代码结构和关键模块。

## Purpose

SQLiteStudio3 的核心业务逻辑层，负责 SQLite 数据库操作、SQL 解析/格式化、查询执行、插件管理和服务注册。

## Architecture

- Framework: Qt 6.x (Core)
- ORM: 直接 SQLite C API (sqlite3)
- Build: CMake + Ninja
- Language: C++17

## Key Files

| File | Purpose |
|------|---------|
| `coreSQLiteStudio/db/` | 数据库抽象层和查询执行 |
| `coreSQLiteStudio/parser/` | SQL 解析器 (tokenizer, parser) |
| `coreSQLiteStudio/services/` | 服务管理器（插件、配置、函数等）|
| `coreSQLiteStudio/log.h` | SQL 执行日志 |
| `coreSQLiteStudio/datatype.cpp/h` | 数据类型系统 |

## Module Map

```
coreSQLiteStudio/
├── db/                      # 数据库层
│   ├── dbsqlite3.cpp/h     # SQLite3 实现
│   ├── queryexecutor.cpp/h  # 查询执行器
│   └── queryexecutorsteps/ # 执行步骤
├── parser/                  # SQL 解析器
│   ├── ast/                # AST 节点
│   ├── lexer.cpp/h         # 词法分析
│   └── sqlite3_parse.cpp  # 语法分析
├── services/               # 服务层
│   └── impl/              # 服务实现
├── config_builder/         # 配置系统
├── common/                # 通用工具
└── log.h                  # 日志工具
```

## Data Flow

```
SQL Query → Lexer (tokens) → Parser (AST) → QueryExecutor → SQLite3 API → Results
```

## Navigation Guide

- 数据库操作 → `db/` + QueryExecutor
- SQL 解析 → `parser/` + Lexer/Parser
- 新增服务 → `services/` + PluginManager 注册
- 配置系统 → `config_builder/`
