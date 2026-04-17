# Backend Logging

## Rules
- 关键路径必须输出结构化日志 (使用 log.h)
- 日志中不得包含明文密钥与个人敏感信息

## Patterns
- 使用 logSql() 记录 SQL 执行
- 使用 logExecutorStep() 记录查询执行步骤
- 使用 qDebug/qWarning/qInfo/qCritical  Qt日志宏

## Anti-Patterns
- 禁止在高频循环中打印大量日志
- 禁止在 log 输出中包含用户密码
