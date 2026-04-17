# cf-init

项目规范体系一键初始化。检测技术栈，生成完整的 .code-flow/ 目录、spec 模板、配置文件和 Hook 配置，并自动扫描项目填充真实规范。

## 输入

- `/project:cf-init` — 自动检测技术栈
- `/project:cf-init frontend` — 强制前端项目
- `/project:cf-init backend` — 强制后端项目
- `/project:cf-init fullstack` — 强制全栈项目
- `/project:cf-init --skip-learn` — 跳过自动扫描，仅生成模板

## 执行步骤

### 1. 检测技术栈

用 Glob 扫描项目根目录：

- `package.json` 存在 → 前端项目。用 Read 读取，检查 dependencies 中的 react/vue/@angular/core 确定框架。
- `pyproject.toml` 或 `requirements.txt` 存在 → Python 后端
- `go.mod` 存在 → Go 后端
- 同时存在前后端标识 → 全栈项目
- 均不存在 → Generic（前后端均生成）

如果用户指定了 `frontend|backend|fullstack` 参数，跳过检测直接使用。

### 2. 生成 .code-flow/config.yml

如果文件不存在，用 Write 生成。如果已存在，用 Read 读取后仅补充缺失的顶层 key。

模板内容：

```yaml
version: 1

budget:
  total: 2500
  l0_max: 800
  l1_max: 1700
  map_max: 400

inject:
  auto: true
  code_extensions:
    - ".py"
    - ".go"
    - ".ts"
    - ".tsx"
    - ".js"
    - ".jsx"
    - ".java"
    - ".rs"
    - ".rb"
    - ".vue"
    - ".svelte"
  skip_extensions:
    - ".md"
    - ".txt"
    - ".json"
    - ".yml"
    - ".yaml"
    - ".toml"
    - ".lock"
    - ".csv"
    - ".xml"
    - ".svg"
    - ".png"
    - ".jpg"
  skip_paths:
    - "docs/**"
    - "*.config.*"
    - ".code-flow/**"
    - ".claude/**"
    - "node_modules/**"
    - "dist/**"
    - "build/**"
    - ".git/**"

path_mapping:
  frontend:
    patterns:
      - "src/components/**"
      - "src/pages/**"
      - "src/hooks/**"
      - "src/styles/**"
      - "**/*.tsx"
      - "**/*.jsx"
      - "**/*.css"
      - "**/*.scss"
    specs:
      - path: "frontend/_map.md"
        tags: ["*"]
        tier: 0
      - path: "frontend/directory-structure.md"
        tags: ["directory", "structure", "folder", "route", "page", "layout"]
        tier: 1
      - path: "frontend/quality-standards.md"
        tags: ["quality", "type", "lint", "error", "test", "state", "strict"]
        tier: 1
      - path: "frontend/component-specs.md"
        tags: ["component", "prop", "hook", "render", "ui", "style"]
        tier: 1
  backend:
    patterns:
      - "services/**"
      - "api/**"
      - "models/**"
      - "**/*.py"
      - "**/*.go"
    specs:
      - path: "backend/_map.md"
        tags: ["*"]
        tier: 0
      - path: "backend/directory-structure.md"
        tags: ["directory", "structure", "folder", "module", "layout"]
        tier: 1
      - path: "backend/logging.md"
        tags: ["log", "logging", "debug", "trace", "monitor", "observe"]
        tier: 1
      - path: "backend/database.md"
        tags: ["database", "db", "sql", "orm", "model", "migration", "query", "table", "schema"]
        tier: 1
      - path: "backend/platform-rules.md"
        tags: ["api", "deploy", "config", "version", "compatibility", "release", "flag"]
        tier: 1
      - path: "backend/code-quality-performance.md"
        tags: ["quality", "performance", "error", "exception", "test", "timeout", "retry", "cache"]
        tier: 1
```

