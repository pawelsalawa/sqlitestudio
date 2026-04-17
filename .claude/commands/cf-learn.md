# cf-learn

自动扫描项目配置文件和代码模式，提取隐含的编码约束和团队规范，呈现给用户确认后写入 CLAUDE.md 或 spec 文件。支持生成 Retrieval Map（导航地图）和基于当前工作区变更提炼新规范。

## 输入

- `/project:cf-learn` — 全量扫描
- `/project:cf-learn frontend` — 仅扫描前端相关
- `/project:cf-learn backend` — 仅扫描后端相关
- `/project:cf-learn --map` — 扫描并生成/更新 Retrieval Map（导航地图）
- `/project:cf-learn frontend --map` — 仅生成前端导航地图
- `/project:cf-learn --review` — 基于当前工作区变更提炼可沉淀的规范建议（默认 staged + unstaged + untracked 代码文件）
- `/project:cf-learn --review --staged` — 仅分析 staged 变更

## 执行步骤

### 1. 扫描项目配置文件

用 Glob 查找以下配置文件（存在则 Read 读取内容）：

**前端配置**：
- `.eslintrc*` / `eslint.config.*` — 提取 lint 规则（no-any、import 排序、命名规范等）
- `tsconfig.json` — 提取 strict 模式、path alias、target 等关键配置
- `.prettierrc*` / `prettier.config.*` — 提取格式化规则（缩进、引号、分号）
- `tailwind.config.*` — 提取自定义 theme、spacing 规则
- `next.config.*` / `nuxt.config.*` / `vite.config.*` — 提取框架特定约束
- `jest.config.*` / `vitest.config.*` — 提取测试配置（覆盖率阈值等）

**后端配置**：
- `pyproject.toml` — 提取 ruff/mypy/pytest 配置、Python 版本要求
- `setup.cfg` / `tox.ini` — 提取测试和 lint 配置
- `.golangci.yml` — 提取 Go lint 规则
- `Makefile` — 提取构建和测试命令
- `Dockerfile` / `docker-compose.yml` — 提取运行时约束

**通用配置**：
- `.github/workflows/*.yml` / `.gitlab-ci.yml` — 提取 CI 检查步骤（哪些 lint/test 是必须通过的）
- `.editorconfig` — 提取编辑器统一配置
- `.gitignore` — 解析为扫描排除规则（而非仅参考）
- `package.json` 的 scripts 字段 — 提取常用命令

在进入代码扫描前，先构建 **统一排除集**：
- 默认排除：`.git/**`、`.code-flow/**`、`.claude/**`、`.codex/**`、`.agents/**`、`.codex_flow/**`、`node_modules/**`、`dist/**`、`build/**`、`coverage/**`、`.next/**`、`.venv/**`、`venv/**`、`__pycache__/**`
- 额外排除所有隐藏目录（名称以 `.` 开头），但保留白名单 `.github/workflows/**` 用于 CI 规则提取
- 从 `.gitignore` 追加排除模式（忽略空行和注释行）
- 所有 Glob/Grep/Read 仅针对“未被排除”的路径执行

### 2. 扫描代码结构和模式（遵循排除集）

用 Grep 在项目代码中搜索以下模式，提取隐含规范：

- 错误处理模式：`try/except`、`catch`、自定义 Error 类的使用方式
- 日志模式：使用的日志库和格式（structlog、winston、pino 等）
- 测试模式：测试框架、断言风格、mock 方式
- 导入规范：absolute vs relative imports、barrel exports
- 命名模式：文件命名（kebab-case / PascalCase）、变量命名风格

**代码结构扫描**（用于 Retrieval Map）：
- 用 Glob 扫描 `src/**/*` 顶层目录结构（先应用统一排除集）
- 识别入口文件（main.ts/py、index.ts、app.ts 等）
- 识别框架和技术栈（从 package.json dependencies、pyproject.toml 等提取）
- 识别模块划分方式（按功能域 / 按技术层 / 混合）
- 追踪关键数据流（路由 → handler → service → model）

