# cf-inject

强制重新注入指定领域的编码规范（通常不需要手动调用）。

## 自动注入机制

本项目的规范注入是**全自动**的，通过两层机制保障：

1. **CLAUDE.md 指令**：Claude 根据问题自动判断领域，读取 `_map.md` 导航地图理解项目结构
2. **PreToolUse Hook**：编辑代码文件时根据文件路径标签自动注入匹配的约束 specs

两层职责分离：CLAUDE.md 负责导航（Tier 0），Hook 负责约束注入（Tier 1）。

正常情况下**无需手动调用**此命令。

## 何时使用

- 需要强制刷新已注入的规范（如 spec 文件刚被修改）
- 想要预览某个领域的完整规范内容
- 自动注入未生效时的排查手段
- Hook 标签匹配遗漏了某些 spec，需要手动补充

## 输入

- `/project:cf-inject frontend` — 强制加载前端全部 specs
- `/project:cf-inject backend` — 强制加载后端全部 specs

## 执行步骤

1. 用 Read 读取 `.code-flow/config.yml`，获取指定领域的 `specs` 列表
2. 提取所有 spec 的 `path` 字段（兼容新旧格式）
3. 按 tier 排序：tier 0（`_map.md`）在前，tier 1 按列表顺序
4. 用 Read 逐个读取 `.code-flow/specs/` 下的匹配文件
5. 将规范内容直接输出到对话中，格式：

```
## Active Specs (manual inject)

### Navigation (Retrieval Map)

#### [domain]/_map.md
[导航地图内容]

### Constraints (all specs for domain)

#### [spec-path]
[spec 内容]

---
以上规范是本次开发的约束条件，生成代码必须遵循。
```

6. 用 Write 更新 `.code-flow/.inject-state`，将该域所有 spec 路径标记为已注入，防止 Hook 重复注入

> 注：手动 inject 会加载该域的**全部** spec，不做标签过滤。这是与 Hook 自动注入的区别——Hook 按标签精准匹配，手动注入用于全量预览或强制刷新。
