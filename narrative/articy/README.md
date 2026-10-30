# articy:draft X FREE — Emberfall dialogue workflow

Status on 2026-10-30: **working native MCP integration, articy minimized**.
Seven conversations, 41 lines, two speakers and 34 native connections are saved
in the real client. The structural-child readback bug is fixed. All nine MCP
tools were exercised against the native project. Read `QA.json` for evidence;
`RESUME.md` now describes a resolved historical pause.

The user's standing preference from 2026-10-30 is to plan Emberfall's **story
and lore**, as well as quests, characters and dialogues, in this native articy
project while the program stays in the background. Existing canon is the
baseline; draft proposals must be distinguished from approved game content.
The current MCP handles dialogues and their speakers. Other native lore object
types need an appropriate extension of this same free MDK bridge when used;
storing a loose JSON alone must not be reported as authoring inside articy.

## What is available

- Installed client: `C:\Program Files\articy draft X\bin\ArticyDraft.exe`,
  version **4.4.0.43891**, using .NET 10. Its legacy EXE bootstrap did not open a
  window in this session; invoking its installed DLL with installed .NET did.
- Plugin source: `tools/narrative/Emberfall.DialogueBridge`.
  Plugin **1.0.1**, compiled against official Articy.MDK 2.5.0,
  net10.0-windows, x64.
- Installed plugin:
  `%APPDATA%\Articy Software\articy\4.x\Plugins\Local\Emberfall.DialogueBridge\1.0.1`.
  A packaged `.mdk` is in `C:\AI2D\narrative`. No native app libraries are bundled.
- Official portable SDK **10.0.403** and MDK package:
  `C:\AI2D\Tools\articy-dev`. SDK SHA-512 was checked against Microsoft's
  release metadata; provenance is copied to `dependency-provenance.json`.
- Project MCP configuration: `.codex/config.toml`, server `emberfall_articy`.
  The current chat exposes all nine native MCP tools. CLI below uses the
  identical local transport. Start every session with a fresh `articy_status`.
- `plans/`: exact UTF-8 snapshots of **7 current conversations / 41 lines** from
  `src/intro_story.hpp` and `src/tower_story.hpp`, with source hashes. These are
  **source plans**, not native exports. All seven now exist in articy.
  Actual native exports and job audit are in `exports/`. Exported C++ compiles
  and matches both source headers byte for byte (including BOM/CRLF), so no
  runtime source, quest behavior, executable or player package change was needed.

## Free license and activation