### 3. 呈现扫描发现，用户选择聚焦域

将扫描结果按模块/功能域分组展示，让用户选择关注的领域：

```
项目扫描完成，发现以下模块/功能域：

前端:
  1. [x] components/ — 23 个组件文件，React + TypeScript
  2. [x] pages/ — 8 个页面，使用 React Router
  3. [ ] styles/ — Tailwind CSS 配置
  4. [x] hooks/ — 12 个自定义 hook

后端:
  5. [x] api/ — 15 个路由文件，FastAPI
  6. [x] services/ — 10 个业务逻辑模块
  7. [ ] models/ — 8 个 ORM 模型
  8. [x] middleware/ — 认证 + 日志中间件

选择要分析的模块（输入编号，all 全选，或 skip 跳过直接生成）：
```

用户选择后，仅对选中的模块深入扫描代码模式。未选中的模块跳过详细分析。

> 注：这一步让用户控制分析范围，避免在不关心的模块上浪费 token，也让生成的规范更有针对性。

### 4. 综合分析并生成候选约束

将扫描结果综合分析，提取 **具体的、可执行的** 编码约束。每条约束格式：

```
[来源] 约束描述
```

例如：
```
[tsconfig.json] strict 模式已启用，禁止 implicit any
[.eslintrc] import 必须按 builtin → external → internal 排序
[pyproject.toml] Python 最低版本 3.11，可使用 match/case 语法
[CI: lint.yml] PR 必须通过 ruff check + mypy --strict
[代码模式] 错误处理统一使用自定义 AppError 类，不使用裸 Exception
[Makefile] 测试命令为 make test，覆盖率阈值 80%
```

**过滤规则**：
- 跳过已在 CLAUDE.md 或 spec 文件中记录的规范（避免重复）
- 只提取对 AI 生成代码有实际影响的约束
- 忽略纯格式化规则（如果有 Prettier/formatter 自动处理）

### 5. 呈现给用户确认

将候选约束分组展示：

```
扫描发现以下未记录的编码约束：

全局约束（建议写入 CLAUDE.md）：
  1. [x] [tsconfig.json] strict 模式已启用，禁止 implicit any
  2. [x] [CI] PR 必须通过 lint + type check
  3. [ ] [.editorconfig] 缩进使用 2 空格

前端约束（建议写入 specs/frontend/）：
  4. [x] [.eslintrc] React hooks 必须遵循 exhaustive-deps 规则
  5. [x] [代码模式] 组件文件使用 PascalCase 命名

后端约束（建议写入 specs/backend/）：
  6. [x] [pyproject.toml] 使用 ruff 替代 flake8/isort
  7. [x] [代码模式] 所有 API handler 使用 async def

确认要写入的条目（输入编号，或 all 全部写入，或 none 跳过）：
```

等待用户确认。

### 6. 写入确认的条目

根据用户选择，将每条约束**分类后插入对应章节**：

- **规则类**（必须遵守的硬性约束）→ 追加到目标文件的 `## Rules` 段落
- **模式类**（推荐的实现方式）→ 追加到目标文件的 `## Patterns` 段落
- **反模式类**（明确禁止的做法）→ 追加到目标文件的 `## Anti-Patterns` 段落

写入目标：
- **全局约束** → 用 Edit 追加到 `CLAUDE.md` 的 `## Core Principles` 或 `## Forbidden Patterns`
- **域约束** → 询问用户写入哪个 spec 文件，用 Edit 追加到对应章节

每条写入后输出确认。

### 7. Retrieval Map 生成（--map 或自动建议）

如果传入 `--map` 参数，或者检测到 `_map.md` 文件内容仍为初始模板（含 `[一句话描述` 占位符），自动进入 Map 生成流程：

1. 基于步骤 2 的代码结构扫描结果，填充 `_map.md` 的各个段落：
   - **Purpose**：从 README 或 package.json description 提取项目描述
   - **Architecture**：从依赖和配置文件推断技术栈
   - **Key Files**：列出入口文件和核心模块文件（用 Read 验证存在性）
   - **Module Map**：基于实际目录结构生成树形图
   - **Data Flow**：从代码模式推断数据流向
   - **Navigation Guide**：基于现有模式生成"做 X 去哪里"的快速指引

