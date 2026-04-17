#!/usr/bin/env python3
import json
import os
import sys

from cf_core import (
    _log,
    assemble_context,
    build_effective_mapping,
    debug_log,
    extract_context_tags,
    fallback_domains_for_context,
    is_code_file,
    load_config,
    load_inject_state,
    match_domains,
    match_specs_by_tags,
    normalize_spec_entry,
    read_matched_specs,
    resolve_session_id,
    save_inject_state,
    select_specs_tiered,
)





def main() -> None:
    try:
        raw = sys.stdin.read()
        if not raw.strip():
            return
        data = json.loads(raw)
        tool_name = data.get("tool_name", "")
        tool_input = data.get("tool_input") or {}
        file_path = tool_input.get("file_path", "")
        if tool_name not in {"Edit", "Write", "MultiEdit"}:
            return
        if not isinstance(file_path, str) or not file_path:
            return

        project_root = os.getcwd()
        abs_path = file_path
        if not os.path.isabs(abs_path):
            abs_path = os.path.join(project_root, file_path)
        rel_path = os.path.relpath(abs_path, project_root)

        config = load_config(project_root)
        if not config:
            return
        inject_config = config.get("inject") or {}
        if inject_config.get("auto") is False:
            return
        if not is_code_file(rel_path, inject_config):
            return

        mapping = config.get("path_mapping") or {}
        effective_mapping = build_effective_mapping(project_root, mapping)
        domains = match_domains(rel_path, effective_mapping)

        # Load state with session isolation (fix #10)
        sid = resolve_session_id(data)
        state = load_inject_state(project_root)
        state_sid = state.get("session_id", "")
        if state_sid != sid:
            injected_specs = set()
        else:
            injected_specs = set(state.get("injected_specs") or [])

        # Extract context tags from file path
        context_tags = extract_context_tags(rel_path)
        if not domains:
            domains = sorted(fallback_domains_for_context(effective_mapping, context_tags))
            if not domains:
                return

        # Budget config
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

        # Match specs by tags per domain, with fallback (fix #1)
        all_matched = []
        for domain in domains:
            domain_cfg = effective_mapping.get(domain) or {}
            specs_config = domain_cfg.get("specs") or []
            matched, has_tier1_match = match_specs_by_tags(specs_config, context_tags)

            # Fallback: if no tier 1 spec matched by tags, load ALL tier 1 specs
            if not has_tier1_match:
                matched = [normalize_spec_entry(e) for e in specs_config if normalize_spec_entry(e).get("path")]
                debug_log(
                    f"inject_hook fallback domain={domain} path={rel_path} loaded={len(matched)} reason=no_tag_match",
                    project_root,
                )

            # Filter already-injected
            new_matched = [m for m in matched if m["path"] not in injected_specs]
            if new_matched:
                specs = read_matched_specs(project_root, domain, new_matched)
                all_matched.extend(specs)

        if not all_matched:
            return

        selected = select_specs_tiered(all_matched, l1_budget, map_max)
        if not selected:
            return

        # Update state with newly injected spec paths
        new_injected = injected_specs | {s["path"] for s in selected}
        debug_log(f"inject_hook injected={[s['path'] for s in selected]} path={rel_path}", project_root)
        save_inject_state(project_root, {
            "session_id": sid,
            "injected_specs": sorted(new_injected),
            "last_file": abs_path,
        })

        payload = {
            "hookSpecificOutput": {
                "hookEventName": "PreToolUse",
                "additionalContext": assemble_context(
                    selected, "## Active Specs (auto-injected)"
                ),
            }
        }
        if os.environ.get("CF_DEBUG") == "1":
            payload["debug"] = {
                "target": abs_path,
                "domains": sorted(domains),
                "context_tags": sorted(context_tags),
                "matched_specs": [s["path"] for s in selected],
            }
        sys.stdout.write(json.dumps(payload))
    except Exception as exc:
        # Fix #9: log errors to stderr instead of silently swallowing
        _log(f"cf_inject_hook error: {exc}")
        return


if __name__ == "__main__":
    main()
