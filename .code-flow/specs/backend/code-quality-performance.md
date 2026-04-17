# Backend Code Quality & Performance

## Rules
- 关键路径必须有结构化日志 (使用 log.h)
- 所有导出类使用 API_EXPORT macro
- 单个函数不超过 50 行

## Patterns
- 使用 QueryExecutorStep 链式处理查询
- 使用 Db::Flags 控制查询行为
- 内存管理使用 Qt smart pointers

## Anti-Patterns
- 禁止在查询执行路径中吞掉异常
- 禁止直接字符串拼接 SQL（使用参数绑定）
- 禁止在 tight loop 中进行日志输出