2. 展示生成的 Map 内容，等待用户确认或调整

3. 用户确认后，用 Write 写入 `.code-flow/specs/<domain>/_map.md`

```
Retrieval Map 已生成:

frontend/_map.md:
  Purpose: 基于 React 18 + TypeScript 的管理后台
  Architecture: Vite + React Router + Zustand + Tailwind
  Key Files: 6 个入口文件
  Modules: 7 个模块
  Navigation: 4 条导航规则

确认写入？可先修改再确认:
```

### 8. 输出摘要

```
已写入 N 条约束：
- CLAUDE.md: +3 条
- specs/frontend/quality-standards.md: +2 条
- specs/backend/code-quality-performance.md: +2 条
- specs/frontend/_map.md: 已更新导航地图
Token 变化: CLAUDE.md 138 → 195 tokens
```

## --review 模式：基于当前变更提炼规范

当传入 `--review` 参数时，跳过步骤 1-8，执行以下流程：

### R1. 采集当前变更范围

运行以下命令获取工作区变更：

```bash
git diff --name-only
git diff --cached --name-only
git ls-files --others --exclude-standard
```

合并去重后得到候选文件集，并过滤：
- 仅保留代码文件（`.py`、`.js`、`.ts`、`.tsx`、`.jsx`、`.go`、`.rs` 等）
- 排除配置、文档和生成产物
- 路径过滤遵循步骤 1 的统一排除集与 `.gitignore`

### R2. 读取变更内容与上下文

对候选文件读取变更：

```bash
git diff -- <file>
git diff --cached -- <file>
```

如果是新文件（untracked）没有 diff 上下文，则用 Read 读取完整文件内容作为分析输入。

### R3. 提炼候选规范模式

从当前变更中提炼“值得沉淀为 spec 的稳定规则”：

**提取维度**：
- 新增的强约束（Rule）：例如统一错误处理、统一校验入口、必须补测试
- 新增的推荐实现（Pattern）：例如目录组织、接口分层、命名习惯
- 新增的明确禁用（Anti-Pattern）：例如裸异常吞掉、重复配置解析、stdout 非协议输出

**置信度规则**：
- 同一模式在多个文件中重复出现：高置信度
- 仅单点改动但语义明确：低置信度（保留，展示时排后）

### R4. 去重过滤

将候选规则与**现有 spec 文件**和 **CLAUDE.md** 对比，已覆盖的规则标记为 `[已覆盖]` 并跳过，仅保留**未覆盖**的候选规则。

### R5. 呈现给用户确认

按 domain 分组、按置信度排序展示，等待用户确认要写入的条目。展示内容必须附带“来源文件 + 变更证据片段”，便于快速判断是否值得沉淀。

### R6. 写入确认的条目

与主流程步骤 6 相同：
- 按分类（Rule / Pattern / Anti-Pattern）插入对应 spec 文件的对应章节
- 全局规则写入 `CLAUDE.md` 的 `## Core Principles` 或 `## Forbidden Patterns`
- 每条写入后输出确认

### R7. 输出摘要

```
review 完成：
- 扫描变更文件: 9（staged: 3, unstaged: 4, untracked: 2）
- 提取候选规则: 5（高置信度: 3, 低置信度: 2）
- 已覆盖跳过: 1
- 用户确认写入: 3
```

## 异常处理

- 无配置文件可扫描 → 提示项目可能未初始化，建议手动添加
- 未发现新约束 → 输出"未发现未记录的约束，当前规范已覆盖项目配置"
- `.code-flow/` 不存在 → 提示运行 `/project:cf-init`
- `_map.md` 已有自定义内容 → 展示 diff，让用户选择合并方式
- 无变更文件 → 提示"当前工作区无代码变更，先完成改动后再运行 --review"
- 仅文档/配置变更 → 提示"未检测到代码变更，暂无可沉淀的实现规范"
