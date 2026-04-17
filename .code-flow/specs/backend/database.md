# Backend Database

## Rules
- 所有 SQL 查询必须使用参数绑定，禁止字符串拼接
- 使用 Db::Flags 控制查询行为
- 查询执行通过 QueryExecutor 链式步骤

## Patterns
- 使用 sqlQuery() 获取查询结果
- 使用 sqlExec() 执行无返回的 DDL/DML
- 批量操作使用事务

## Anti-Patterns
- 禁止在事务内发起外部网络调用
- 禁止直接调用 sqlite3_* 函数（通过 Db 抽象）
