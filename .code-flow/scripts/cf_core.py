#!/usr/bin/env python3
import fnmatch
import json
import os
import re
import sys

# --- Config cache (fix #3: avoid re-parsing YAML on every hook call) ---

_config_cache: dict = {}


def load_config(project_root: str) -> dict:
    config_path = os.path.join(project_root, ".code-flow", "config.yml")
    if not os.path.exists(config_path):
        return {}
    try:
        mtime = os.path.getmtime(config_path)
    except OSError:
        return {}
    cached = _config_cache.get(config_path)
    if cached and cached["mtime"] == mtime:
        return cached["data"]
    try:
        import yaml
    except Exception:
        return {}
    try:
        with open(config_path, "r", encoding="utf-8") as file:
            data = yaml.safe_load(file)
        result = data or {}
        _config_cache[config_path] = {"mtime": mtime, "data": result}
        return result
    except Exception:
        return {}


def estimate_tokens(text: str) -> int:
    return len(text) // 4


def normalize_path(path: str) -> str:
    return path.replace(os.sep, "/")


def _spec_path_from_entry(entry) -> str:
    cfg = normalize_spec_entry(entry)
    return normalize_path(cfg.get("path", ""))


def discover_spec_domains(project_root: str) -> dict:
    specs_root = os.path.join(project_root, ".code-flow", "specs")
    discovered = {}
    if not os.path.isdir(specs_root):
        return discovered

    for root, _, files in os.walk(specs_root):
        for filename in files:
            if not filename.endswith(".md"):
                continue
            full_path = os.path.join(root, filename)
            rel = normalize_path(os.path.relpath(full_path, specs_root))
            parts = rel.split("/", 1)
            if len(parts) < 2:
                continue
            domain = parts[0]
            discovered.setdefault(domain, []).append(rel)

    for domain in discovered:
        discovered[domain] = sorted(set(discovered[domain]))
    return discovered


def _default_spec_entry(rel: str) -> dict:
    tier = 0 if rel.endswith("/_map.md") else 1
    return {"path": rel, "tags": ["*"], "tier": tier}


def build_effective_mapping(project_root: str, mapping: dict) -> dict:
    discovered = discover_spec_domains(project_root)
    effective = {}

    for domain, rel_paths in discovered.items():
        source_cfg = mapping.get(domain) or {}
        normalized_specs = []
        seen = set()

        for entry in source_cfg.get("specs") or []:
            rel = _spec_path_from_entry(entry)
            if not rel or rel in seen:
                continue
            normalized_specs.append(normalize_spec_entry(entry))
            seen.add(rel)

        for rel in rel_paths:
            if rel in seen:
                continue
            normalized_specs.append(_default_spec_entry(rel))
            seen.add(rel)

        effective[domain] = {
            "patterns": source_cfg.get("patterns") or [],
            "specs": normalized_specs,
        }

    for domain, source_cfg in (mapping or {}).items():
        if domain in effective:
            continue
        normalized_specs = []
        seen = set()
        for entry in source_cfg.get("specs") or []:
            rel = _spec_path_from_entry(entry)
            if not rel or rel in seen:
                continue
            normalized_specs.append(normalize_spec_entry(entry))
            seen.add(rel)
        effective[domain] = {
            "patterns": source_cfg.get("patterns") or [],
            "specs": normalized_specs,
        }

    return effective


def fallback_domains_for_context(mapping: dict, context_tags: set) -> set:
    if not mapping:
        return set()
    by_name = {domain for domain in mapping.keys() if domain.lower() in context_tags}
    if by_name:
        return by_name
    return set(mapping.keys())


def is_code_file(rel_path: str, inject_config: dict) -> bool:
    rel_path = normalize_path(rel_path)
    for pattern in inject_config.get("skip_paths") or []:
        if fnmatch.fnmatch(rel_path, pattern):
            return False
    _, ext = os.path.splitext(rel_path)
    if ext in (inject_config.get("skip_extensions") or []):
        return False
    code_exts = inject_config.get("code_extensions") or []
    return ext in code_exts


def match_domains(rel_path: str, mapping: dict) -> list:
    rel_path = normalize_path(rel_path)
    domains = []
    for domain, cfg in (mapping or {}).items():
        patterns = cfg.get("patterns") or []
        for pattern in patterns:
            if fnmatch.fnmatch(rel_path, pattern):
                domains.append(domain)
                break
    return domains


# --- Fix #2: safe depluralization with allowlist ---

