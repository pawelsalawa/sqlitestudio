# Frontend Retrieval Map

> AI 导航地图：帮助快速定位 GUI 前端代码结构和关键模块。

## Purpose

SQLiteStudio3 的 Qt GUI 前端，负责用户交互、SQL 编辑、数据库浏览和结果展示。使用 Qt Widgets 框架构建桌面应用。

## Architecture

- Framework: Qt 6.x (Widgets)
- Build: CMake + Ninja
- Language: C++17
- UI: Qt .ui files + QSS stylesheets

## Key Files

| File | Purpose |
|------|---------|
| `guiSQLiteStudio/mainwindow.cpp` | 主窗口，应用入口 |
| `guiSQLiteStudio/sqleditor.cpp` | SQL 编辑器组件 |
| `guiSQLiteStudio/datagrid/` | 数据表格组件 |
| `guiSQLiteStudio/dialogs/` | 各种对话框 |
| `guiSQLiteStudio/windows/` | 专用窗口（表编辑器、视图编辑器等）|

## Module Map

```
guiSQLiteStudio/
├── mainwindow.cpp/h         # 主窗口
├── dialogs/                 # 对话框（导入/导出/配置等）
├── windows/                 # 专用窗口（table/view/extension编辑器）
├── datagrid/                # 数据表格
├── dataview.cpp/h           # 数据视图
├── sqleditor.cpp/h          # SQL编辑器
├── multieditor/             # 多编辑器
├── qhexedit2/               # Hex编辑器
├── forms/                   # Qt .ui表单文件
└── translations/            # 国际化文件
```

## Data Flow

```
用户操作 → Qt Signal/Slot → MdiChild/Window → Db/QueryExecutor → SQL Query → Results → DataGrid/View
```

## Navigation Guide

- 新增对话框 → `dialogs/` + 对应 .ui 文件
- 新增窗口 → `windows/` + 继承 MdiChild
- SQL 编辑 → `sqleditor.cpp` + SyntaxHighlighter
- 数据展示 → `datagrid/` + QueryModel
