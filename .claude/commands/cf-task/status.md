# cf-task:status

显示任务状态总览。

## 输入

- `/cf-task:status` — 显示所有活跃 task 文件的状态
- `/cf-task:status <file>` — 显示指定文件的详细状态

## 执行步骤

### 1. 读取任务数据

- 用 Glob 搜索 `.code-flow/tasks/**/*.md`，从结果中排除包含 `archived/` 的路径
- 逐个 Read，提取每个 `## TASK-xxx` 的：ID、标题、Status、Priority、Depends、Checklist 完成度

### 2. 输出总览表格

```
任务状态总览
============

📋 auth-module.md (来源: docs/auth-design.md)
┌──────────┬──────────────────┬────────────┬──────┬──────────────┐
│ ID       │ 标题             │ 状态       │ 优先 │ 进度         │
├──────────┼──────────────────┼────────────┼──────┼──────────────┤
│ TASK-001 │ 用户模型定义     │ done       │ P0   │ 3/3 [100%]   │
│ TASK-002 │ 注册接口实现     │ in-progress│ P0   │ 1/4 [25%]    │
│ TASK-003 │ JWT 工具函数     │ draft      │ P1   │ 0/2 [0%]     │
│ TASK-004 │ 登录接口实现     │ blocked    │ P0   │ 0/3 [0%]     │
└──────────┴──────────────────┴────────────┴──────┴──────────────┘
汇总: 4 个子任务 | done: 1 | in-progress: 1 | draft: 1 | blocked: 1
整体完成度: 25%

📋 payment-api.md (来源: docs/payment-design.md)
...

═══════════════════════════
全局汇总: 2 个文件, 8 个子任务
  done: 3 | in-progress: 2 | draft: 2 | blocked: 1
```

### 3. 阻塞详情

如果存在 blocked 子任务，额外输出：

```
⚠ 阻塞任务:
  - auth-module/TASK-004: [BLOCKED] 等待第三方 SDK 文档
  - auth-module/TASK-004: [NOTE-2] 接口签名需与前端对齐
```

### 4. 指定文件模式

当指定 `<file>` 时，输出该文件的详细信息，包括：
- 完整的子任务表格
- 每个子任务的 Checklist 明细
- 未解决的 Notes 列表
- 依赖关系简图
