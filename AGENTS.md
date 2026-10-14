# AGENTS.md — Letos

## Purpose and priorities

Letos is a mature, open-source, cross-platform database tool focused on SQLite. Treat existing behavior, user data, compatibility, and maintainability as higher priorities than stylistic refactoring or architectural novelty.

Follow this priority order:
1. Explicit instructions from the current user.
2. Repository-local instructions and established architecture, where consistent with the user's instructions.
3. Verified project conventions and relevant decisions recorded in `.ai/runs/`.
4. General engineering best practices.

Never treat an earlier agent suggestion as a user-approved decision unless approval is explicitly recorded.

## Understand the repository first

- Inspect the relevant source files, build scripts, tests, and nearby implementations before editing.
- Use actual repository contents as the source of truth. Do not assume paths, targets, or commands listed here still exist.
- Keep changes focused; avoid unrelated cleanup, mass formatting, dependency upgrades, and speculative abstractions.
- Explain significant trade-offs and ask before making changes that alter architecture, persistence formats, public APIs, compatibility, or release processes.
- Prefer an existing abstraction over adding a parallel implementation.
- Do not overwrite or revert user changes. Inspect `git status` and relevant diffs before modifying files.

## Project-specific engineering guidance

### C++ and Qt
- The desktop application is based on C++ and Qt. Follow the established Qt version, C++ standard, ownership model, coding style, and build configuration found in the repository.
- Respect QObject ownership, signal/slot lifetimes, thread affinity, and model/view contracts.
- Keep potentially blocking database, filesystem, network, and device operations off the GUI thread when practical and consistent with existing architecture.
- Preserve keyboard interaction, accessibility, high-DPI behavior, and platform-specific UI expectations.
- Do not introduce Python/Qt bindings in place of C++/Qt code.

### SQLite and database safety
- Assume databases may contain valuable user data. Never perform destructive operations against a real user database as part of development or testing.
- Prefer parameter binding for values; handle identifiers and SQL dialect differences explicitly.
- Preserve transaction semantics, error propagation, schema compatibility, and behavior of existing SQLite features.
- Treat SQLite extensions, encryption, and SQLite3MultipleCiphers as compatibility-sensitive areas. Verify initialization order and connection-specific configuration rather than assuming standard SQLite behavior.
- Test against disposable fixtures, including edge cases and failure paths.

### Tcl and plugins
- Letos also contains Tcl-based functionality and plugins. Preserve established plugin contracts, scripting APIs, initialization order, and compatibility unless a change is explicitly approved.
- Check whether a change in the core application affects plugins, scripting integrations, or translated strings.

### Android integration
- Some workflows access Android application databases through ADB. Respect Android sandboxing, `run-as` limitations, permissions, SELinux, ABI differences, and the distinction between host-side tooling and code installed in the target app.
- Do not silently modify the target Android application or assume privileged/root access.

### Platforms, packaging, and releases
- Consider Windows (including x64 and ARM64), macOS, and Linux when changing paths, processes, encodings, libraries, or packaging. Consider Android-specific tooling where applicable.
- Respect established deployment, signing, notarization, installer, portable-build, and CI workflows.
- Do not change signing credentials, release metadata, distribution workflows, or publish artifacts without explicit authorization.
- Never print, commit, or copy secrets, signing identities, private keys, or credentials into run notes.

### Translations and UX
- User-facing UI strings should remain translatable through the project's existing Qt translation workflow. Follow the actual `.ts` naming conventions and plugin translation structure.
- Keep UI terminology consistent; existing English labels and translations are preferable to newly invented synonyms.
- Avoid changes that break layouts at different interface scales or in compact mode.

## Implementation workflow

1. **Orient:** Read relevant code, tests, instructions, and session history as described below.
2. **Plan:** For nontrivial work, briefly state the intended approach, assumptions, and any decisions requiring user input.
3. **Implement:** Make the smallest coherent change that solves the problem. Match surrounding code conventions.
4. **Verify:** Run the most relevant available tests, build checks, and static analysis. Do not claim checks ran unless they actually ran. If unavailable, state why.
5. **Review:** Inspect the diff for regressions, unrelated edits, secrets, generated files, and platform-specific consequences.
6. **Report and remember:** Summarize results to the user and update the session note in `.ai/runs/`.

Ask the user rather than guessing when the choice has meaningful consequences. Do not interrupt for routine, reversible implementation details that follow existing patterns.

## Persistent session memory: `.ai/runs/`

The entire `.ai/` directory is private and local-only. This directory is a history of work and decisions. It must never be committed, pushed, uploaded, or included in shared patches, archives, diagnostics, or release artifacts. It is **not** a substitute for code, tests, specifications, or direct user instructions.

### Privacy and Git exclusion

