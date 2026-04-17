# Frontend Directory Structure (Qt GUI)

## Rules
- GUI 组件放在 guiSQLiteStudio/
- 对话框放在 dialogs/
- 窗口放在 windows/
- Qt .ui 表单放在 forms/

## Patterns
- .ui 文件与对应 .cpp/.h 同名
- 组件按功能域分子目录

## Anti-Patterns
- 禁止在 guiSQLiteStudio 外新增 GUI 代码
