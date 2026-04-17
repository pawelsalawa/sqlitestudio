#!/usr/bin/env python3
import json
import os


def main() -> None:
    """Reset inject state for this session.

    Instead of deleting the state file (which breaks other sessions),
    write a fresh state with the current session's PID.
    Other sessions will detect the PID mismatch and reset their own state.
    """
    try:
        project_root = os.getcwd()
        state_path = os.path.join(project_root, ".code-flow", ".inject-state")
        payload = {
            "session_id": str(os.getpid()),
            "injected_specs": [],
            "last_file": "",
        }
        with open(state_path, "w", encoding="utf-8") as f:
            json.dump(payload, f)
    except Exception:
        return


if __name__ == "__main__":
    main()
