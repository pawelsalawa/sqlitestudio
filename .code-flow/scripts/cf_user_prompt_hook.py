#!/usr/bin/env python3
"""Unified UserPromptSubmit Hook: inject matching specs into context.

This hook works for both Claude Code and Codex adapters. It extracts:
1. File paths from the prompt (for context_tags)
2. Keywords from the prompt (for prompt_tags via extract_prompt_tags)

Both tag sets are used to match specs for injection.

Input (stdin): {"prompt": "...", "session_id": "..."}
Output (stdout): {"hookSpecificOutput": {"hookEventName": "UserPromptSubmit", "additionalContext": "..."}}
or empty on no-op
"""
import json
import os
import re
import sys
from cf_core import (
    _log,
    assemble_context,
    build_effective_mapping,
    debug_log,
    extract_context_tags,
    extract_prompt_tags,
    fallback_domains_for_context,
    load_config,
    load_inject_state,
    match_domains,
    match_specs_by_tags,
    normalize_path,
    normalize_spec_entry,
    read_matched_specs,
    resolve_session_id,
    save_inject_state,
    select_specs_tiered,
)

# Match bare paths, @-prefixed paths, and backtick-quoted paths
_PATH_RE = re.compile(r'[@`]?([a-zA-Z0-9_.][a-zA-Z0-9_./\-]*\.[a-zA-Z]{1,6})\b')


def extract_paths_from_prompt(prompt: str) -> list:
    """Extract candidate file paths referenced in prompt text."""
    paths = []
    seen = set()
    for m in _PATH_RE.finditer(prompt):
        candidate = normalize_path(m.group(1).lstrip('@`'))
        # Require at least one slash or a meaningful extension to reduce noise
        if candidate in seen:
            continue
        if '/' in candidate or re.search(r'\.(py|js|ts|go|rs|java|rb|cs|cpp|c|h)$', candidate):
            paths.append(candidate)
            seen.add(candidate)
    return paths


def main() -> None:
    try:
        raw = sys.stdin.read()
        if not raw.strip():
            return
        data = json.loads(raw)
        prompt = data.get("prompt", "")
        if not isinstance(prompt, str) or not prompt.strip():
            return

        project_root = os.getcwd()
        
        # Resolve session_id consistently with PreToolUse hook
        sid = resolve_session_id(data)
        debug_log(f"user_prompt_hook start session={sid} prompt_len={len(prompt)}", project_root)

        config = load_config(project_root)
        if not config:
            debug_log(f"user_prompt_hook no_config session={sid}", project_root)
            return

        inject_config = config.get("inject") or {}
        if inject_config.get("auto") is False:
            return

        mapping = config.get("path_mapping") or {}
        effective_mapping = build_effective_mapping(project_root, mapping)

        budget_cfg = config.get("budget") or {}
        l1_budget = 1700
        map_max = 400
        try:
            l1_budget = int(budget_cfg.get("l1_max", 1700))
        except (ValueError, TypeError):
            pass
        try:
            map_max = int(budget_cfg.get("map_max", 400))
        except (ValueError, TypeError):
            pass

        # Derive context_tags from file paths mentioned in the prompt
        candidate_paths = extract_paths_from_prompt(prompt)
        context_tags: set = set()
        matched_domains: set = set()
        for cp in candidate_paths:
            context_tags.update(extract_context_tags(cp))
            matched_domains.update(match_domains(cp, effective_mapping))

        # NEW: Extract prompt_tags from user prompt keywords
        prompt_tags = extract_prompt_tags(prompt)
        debug_log(f"user_prompt_hook paths={candidate_paths} context_tags={sorted(context_tags)} prompt_tags={sorted(prompt_tags)}", project_root)

        # Fallback: unresolved domains → prefer domain-name hint in tags, then all domains
        if not matched_domains:
            matched_domains = fallback_domains_for_context(effective_mapping, context_tags)
            debug_log(f"user_prompt_hook fallback_domains={sorted(matched_domains)}", project_root)

        state = load_inject_state(project_root)
        if state.get("session_id") != sid:
            injected_specs: set = set()
        else:
            injected_specs = set(state.get("injected_specs") or [])

        all_matched = []
        for domain in matched_domains:
            domain_cfg = effective_mapping.get(domain) or {}
            specs_config = domain_cfg.get("specs") or []
            # Pass prompt_tags to match_specs_by_tags for Chinese/English keyword matching
            matched, has_tier1 = match_specs_by_tags(specs_config, context_tags, prompt_tags)
            if not has_tier1:
                # Fallback: load all specs for this domain
                matched = [
                    normalize_spec_entry(e) for e in specs_config
                    if normalize_spec_entry(e).get("path")
                ]
                debug_log(
                    f"user_prompt_hook fallback domain={domain} loaded={len(matched)} reason=no_tag_match",
                    project_root,
                )
            new_matched = [m for m in matched if m["path"] not in injected_specs]
            if new_matched:
                specs = read_matched_specs(project_root, domain, new_matched)
                all_matched.extend(specs)

        if not all_matched:
            debug_log(f"user_prompt_hook no_specs_matched session={sid}", project_root)
            return

        selected = select_specs_tiered(all_matched, l1_budget, map_max)
        if not selected:
            return

        new_injected = injected_specs | {s["path"] for s in selected}
        save_inject_state(project_root, {
            "session_id": sid,
            "injected_specs": sorted(new_injected),
            "last_file": "",
        })

        debug_log(f"user_prompt_hook injected={[s['path'] for s in selected]} session={sid}", project_root)

        payload = {
            "hookSpecificOutput": {
                "hookEventName": "UserPromptSubmit",
                "additionalContext": assemble_context(selected, "## Active Specs (auto-injected)"),
            }
        }
        if os.environ.get("CF_DEBUG") == "1":
            payload["debug"] = {
                "candidate_paths": candidate_paths,
                "domains": sorted(matched_domains),
                "context_tags": sorted(context_tags),
                "prompt_tags": sorted(prompt_tags),
                "matched_specs": [s["path"] for s in selected],
            }

        sys.stdout.write(json.dumps(payload))

    except Exception as exc:
        _log(f"cf_user_prompt_hook error: {exc}")
        return


if __name__ == "__main__":
    main()
