# cf-task:block

标记子任务为阻塞状态。

## 输入

- `/cf-task:block <file> TASK-001 "阻塞原因"`

其中 `<file>` 可省略日期目录前缀和 `.md` 后缀。

查找逻辑：用 Glob 搜索 `.code-flow/tasks/**/<file>.md`，从结果中排除包含 `archived/` 的路径。如果匹配到多个结果，输出警告列出所有匹配项，让用户指定完整路径；如果只有一个结果，直接使用。

## 执行步骤

1. 用 Glob 定位任务文件，Read 读取
2. 定位 `## TASK-001` 段落，检查当前 Status：
   - `done` → 拒绝：`TASK-001 已完成，无法标记阻塞`
   - `blocked` → 提示：`TASK-001 已处于 blocked 状态`，仍追加新的阻塞原因
   - `draft` / `in-progress` → 继续
3. 用 Edit 更新 Status 为 `blocked`
4. 在 `### Log` 前追加：`> BLOCKED: <阻塞原因>`
5. 在 `### Log` 追加：`- [<当前日期>] blocked (<阻塞原因>, was <原状态>)`
6. 更新文件头 `Updated` 日期
7. 输出确认：`TASK-001 已标记为 blocked: <原因>`

## 解除阻塞

阻塞的解除不通过本命令操作，而是：
- 如果是 Notes 导致的阻塞 → 用 `/project:cf-task:note` 与 AI 讨论解决后自动解除
- 如果是 `[BLOCKED]` 标记 → 用户手动编辑文件移除 `[BLOCKED]` 条目，或用 `/project:cf-task:start` 重新启动（需所有阻塞条件清除）