- Treat all `.ai/` contents as confidential, even if individual notes seem harmless.
- Before writing the first note, ensure `.ai/` is excluded from Git tracking.
- Do not modify the project's tracked `.gitignore` solely to manage these private notes unless the user explicitly requests it.
- Check whether any `.ai/` files are already tracked. If so, alert the user: ignore rules do not untrack existing files. Do not automatically rewrite Git history.
- Never use `git add -f` on `.ai/`, and exclude it from any bulk staging, commits, patches, uploads, or automated synchronization.
- Store only minimal technical context and decisions. Do not record credentials, tokens, private paths, personal details, sensitive customer data, or copied conversation transcripts.
- If a requested action could expose session notes outside the local machine, obtain explicit user authorization first.

### At the beginning of every session

- Check whether `.ai/runs/` exists; create it when writing the first note.
- Read the newest relevant session notes (normally up to five), then search filenames and note contents for the components, features, or decisions involved in the current task.
- Read older notes when their topics match; do not blindly load the entire archive.
- Extract applicable **user-confirmed decisions** and their rationale. Distinguish them from agent suggestions, temporary workarounds, and unresolved questions.
- Treat historical decisions as context, not immutable rules. The user's latest explicit instruction wins. If two decisions conflict and the current task depends on them, ask for clarification.
- Never infer a general user preference from a single narrow decision without evidence that it applies more broadly.

### Exactly one note per agent session

Create or update **one Markdown file for the current session** in `.ai/runs/`. A session is one continuous agent working context, even when it contains several user prompts or subtasks. Do not create a new note for each message, tool invocation, or commit.

Filename format:

`YYYY-MM-DD_HHMM_<topic>.md`

Rules:
- Use the session's local start date and time, in 24-hour format; if local time is unavailable, use UTC and record `UTC` in the note.
- `<topic>` is a concise, searchable lowercase `kebab-case` description, ideally 2–5 words, using component or feature names when useful (for example `sqlite-encryption`, `android-adb-access`, `qt-model-refactoring`).
- Keep the same filename throughout the session, even if its scope expands. Summarize additional subjects inside the file.
- If the filename already exists for a different session, append `-02`, `-03`, etc. before `.md`.
- Never overwrite or edit another session's note to make your own history look consistent.
- Prefer one concise note of roughly 10–40 lines. Longer notes are justified only by consequential decisions or unresolved technical details.

Create the note early enough that an interrupted session still leaves useful context. Update it after important user decisions and before the final response. If the environment prevents writing files, disclose that limitation in the final response; do not claim the note was saved.

### Required note structure

```markdown
# Session: <topic>

- Started: YYYY-MM-DD HH:MM <timezone>
- Scope: <components / features>
- Status: completed | partial | blocked

## Work performed
- <concise, factual changes; include useful file paths>

## User decisions
- **[CONFIRMED]** <what the user explicitly chose or rejected; why, if stated>
- **[CONFIRMED]** <scope or conditions under which that choice applies>
- None explicitly recorded.  <!-- use only if applicable -->

## Agent decisions and assumptions
- **[AGENT]** <implementation choice and rationale, not attributed to user>
- **[ASSUMPTION]** <unverified premise, if any>

## Verification
- <tests/builds actually executed and results; or not run and why>

## Follow-up
- <remaining work, risks, open questions; or None>
```

Remove unused placeholder bullets; do not leave contradictory entries such as both confirmed decisions and “None explicitly recorded.”

### Decision-recording rules

- Record **explicit user choices, corrections, rejected approaches, preferences, and their stated rationale**, especially when they may influence similar future tasks.
- Quote a short, precise formulation when ambiguity would matter; otherwise paraphrase faithfully.
- Capture *scope*: e.g. “for the Android ADB workflow” is not the same as “for all Android functionality.”
- Label independently selected implementation details `[AGENT]`, even if the user did not object.
- Do not convert silence, lack of objection, or a successful test into `[CONFIRMED]`.
- If the user reverses a previous decision, record the new choice and note the superseded decision, without rewriting historical notes.
- Avoid personal information, secrets, raw logs, large diffs, and verbose conversation transcripts.
- If there were no explicit user decisions, say so. Never fabricate decisions just to populate the section.

### Reusing memory in future work

- Prefer specific decisions that match the current component and problem over vague historical preferences.
- When a recorded decision materially influences a new implementation, briefly mention that precedent and confirm it still applies if circumstances have changed.
- When proposing an alternative to an established user decision, clearly identify the trade-off rather than silently replacing it.
- If session notes become numerous, use ordinary text search (`rg`) by component, keyword, or `[CONFIRMED]` marker. Do not build a separate indexing system without user approval.

## Communication and completion

- Be concise and technically precise. Separate observed facts, assumptions, proposals, and verified outcomes.
- Present options with trade-offs when the user needs to choose; recommend an approach when evidence supports it.
- State what changed, what was verified, and what remains uncertain.
- Before finishing, ensure the current session note is updated. The note should reflect the user's actual decisions, not just the code diff.
