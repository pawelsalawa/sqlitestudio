#!/usr/bin/env python3
import json
import os
import re
import sys

from cf_core import estimate_tokens, load_config


PATH_PATTERN = re.compile(r"(?:[\w./-]+/)+[\w.-]+\.[A-Za-z0-9]+")


def read_text(path: str) -> str:
    try:
        with open(path, "r", encoding="utf-8") as file:
            return file.read().strip()
    except Exception:
        return ""


def normalize_line(line: str) -> str:
    return " ".join(line.strip().split())


def find_redundant_lines(specs: list) -> dict:
    line_map = {}
    for spec in specs:
        for raw in spec["content"].splitlines():
            line = normalize_line(raw)
            if not line:
                continue
            if line.startswith("#") or line == "---":
                continue
            if len(line) < 6:
                continue
            line_map.setdefault(line, set()).add(spec["path"])
    redundant = {line: paths for line, paths in line_map.items() if len(paths) >= 3}
    return redundant


def find_missing_paths(text: str, project_root: str) -> list:
    missing = []
    for token in set(PATH_PATTERN.findall(text)):
        if token.startswith("http://") or token.startswith("https://"):
            continue
        abs_path = token if os.path.isabs(token) else os.path.join(project_root, token)
        if not os.path.exists(abs_path):
            missing.append(token)
    return sorted(missing)


def main() -> None:
    project_root = os.getcwd()
    files = []
    total_tokens = 0
    json_output = "--json" in sys.argv
    only_issues = "--only-issues" in sys.argv
    limit = None
    for arg in sys.argv[1:]:
        if arg.startswith("--limit="):
            try:
                limit = int(arg.split("=", 1)[1])
            except Exception:
                limit = None

    claude_path = os.path.join(project_root, "CLAUDE.md")
    if os.path.exists(claude_path):
        content = read_text(claude_path)
        tokens = estimate_tokens(content)
        total_tokens += tokens
        files.append({"path": "CLAUDE.md", "tokens": tokens, "issues": []})

    specs_root = os.path.join(project_root, ".code-flow", "specs")
    specs = []
    if os.path.isdir(specs_root):
        for root, _, filenames in os.walk(specs_root):
            for name in filenames:
                if not name.endswith(".md"):
                    continue
                full_path = os.path.join(root, name)
                content = read_text(full_path)
                if not content:
                    continue
                tokens = estimate_tokens(content)
                total_tokens += tokens
                rel = os.path.relpath(full_path, specs_root)
                rel_path = os.path.join("specs", rel).replace(os.sep, "/")
                spec_entry = {
                    "path": rel_path,
                    "tokens": tokens,
                    "issues": [],
                    "content": content,
                }
                specs.append(spec_entry)

    redundant_map = find_redundant_lines(specs)

    for spec in specs:
        issues = []
        if spec["tokens"] > 500:
            issues.append(f"冗长: {spec['tokens']} tokens")

        missing_paths = find_missing_paths(spec["content"], project_root)
        for path in missing_paths[:3]:
            issues.append(f"过时: 路径不存在 {path}")

        redundant_lines = [line for line, paths in redundant_map.items() if spec["path"] in paths]
        for line in redundant_lines[:3]:
            issues.append(f"冗余: '{line}' 出现于 {len(redundant_map[line])} 个文件")

        files.append({"path": spec["path"], "tokens": spec["tokens"], "issues": issues})

    for entry in files:
        if total_tokens <= 0:
            entry["percent"] = "0%"
        else:
            entry["percent"] = f"{round(entry['tokens'] * 100 / total_tokens)}%"

    config = load_config(project_root)
    budget = (config.get("budget") or {}).get("total", 2500)
    try:
        budget = int(budget)
    except Exception:
        budget = 2500

    output = {
        "files": files,
        "total_tokens": total_tokens,
        "budget": budget,
        "warnings": [],
    }
    if limit is not None:
        files = files[:limit]

    if only_issues:
        files = [entry for entry in files if entry.get("issues")]

    if json_output:
        output = {
            "files": files,
            "total_tokens": total_tokens,
            "budget": budget,
            "warnings": [],
        }
        print(json.dumps(output, ensure_ascii=False))
        return

    print("FILE | TOKENS | PERCENT | ISSUES")
    for entry in files:
        issues = entry.get("issues") or []
        issue_text = " / ".join(issues) if issues else "-"
        print(f"{entry['path']} | {entry['tokens']} | {entry['percent']} | {issue_text}")
    print("TOTAL:", f"{total_tokens} / {budget}")


if __name__ == "__main__":
    main()