_SAFE_DEPLURALS = {
    "models": "model",
    "services": "service",
    "components": "component",
    "handlers": "handler",
    "controllers": "controller",
    "middlewares": "middleware",
    "validators": "validator",
    "schemas": "schema",
    "repositories": "repository",
    "migrations": "migration",
    "fixtures": "fixture",
    "plugins": "plugin",
    "routes": "route",
    "routers": "router",
    "hooks": "hook",
    "pages": "page",
    "stores": "store",
    "styles": "style",
    "types": "type",
    "configs": "config",
    "scripts": "script",
    "tasks": "task",
    "specs": "spec",
    "tests": "test",
    "utils": "util",
    "helpers": "helper",
    "views": "view",
    "templates": "template",
    "errors": "error",
    "exceptions": "exception",
    "docs": "doc",
}

# --- Fix #1: semantic directory → concept tag mapping ---

_DIR_SEMANTIC_TAGS = {
    "handlers": ["api", "error"],
    "controllers": ["api"],
    "middleware": ["api", "config"],
    "middlewares": ["api", "config"],
    "routers": ["api", "route"],
    "routes": ["api", "route"],
    "views": ["ui", "render"],
    "templates": ["ui", "render"],
    "models": ["model", "database", "orm", "schema"],
    "model": ["model", "database", "orm", "schema"],
    "schemas": ["model", "schema", "database"],
    "migrations": ["database", "migration"],
    "repositories": ["database", "query"],
    "dao": ["database", "query"],
    "validators": ["quality", "error"],
    "exceptions": ["error", "exception"],
    "errors": ["error", "exception"],
    "auth": ["api", "config"],
    "config": ["config", "deploy"],
    "configs": ["config", "deploy"],
    "settings": ["config"],
    "tests": ["test", "quality"],
    "test": ["test", "quality"],
    "utils": ["quality"],
    "helpers": ["quality"],
    "lib": ["quality"],
    "common": ["quality"],
    "shared": ["quality"],
    "core": ["quality"],
    "logging": ["log", "logging"],
    "logger": ["log", "logging"],
    "logs": ["log", "logging"],
    "cache": ["cache", "performance"],
    "queue": ["performance"],
    "jobs": ["performance"],
    "workers": ["performance"],
}


def extract_context_tags(rel_path: str) -> set:
    """Extract context tags from a file path for spec matching.

    Uses three strategies:
    1. Directory names as tags (with safe depluralization)
    2. Semantic mapping: common directory names → concept tags
    3. Filename stem word splitting
    """
    rel_path = normalize_path(rel_path)
    tags = set()
    parts = rel_path.split("/")

    for part in parts[:-1]:
        lower = part.lower()
        tags.add(lower)
        deplural = _SAFE_DEPLURALS.get(lower)
        if deplural:
            tags.add(deplural)
        semantic = _DIR_SEMANTIC_TAGS.get(lower)
        if semantic:
            tags.update(semantic)

    filename = parts[-1] if parts else ""
    stem = os.path.splitext(filename)[0].lower()
    if stem:
        words = re.findall(r"[a-z]+", stem.replace("_", " ").replace("-", " "))
        tags.update(words)
        for word in words:
            semantic = _DIR_SEMANTIC_TAGS.get(word)
            if semantic:
                tags.update(semantic)

    return tags


# --- Fix #5: prompt-text → tag mapping (bilingual aliases) ---
#
# Maps each canonical English tag (as used in .code-flow/config.yml spec tags)
# to a list of substrings we look for in user prompt text. When a user writes
# "写一个用户登录服务，注意性能和异常处理", "性能" → performance and "异常" →
# exception, so the corresponding specs get injected at prompt submission time
# instead of waiting for the Edit/Write tool call.
#
# Kept in Python (not config.yml) because .code-flow/config.yml is category
# 'merge' in cli.js — existing installs wouldn't pick up new aliases on
# upgrade. cf_core.py is category 'tool' so it's overwritten on upgrade.
_TAG_ALIASES = {
    "quality": ["质量", "代码质量", "规范"],
    "performance": ["性能", "优化", "效率", "perf"],
    "error": ["错误", "报错"],
    "exception": ["异常", "异常处理"],
    "test": ["测试", "单元测试", "unit test"],
    "timeout": ["超时"],
    "retry": ["重试"],
    "cache": ["缓存"],
    "log": ["日志", "日志记录", "打印日志"],
    "logging": ["日志", "日志记录"],
    "database": ["数据库", "db"],
    "query": ["查询", "sql"],
    "migration": ["迁移"],
    "schema": ["数据模型", "模式"],
    "api": ["接口", "api"],
    "deploy": ["部署", "发布"],
    "config": ["配置"],
    "component": ["组件"],
    "render": ["渲染"],
    "ui": ["界面", "ui", "样式"],
    "route": ["路由"],
    "page": ["页面"],
    "state": ["状态"],
}

