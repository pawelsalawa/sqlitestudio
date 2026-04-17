# cf-scan

审计规范文件的 token 分布，检测冗余和过时内容，输出优化建议。

## 输入

- `/project:cf-scan` — 完整审计
- `/project:cf-scan --json` — 仅输出原始 JSON
- `/project:cf-scan --only-issues` — 仅显示有问题的文件
- `/project:cf-scan --limit=N` — 限制输出行数

## 执行步骤

### 1. 调用 Python 脚本

用 Bash 执行：

```bash
python3 .code-flow/scripts/cf_scan.py [--json] [--only-issues] [--limit=N]
```

将用户传入的参数原样透传。

### 2. 解析输出

解析 stdout 的 JSON 输出，格式如下：

```json
{
  "files": [
    {"path": "CLAUDE.md", "tokens": 650, "percent": "26%", "issues": []},
    {"path": "specs/frontend/component-specs.md", "tokens": 420, "percent": "17%", "issues": ["冗余: '结构化日志' 3处重复"]}
  ],
  "total_tokens": 2150,
  "budget": 2500,
  "warnings": []
}
```

### 3. 格式化输出

将 JSON 格式化为人类可读的表格：

| 文件 | Tokens | 占比 | 问题 |
|------|--------|------|------|
| CLAUDE.md | ~650 | 26% | - |
| specs/frontend/component-specs.md | ~420 | 17% | - |
| **合计** | **~2150** | **/ 2500** | |

如有 warnings，在表格下方输出优化建议。

## 异常处理

- `.code-flow/` 不存在 → 提示运行 `/project:cf-init`
- Python 脚本执行失败 → 输出错误信息，建议检查 Python 环境和 pyyaml 安装
- spec 文件全为空模板 → 输出 warning，提示先填充规范内容