根据检测结果，只保留相关的 path_mapping 条目（仅前端项目删除 backend，仅后端项目删除 frontend）。

> **自定义域扩展**：用户可在 `path_mapping` 中添加任意域（如 `infra`、`mobile`、`shared`），遵循相同的 patterns/specs/tags 结构。在 `.code-flow/specs/` 下创建对应目录和 `_map.md`，Hook 会自动识别。示例：
> ```yaml
> infra:
>   patterns: ["infra/**", "terraform/**", "*.tf", "Dockerfile", "docker-compose.yml"]
>   specs:
>     - path: "infra/_map.md"
>       tags: ["*"]
>       tier: 0
>     - path: "infra/deployment-rules.md"
>       tags: ["deploy", "docker", "terraform", "ci", "pipeline"]
>       tier: 1
> ```

### 3. 裁剪 .code-flow/validation.yml

`code-flow init` 已把默认模板（来源 `src/core/code-flow/validation.yml`）拷贝到 `.code-flow/validation.yml`，**不要重写整份文件**。

用 Read 打开它，根据步骤 1 确定的技术栈，用 Edit 删除不相关的 validator：

- 纯前端项目：删除 `Python 类型检查`、`Pytest`
- 纯后端项目（Python/Go）：删除 `TypeScript 类型检查`、`Vue 类型检查`、`ESLint`、`Stylelint`、`前端单元测试`
- fullstack：全部保留
- 其他语言栈（Go/Rust/Java）：按实际工具链增删，格式保持与模板一致

若 `.code-flow/validation.yml` 因异常缺失，提示用户重跑 `code-flow init`，不要手工拼装 yaml。

### 4. 生成 spec 文件

**严格按步骤 1 确定的 stack 生成，禁止超出范围：**

| stack | 生成目录 | 禁止生成 |
|-------|---------|---------|
| `frontend` | `specs/frontend/` | `specs/backend/`（不得创建） |
| `backend` | `specs/backend/` | `specs/frontend/`（不得创建） |
| `fullstack` / `generic` | 两者都生成 | — |

在 `.code-flow/specs/` 下，按确定的 stack 生成 spec 模板。

**约束规范**遵循统一格式：

```markdown
# [规范名称]

## Rules
- 规则1

## Patterns
- 推荐模式

## Anti-Patterns
- 禁止模式
```

**导航地图**（`_map.md`）遵循 Retrieval Map 格式：

```markdown
# [Domain] Retrieval Map

> AI 导航地图：帮助快速定位代码结构和关键模块。

## Purpose
[项目角色描述]

## Architecture
[技术栈和架构模式]

## Key Files
| File | Purpose |
|------|---------|

## Module Map
[目录树形图]

## Data Flow
[数据流向]

## Navigation Guide
[做 X 去哪里的快速指引]
```

前端项目生成：
- `.code-flow/specs/frontend/_map.md`
- `.code-flow/specs/frontend/directory-structure.md`
- `.code-flow/specs/frontend/quality-standards.md`
- `.code-flow/specs/frontend/component-specs.md`

后端项目生成：
- `.code-flow/specs/backend/_map.md`
- `.code-flow/specs/backend/directory-structure.md`
- `.code-flow/specs/backend/logging.md`
- `.code-flow/specs/backend/database.md`
- `.code-flow/specs/backend/platform-rules.md`
- `.code-flow/specs/backend/code-quality-performance.md`

**已存在的 spec 文件不覆盖**。

### 5. 生成 CLAUDE.md

如果 CLAUDE.md 不存在，用 Write 生成 L0 模板：