Use the official [FREE registration](https://www.articy.com/en/articydraft/free/).
The vendor emails an access key after the form is submitted. Downloading an
installer alone does not prove a key has been requested. If already registered,
check Spam/Offers and contact the vendor through official support if necessary.
The human user enters their key in the client; never put it in chat, scripts,
MCP jobs, repository files or logs.

FREE has no time limit and allows unlimited projects and commercial use, with
**700 objects per project divided into categories**. The native object counter
and license restrictions remain authoritative. The bridge does not alter these
limits or provide unlimited objects. Work within a normal project budget.
No paid AI extension is used. The integration uses the free client MDK, not
the separately licensed server Articy API.

Official references:

- [Free MDK availability](https://www.articy.com/help/adx/Integrations_DevKit.html)
- [MDK setup](https://www.articy.com/adxdevkit/html/per_plugin_setup.htm)
- [MDK packaging](https://www.articy.com/adxdevkit/html/packaging_a_plugin.htm)
- [Official DebugManifest startup option](https://www.articy.com/adxdevkit/html/debugging_a_plugin.htm)
- [Standalone server API license requirements](https://www.articy.com/en/downloads/api/)

## Commands after activation

Run PowerShell 7 (`pwsh`) from the repository. Python is the already available
runtime; these scripts use its standard library only.

```powershell
$py = 'C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
$env:PYTHONIOENCODING = 'utf-8'

# Rebuild/install only after source changes. No forced process termination.
pwsh -NoProfile -File tools/narrative/Install-ArticyBridge.ps1

# Start minimized with the public MDK DebugManifest option. Select
# "Emberfall Dialogues" once in the client after a new process starts.
pwsh -NoProfile -File tools/narrative/Start-ArticyBridge.ps1

& $py tools/narrative/articy_cli.py status
```

Verify a **live queued status response**, not merely configuration/heartbeat
files. The established project is
`C:\AI2D\narrative\EmberfallDialogues\ProjectInfo.aph`, GUID
`cb99e03a-ae90-4443-b79f-7f8da1539c34`. The launcher is restricted to it.

One startup limitation remains: the supported standalone `OpenProject` API is
multi-user/server only. No working single-user project-opening CLI was verified.
After a fresh client process, the user must select the existing project once.
Do not invent startup flags or use a paid server API. The launcher starts the
client minimized and an owned background watcher; after project selection it
minimizes only that verified project window. Current MCP operation requires no
mouse, restoration or focus. `articy_background.py` can also minimize an already
open, identity-checked worker through Win32 window management without input.

```powershell
# Imports only missing mapped conversations. Unchanged existing ones are skipped;
# differences from native edits cause refusal instead of silent overwrite.
& $py tools/narrative/story_sync.py import-plans narrative/articy/plans --output C:/AI2D/narrative/first-native-import.json

# Export exact native nodes/speakers/connections to a fresh file.
& $py tools/narrative/articy_cli.py export_story --output C:/AI2D/narrative/native-story.json

# Validate and stage C++ copies. This never overwrites production headers.
& $py tools/narrative/story_sync.py stage C:/AI2D/narrative/native-story.json C:/AI2D/narrative/staged-story

# Deliberately apply an approved native dialogue edit to the game's mapped arrays.
# Compiles and runs a hidden/silent C++ probe, retains source backups, checks
# source hashes before replacement, and writes only changed files.
& $py tools/narrative/story_sync.py apply C:/AI2D/narrative/native-story.json C:/AI2D/narrative/applied-story
```

All 41 texts, speakers and native connections were compared against the game.
The original prologue was read after closing/reopening the client; its native
object identity was preserved and its failed import was not replayed. Six
remaining conversations were created once. A real native edit/CAS-conflict/
restore/save/export test passed; all final text matches the game. This session
did not perform another full client restart after importing the six new dialogues.
Existing dialogue updates require the returned revision; single-line edits
require expected text. Never overwrite native edits with an old source snapshot.

Only after that round trip succeeds, review staged header diffs, preserve source
backups, apply them and run the game's appropriate checks. Current game dialogue
is linear. The exporter rejects branches, cycles, missing links, unknown speakers
and unmapped conversations instead of silently losing narrative logic. Extending
the game's branching runtime is a separate implementation task.

## Transport and recovery

MCP tools: `articy_status`, `articy_list_dialogues`, `articy_read_dialogue`,
`articy_export_story`, `articy_upsert_dialogue`, `articy_set_line`, `articy_connect`,
`articy_save`, `articy_job_status`.

STDIO JSON-RPC goes to the local Python process. It writes atomic JSON jobs in
`%LOCALAPPDATA%\Emberfall\ArticyBridge`. The in-client WPF dispatcher processes
them using public MDK calls. No web port, upload, cloud account or AI service.
The queue is restricted to stable `EF_` IDs; no arbitrary script evaluation,
license manipulation, file imports, project deletion or user-project closing.

- Writes require matching project GUID and a project name starting `Emberfall`.
- Default job deadline is 15 seconds. Expired jobs are rejected before execution.
- An error or timeout may follow a partially applied native operation. Inspect
  `articy_job_status` and the actual project; **never blindly replay mutations**.
- Requests, processing copies and responses retain IDs; completed jobs are not
  executed again. UI text changes are protected by compare-and-set/revisions.
- `backups/<job-id>.json` contains a snapshot of owned dialogue data before a
  mutation. It is not a complete native project backup or a transaction rollback.
- Automatic deletion/shortening is refused. Conservative workflow budgets are
  200 fragments, 15 dialogues and 50 entities; these are bridge safety budgets,
  not a claim about the vendor's category allocations. Native limits still apply.
- No native connection is reported while heartbeat is absent/stale. A file-based
  fixture test cannot prove the installed client loaded the plugin.

## Verification completed here

```powershell
& $py tools/narrative/test_articy_bridge.py
codex mcp get emberfall_articy --json
```

**32 offline checks passed**, including complete-batch/stale-source preflight,
real MCP STDIO protocol exchange,
simulated IPC/UTF-8 response, missing/stale connection rejection, project guard,
timeout/no replay, size/path checks, exact source extraction, output preservation,
branch/cycle rejection, escaping and a real compiled/executed C++ staging fixture.
The native adapter distinguishes DialogueFragments from structural Connection
and Pin children and refuses unsupported narrative node types/nonempty scripts.
Native child-type metadata and explicit connections are retained in each export.
The C++ CLI fixture runs hidden and silently. The game itself was not launched.
Plugin compilation: zero errors/warnings. All nine MCP tools passed real native
use, including conflict rejection, text restoration, duplicate-connection
protection, existing-dialogue upsert and historical job inspection. Real native
exports compile into the exact game sources. The actual game's logic test suite
passed **194,309 checks**, with hidden execution and `--no-audio --background-test`.
Articy remained minimized during import, editing, saving and export.

Before-repair and blank native ZIP backups are in `C:\AI2D\narrative`.
Final source-application receipts and backups are in
`C:\AI2D\narrative\final-game-export-20261030`; with unchanged dialogue this
application is intentionally a no-op. An isolated changed-text fixture verifies
that a future real edit is compiled, applied and backed up correctly.