# ASCII aliases this short would false-match inside English words
# ("ui" inside "guide", "db" inside "idb"), so we require word boundaries.
_SHORT_ASCII_ALIAS_THRESHOLD = 3


def _is_short_ascii(token: str) -> bool:
    if len(token) > _SHORT_ASCII_ALIAS_THRESHOLD:
        return False
    try:
        token.encode("ascii")
        return True
    except UnicodeEncodeError:
        return False


def extract_prompt_tags(prompt_text) -> set:
    """Scan prompt text for alias hits and return canonical tags.

    - Lowercases ASCII (Chinese is unaffected by .lower()).
    - Short ASCII aliases are matched with word boundaries to avoid false
      positives like "guide" matching "ui".
    - Chinese aliases and long English aliases use plain substring match.
    - Silent on empty/non-string input.
    """
    if not isinstance(prompt_text, str) or not prompt_text.strip():
        return set()
    lower = prompt_text.lower()
    hits = set()
    for canonical, aliases in _TAG_ALIASES.items():
        candidates = [canonical, *aliases]
        for alias in candidates:
            needle = alias.lower()
            if _is_short_ascii(needle):
                if re.search(r"\b" + re.escape(needle) + r"\b", lower):
                    hits.add(canonical)
                    break
            else:
                if needle in lower:
                    hits.add(canonical)
                    break
    return hits


def normalize_spec_entry(entry) -> dict:
    """Normalize spec config entry. Supports both old (string) and new (dict) format."""
    if isinstance(entry, str):
        return {"path": entry, "tags": ["*"], "tier": 1}
    if isinstance(entry, dict):
        return {
            "path": entry.get("path", ""),
            "tags": entry.get("tags") or ["*"],
            "tier": entry.get("tier", 1),
        }
    return {}


def match_specs_by_tags(
    specs_config: list, context_tags: set, prompt_tags: set = None
) -> tuple:
    """Return (matched_specs, has_tier1_match).

    Wildcard tag '*' always matches (used by tier 0 specs like _map.md).
    prompt_tags (if provided) are unioned with context_tags so that Chinese/
    English keywords from the user prompt can also gate tier 1 specs.
    The caller uses has_tier1_match to decide whether to fallback to all specs.
    """
    effective_tags = context_tags if prompt_tags is None else (context_tags | prompt_tags)
    matched = []
    has_tier1_match = False
    for entry in specs_config:
        cfg = normalize_spec_entry(entry)
        if not cfg.get("path"):
            continue
        spec_tags = set(cfg.get("tags") or [])
        if "*" in spec_tags:
            matched.append(cfg)
        elif spec_tags & effective_tags:
            matched.append(cfg)
            if cfg.get("tier", 1) != 0:
                has_tier1_match = True
    return matched, has_tier1_match


def read_matched_specs(project_root: str, domain: str, matched: list) -> list:
    """Read only the matched spec files from disk."""
    specs_root = os.path.join(project_root, ".code-flow", "specs")
    specs = []
    for cfg in matched:
        rel = cfg["path"]
        spec_path = os.path.join(specs_root, rel)
        try:
            with open(spec_path, "r", encoding="utf-8") as f:
                content = f.read().strip()
            if not content:
                continue
            specs.append(
                {
                    "path": rel,
                    "content": content,
                    "tokens": estimate_tokens(content),
                    "domain": domain,
                    "tier": cfg.get("tier", 1),
                }
            )
        except Exception:
            continue
    return specs


# --- Kept for backward compatibility with old config format ---


def read_specs(project_root: str, domain: str, domain_cfg: dict) -> list:
    specs_root = os.path.join(project_root, ".code-flow", "specs")
    specs = []
    for entry in domain_cfg.get("specs") or []:
        cfg = normalize_spec_entry(entry)
        rel = cfg.get("path", "")
        if not rel:
            continue
        spec_path = os.path.join(specs_root, rel)
        try:
            with open(spec_path, "r", encoding="utf-8") as file:
                content = file.read().strip()
            if not content:
                continue
            specs.append(
                {
                    "path": rel,
                    "content": content,
                    "tokens": estimate_tokens(content),
                    "domain": domain,
                    "tier": cfg.get("tier", 1),
                }
            )
        except Exception:
            continue
    return specs