```markdown
# Project Guidelines

## Team Identity
- Team: [team name]
- Project: [project name]
- Language: [primary language]

## Core Principles
- All changes must include tests
- Single responsibility per function (<= 50 lines)
- No loose typing or silent exception handling
- Handle errors explicitly

## Forbidden Patterns
- Hard-coded secrets or credentials
- Unparameterized SQL
- Network calls inside tight loops

## Spec Loading
This project uses the code-flow two-tier spec system.

**Two-tier architecture**:
- **Tier 0 `_map.md`（导航地图）**：项目结构、关键文件、数据流。你手动读取，帮助理解代码在哪里。
- **Tier 1 约束规范**：编码规则、模式、反模式。由 Hook 根据文件路径标签自动注入，你无需手动加载。

**Your responsibility**:
1. Determine domain from the question:
   - **frontend**: components, pages, hooks, styles, UI, .tsx/.jsx/.css
   - **backend**: services, API, database, models, logging, .py/.go
2. Read `.code-flow/specs/<domain>/_map.md` for navigation context
3. Constraint specs are auto-injected by PreToolUse Hook when you edit code — do NOT manually read them
4. If question spans multiple domains, read all matching `_map.md` files
5. If no domain matches, skip spec loading

Do NOT ask the user which specs to load — decide automatically based on context.
```

如果 CLAUDE.md 已存在，用 Read 读取后仅补充缺失的 `##` 段落（不覆盖已有内容）。展示 diff 供用户确认。

### 6. 生成 .claude/settings.local.json Hook 配置

用 Read 检查是否存在。如果不存在，用 Write 生成：

```json
{
  "hooks": {
    "PreToolUse": [
      {
        "matcher": "Edit|Write|MultiEdit",
        "hooks": [
          {
            "type": "command",
            "command": "python3 .code-flow/scripts/cf_inject_hook.py",
            "timeout": 5
          }
        ]
      }
    ],
    "SessionStart": [
      {
        "hooks": [
          {
            "type": "command",
            "command": "python3 .code-flow/scripts/cf_session_hook.py"
          }
        ]
      }
    ]
  }
}
```

如果已存在，用 Read 读取 JSON，仅合并 `hooks` 字段中缺失的事件条目，保留其他配置（如 permissions）不变。用 Write 回写。

### 7. 安装 pyyaml

用 Bash 执行：

```bash
python3 -m pip install pyyaml
```

成功 → 继续。失败 → 输出 warning（"请手动安装: pip install pyyaml"），不阻塞。

### 8. 自动扫描项目规范（Auto-learn）

如果传入 `--skip-learn` 参数，跳过此步骤。否则自动执行以下扫描流程。

> 此步骤等同于内联执行 `cf-learn --map`，目的是让 init 完成后 specs 就包含项目真实规范，而不是空壳模板。

#### 8.1 扫描项目配置文件

用 Glob 查找并 Read 读取以下配置文件（存在则提取约束）：

**前端配置**：
- `.eslintrc*` / `eslint.config.*` — lint 规则（no-any、import 排序、命名规范等）
- `tsconfig.json` — strict 模式、path alias、target
- `.prettierrc*` / `prettier.config.*` — 格式化规则
- `tailwind.config.*` — 自定义 theme
- `next.config.*` / `nuxt.config.*` / `vite.config.*` — 框架约束
- `jest.config.*` / `vitest.config.*` — 测试配置

**后端配置**：
- `pyproject.toml` — ruff/mypy/pytest 配置、Python 版本
- `.golangci.yml` — Go lint 规则
- `Makefile` — 构建和测试命令
- `Dockerfile` / `docker-compose.yml` — 运行时约束

**通用配置**：
- `.github/workflows/*.yml` / `.gitlab-ci.yml` — CI 检查步骤
- `.editorconfig` — 编辑器配置
- `package.json` scripts — 常用命令

#### 8.2 扫描代码结构和模式

用 Glob + Grep 扫描项目代码：

**结构扫描**（用于填充 `_map.md`）：
- `src/**/*` 顶层目录结构 → Module Map
- 入口文件识别（main.ts/py、index.ts、app.ts 等）→ Key Files
- 从 dependencies 推断技术栈 → Architecture
- README 或 package.json description → Purpose
- 路由/handler → service → model 调用链 → Data Flow

