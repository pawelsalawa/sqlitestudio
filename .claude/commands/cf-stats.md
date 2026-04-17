# cf-stats

统计规范体系的 token 使用情况，包括各文件分布和预算利用率。

## 输入

- `/project:cf-stats` — 完整统计
- `/project:cf-stats --human` — 人类可读格式
- `/project:cf-stats --domain=frontend` — 仅统计指定领域

## 执行步骤

### 1. 调用 Python 脚本

用 Bash 执行：

```bash
python3 .code-flow/scripts/cf_stats.py [--human] [--domain=frontend]
```

将用户传入的参数原样透传。

### 2. 解析输出

解析 stdout 的 JSON 输出，格式如下：

```json
{
  "l0": {"file": "CLAUDE.md", "tokens": 650, "budget": 800},
  "l1": {
    "frontend": [
      {"path": "frontend/directory-structure.md", "tokens": 180},
      {"path": "frontend/quality-standards.md", "tokens": 150}
    ],
    "backend": [
      {"path": "backend/database.md", "tokens": 200}
    ]
  },
  "total_tokens": 1580,
  "total_budget": 2500,
  "utilization": "63%"
}
```

### 3. 格式化输出

将 JSON 格式化为人类可读的统计信息：

```
L0 (CLAUDE.md): ~650 / 800 tokens

L1 Frontend:
  - directory-structure.md: ~180 tokens
  - quality-standards.md: ~150 tokens

L1 Backend:
  - database.md: ~200 tokens

Total: ~1580 / 2500 tokens (63%)
```

## 异常处理

- `.code-flow/` 不存在 → 提示运行 `/project:cf-init`
- Python 脚本执行失败 → 输出错误信息，建议检查 Python 环境
