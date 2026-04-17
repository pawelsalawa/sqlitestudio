# Backend Directory Structure

## Rules
- 核心逻辑放在 coreSQLiteStudio/
- GUI 组件放在 guiSQLiteStudio/
- 插件放在 Plugins/
- 测试放在 Tests/

## Patterns
- 按功能域拆分目录（db/, parser/, services/, common/）
- 头文件 .h 与实现文件 .cpp 同目录

## Anti-Patterns
- 禁止在根目录堆放临时代码