def select_specs(specs: list, budget: int, priorities: dict) -> list:
    """Legacy select by priority. Used by cf-inject manual command."""
    if budget <= 0:
        return []

    def priority(spec: dict) -> int:
        value = priorities.get(spec.get("path"))
        if isinstance(value, int):
            return value
        try:
            return int(value)
        except Exception:
            return 1000

    ordered = sorted(specs, key=lambda spec: (priority(spec), spec.get("path", "")))
    selected = []
    total = 0
    for spec in ordered:
        if total + spec.get("tokens", 0) <= budget:
            selected.append(spec)
            total += spec.get("tokens", 0)
    return selected


def select_specs_tiered(specs: list, budget: int, map_max: int = 400) -> list:
    """Tier-aware spec selection.

    Tier 0: included if within map_max budget (fix #4).
    Tier 1: budget-controlled, ordered by list position (preserved).
    """
    tier0 = [s for s in specs if s.get("tier", 1) == 0]
    tier1 = [s for s in specs if s.get("tier", 1) != 0]

    selected = []
    for spec in tier0:
        if spec.get("tokens", 0) <= map_max:
            selected.append(spec)
        else:
            _log(
                f"WARNING: {spec['path']} exceeds map_max budget "
                f"({spec['tokens']} > {map_max} tokens), skipped"
            )

    total = 0
    for spec in tier1:
        tokens = spec.get("tokens", 0)
        if total + tokens <= budget:
            selected.append(spec)
            total += tokens
    return selected


def assemble_context(specs: list, heading: str) -> str:
    parts = [heading]
    tier0 = [s for s in specs if s.get("tier", 1) == 0]
    tier1 = [s for s in specs if s.get("tier", 1) != 0]

    if tier0:
        parts.append("### Navigation (Retrieval Map)")
        for spec in tier0:
            parts.append(f"#### {spec['path']}")
            parts.append(spec["content"])

    if tier1:
        parts.append("### Constraints (matched by file context)")
        for spec in tier1:
            parts.append(f"#### {spec['path']}")
            parts.append(spec["content"])

    parts.append("---")
    parts.append("以上规范是本次开发的约束条件，生成代码必须遵循。")
    return "\n\n".join(parts)


def load_inject_state(project_root: str) -> dict:
    state_path = os.path.join(project_root, ".code-flow", ".inject-state")
    try:
        with open(state_path, "r", encoding="utf-8") as file:
            data = json.load(file)
        if isinstance(data, dict):
            return data
    except Exception:
        return {}
    return {}


def save_inject_state(project_root: str, payload: dict) -> None:
    state_path = os.path.join(project_root, ".code-flow", ".inject-state")
    try:
        with open(state_path, "w", encoding="utf-8") as file:
            json.dump(payload, file)
    except Exception:
        return


def _log(msg: str) -> None:
    """Log to stderr (fix #9: don't pollute stdout which is hook output)."""
    print(msg, file=sys.stderr)


def resolve_session_id(hook_data: dict) -> str:
    """Derive a session id consistent across PreToolUse / UserPromptSubmit hooks.

    Prefer the session_id Claude Code / Codex pass in the hook payload so all
    hooks within one session share the same id (fix #10 — previously
    cf_inject_hook used PID while cf_codex_user_prompt_hook used the hook-
    provided id, causing .inject-state dedup to break and specs to be
    re-injected). Falls back to the current PID when the hook runtime omits
    the field (older Codex versions, unit tests).
    """
    if isinstance(hook_data, dict):
        sid = hook_data.get("session_id")
        if sid:
            return str(sid)
    return str(os.getpid())


def debug_log(msg: str, project_root: str = None) -> None:
    """Append a debug line to .code-flow/.debug.log when CF_DEBUG=1.
    Silent no-op unless CF_DEBUG=1 so default runs don't pay any IO cost.
    Writes to a dotfile so cf_scan.py skips it and processDir upgrades don't overwrite it.
    Failures (missing dir, unwritable fs) are swallowed on purpose
    — we must never break the hook JSON protocol over logging.
    """
    if os.environ.get("CF_DEBUG") != "1":
        return
    root = project_root or os.getcwd()
    debug_dir = os.path.join(root, ".code-flow")
    log_path = os.path.join(debug_dir, ".debug.log")
    try:
        os.makedirs(debug_dir, exist_ok=True)
        from datetime import datetime
        ts = datetime.now().isoformat(timespec="seconds")
        with open(log_path, "a", encoding="utf-8") as f:
            f.write(f"{ts} {msg}\n")
    except Exception:
        return
