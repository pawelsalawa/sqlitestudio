# cf-validate

根据变更文件自动匹配并执行验证规则（测试、类型检查、lint），失败时自动尝试修复。

## 输入

- `/project:cf-validate` — 基于 git diff 自动获取变更文件
- `/project:cf-validate src/Foo.tsx` — 验证指定文件
- `/project:cf-validate --files=src/a.ts,src/b.ts` — 验证多个文件

## 执行步骤

### 1. 获取变更文件列表

用 Bash 执行：

```bash
git diff --name-only HEAD
```

如果用户指定了文件路径，使用用户指定的文件。如果无变更文件，输出"无变更需要验证"。

### 2. 读取验证规则

用 Read 读取 `.code-flow/validation.yml`。如果不存在，尝试读取 `package.json` 中的 `scripts.test` 和 `scripts.lint` 作为回退。

validation.yml 格式：

```yaml
validators:
  - name: "验证器名称"
    trigger: "**/*.{ts,tsx}"
    command: "npx tsc --noEmit"
    timeout: 30000
    on_fail: "修复建议"
```

### 3. 匹配并执行验证

对每条验证规则：
- 将 `trigger` glob 与变更文件列表做匹配
- 匹配到 → 用 Bash 执行 `command`
  - 如果 command 包含 `{files}` → 替换为匹配到的文件路径列表，**每个路径用单引号包裹**（如 `'src/foo.ts' 'src/bar.ts'`）
  - 如果 command 不含 `{files}` → 直接执行原命令（如 `npx tsc --noEmit` 本身就检查全局）
- 未匹配 → 跳过该规则

每条命令使用对应的 `timeout` 值（毫秒）。

### 4. 汇总结果

- **全部通过** → 输出确认信息
- **有失败** → 展示每个失败项的：
  - 验证器名称
  - 执行的命令
  - 错误输出（截取关键部分）
  - `on_fail` 修复建议

### 5. 自动修复

对于失败的验证项，根据错误输出分析问题根因，自动尝试修复代码。修复后提示用户再次运行 `/project:cf-validate` 确认。

## 安全设计

- 对 `{files}` 中的每个文件路径用单引号包裹，防止 shell 注入
- 仅接受 `git diff` 输出的文件路径或用户显式指定的路径
- 用户指定的路径必须位于项目根目录内

## 异常处理

- validation.yml 不存在 → 尝试检测 package.json scripts 中的 test/lint 命令
- 命令执行超时 → 输出超时提示，建议增大 timeout 或缩小验证范围
- 命令不存在 → 提示安装依赖（如 `pip install mypy`）
- 无变更文件 → 输出"无变更需要验证"