**模式扫描**（用于填充约束 specs）：
- 错误处理：自定义 Error 类、try/catch 模式
- 日志：使用的库和格式
- 测试：框架、断言风格、mock 方式
- 导入：absolute vs relative、barrel exports
- 命名：文件命名、变量命名风格

#### 8.3 填充 spec 文件

将扫描结果**直接写入**对应的 spec 文件（因为是 init 阶段，文件刚创建，内容是模板占位符）：

**填充 `_map.md`**：
- 用步骤 1 检测到的框架信息 + 步骤 8.2 的结构扫描结果，替换模板中的占位符
- Purpose → 从 README/package.json 提取
- Architecture → 从 dependencies 推断
- Key Files → 列出扫描到的入口文件
- Module Map → 基于实际目录结构生成
- Data Flow → 从代码模式推断
- Navigation Guide → 基于模块划分生成

**填充约束 specs**：
- 将扫描到的具体规则追加到对应 spec 文件的 `## Rules` 段落
- 将扫描到的代码模式追加到 `## Patterns` 段落
- 过滤掉纯格式化规则（有 Prettier 等 formatter 自动处理的）
- 过滤掉与模板已有规则重复的条目

#### 8.4 展示扫描结果

不直接写入，先展示供用户确认：

```
项目规范扫描完成:

导航地图 (Retrieval Map):
  frontend/_map.md:
    Purpose: 基于 React 18 + TypeScript 的管理后台
    Architecture: Vite + React Router + Zustand + Tailwind
    Key Files: 6 个入口文件
    Modules: 5 个模块目录

  backend/_map.md:
    Purpose: 基于 FastAPI 的 RESTful API 服务
    Architecture: FastAPI + SQLAlchemy + PostgreSQL
    Key Files: 4 个入口文件
    Modules: 6 个模块目录

编码约束 (从配置和代码中提取):
  全局:
    1. [x] [tsconfig.json] strict 模式，禁止 implicit any
    2. [x] [CI] PR 必须通过 lint + type check + test

  frontend/quality-standards.md:
    3. [x] [.eslintrc] exhaustive-deps 规则已启用
    4. [x] [代码模式] 组件使用 PascalCase 命名

  backend/code-quality-performance.md:
    5. [x] [pyproject.toml] 使用 ruff + mypy --strict
    6. [x] [代码模式] API handler 统一使用 async def

确认写入？（all 全部写入 / 输入编号选择 / skip 跳过）:
```

用户确认后写入。如果用户输入 `skip`，保留模板不填充。

### 9. 输出摘要

```
cf-init 完成 ✓

技术栈: React 18 + TypeScript / FastAPI + Python 3.11

文件结构:
  Created:
    + .code-flow/config.yml
    + .code-flow/validation.yml
    + .code-flow/specs/frontend/_map.md (已填充)
    + .code-flow/specs/frontend/directory-structure.md (+3 条规则)
    + .code-flow/specs/frontend/quality-standards.md (+2 条规则)
    + .code-flow/specs/frontend/component-specs.md
    + .code-flow/specs/backend/_map.md (已填充)
    + .code-flow/specs/backend/directory-structure.md (+2 条规则)
    + .code-flow/specs/backend/logging.md
    + .code-flow/specs/backend/database.md
    + .code-flow/specs/backend/platform-rules.md
    + .code-flow/specs/backend/code-quality-performance.md (+3 条规则)
    + CLAUDE.md
    + .claude/settings.local.json (Hook)

  Skipped:
    · (已存在的文件)

Token 估算:
  CLAUDE.md: ~200 tokens
  Specs 合计: ~800 tokens
  导航地图: ~300 tokens

下一步:
  - 审阅并补充 spec 文件中的规范内容
  - 运行 /cf-learn 补充更多规范
  - 运行 /cf-learn --map 更新导航地图
  - 开始开发: /cf-task:plan <设计文档>
```
