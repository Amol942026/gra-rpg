# Local workflow

The local media-production environment is configured. Before a media task read
`art/pipeline/ASSET_PIPELINE.md`, `art/pipeline/STYLE.md` and the current
`art/pipeline/REPORT.md`. Follow the documented Audacity retention workaround.

The user requires silent development work. Every agent-initiated game launch,
benchmark, smoke test, and graphical/audio diagnostic must pass `--no-audio`.
Do not play game audio through the laptop speakers or change the user's saved
audio preferences to achieve this. Normal launches by the user retain audio.

Game tests must also stay hidden or minimized (user preference, 28.10.2026).
Use `--no-audio --background-test` for the actual game executable, and hidden
windows in QA helpers. Do not focus or restore test windows. A minimized or
hidden window cannot validate displayed FPS: report completed render time
separately and do not claim it proves foreground frame pacing.

# Audacity automation

Local audio editing is configured through Audacity 3.7.9 and its enabled
`mod-script-pipe` module. Use the tested Python integration in
`C:\AI2D\audio\scripts`; do not install an MCP server for this workflow.
Audacity is installed at `C:\Program Files\Audacity\Audacity.exe`.

- Read `C:\AI2D\audio\scripts\README-Audacity.md` before an audio editing job.
- Transport: `audacity_control.py`, `AudacityClient`; native Windows
  `\\.\pipe\ToSrvPipe` and `\\.\pipe\FromSrvPipe`. Never test pipe existence
  with `os.path.exists()`. The client uses Win32 readiness checks, deadlines,
  cancellation, reconnects and a cross-process lock.
- Editing: `audacity_workflow.py`, `AudacityProject`; create an owned project or
  open a private working copy of an AUP3, import, trim, change gain, apply
  effects, normalize, mix, export a new PCM16 WAV and save a new AUP3.
  Finish with `release_saved_project()`: Audacity 3.7.9 can crash while closing
  edited projects, including via normal window close. `close_project()` is
  intentionally blocked. Never re-enable native Close as a routine cleanup.
- Batch entry point: `audacity_process.py --recipe <absolute-path.json>`.
  `audacity_recipe_example.json` is a tested example; choose fresh output and
  project paths before reusing it. Existing outputs are rejected.
- Up to eight saved worker projects may remain open, tracked in
  `C:\AI2D\audio\scripts\audacity-managed-projects.json`. The shared pipeline
  automatically uses FFmpeg/Python for supported WAV recipes once this budget
  is reached. Its report has `project=null` and stores an editable JSON recipe;
  it never claims to have produced an AUP3 through FFmpeg.
- Available Python runtime:
  `C:\Users\Amadeusz\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe`.
  Production modules use the Python standard library. NumPy is used only by
  the sample-comparison acceptance tests.
- Audacity work is silent: never send Play, Record or Scrub commands and never
  alter saved audio preferences to mute the app. The game-specific
  `--no-audio` switch above remains mandatory for game launches; Audacity
  editing uses the supplied client, which blocks playback commands.
- The client checks the process and both pipe owners, then begins with Help
  and GetInfo. Editing operations focus and validate only their owned project
  window. Do not manually edit Audacity while an automated batch is running.
  Never close or modify unrelated user projects. Saved worker windows remaining
  open are deliberate; source and output files are already written and checked.
- Keep originals unchanged. Use new output paths, retain source SHA-256
  checks, command JSONL logs and job reports. Export validation checks all WAV
  samples, duration, peak and clipping without playback.
- Retry connection checks and safe queries only. After a mutating command
  times out, inspect its actual result; never automatically replay it.
  The transport synchronizes delayed responses with a unique Message fence.
- Native OpenProject2 can report Failed after opening successfully; the
  supplied workflow verifies the unique working-copy window and GetInfo.
  Use SelectTime for time-only selections. Normalize parameters are checked
  against the running app's schema, including ApplyVolume in this version.
- C:\AI2D writes and desktop pipe/window access may need an approved command
  outside the Codex filesystem sandbox. Do not interpret sandbox invisibility
  as proof that the desktop Audacity process is stopped.

Verification of the current production workflow: four real Audacity jobs
(music and three SFX), four valid saved AUP3s, and 15 independent final
acceptance checks including the real offline fallback. Older 19+3 component
reports remain historical evidence; their project-close behavior was replaced
after the full pipeline exposed an Audacity crash. Use the current report.

# Local graphics, animation and audio production

Use only free local tools and commercially permitted asset inputs. No paid
API, cloud processing or project upload. New graphics are authored in Krita;
new animation uses Synfig. Work in 2D, with baked 2.5D lighting where useful.
Do not introduce 3D modeling or replace existing working resources without a
backup. Leave unrelated source changes and files alone.

Installed tools (recheck with `doctor` after updates):

| Tool | Version | Program | Automation |
| --- | --- | --- | --- |
| Krita | 5.3.4.1 | `C:\Program Files\Krita (x64)\bin\krita.exe` | existing `krita-codex-mcp==0.2.0` |
| Synfig | 1.4.5 | `C:\Program Files\Synfig\bin\synfig.exe` | SIF/SIFZ + CLI PNG render |
| Laigter | 1.14.0 | `C:\AI2D\Tools\Laigter\laigter.exe` | normal/specular/occlusion/parallax CLI |
| LMMS | 1.2.2 | `C:\Program Files\LMMS\lmms.exe` | MMP XML + offline render |
| Csound | 6.18 | `C:\Program Files\Csound6_x64\bin\csound.exe` | CSD + offline WAV render |
| Audacity | 3.7.9.0 | `C:\Program Files\Audacity\Audacity.exe` | named pipes, save/release, no native Close |

- Main command: `tools/media/Run-MediaPipeline.ps1 -Action <action> -NoAudio`.
  Python alternative: `tools/media/media_pipeline.py --no-audio <action>`.
  Use the interpreter listed above. Available actions: doctor, demo, maps,
  animate, music, sfx, process-audio, loop-music, import-pack, bind-audio,
  restore-audio and resume-integration. Production entry points require silence.
- Use actual PowerShell 7 (`pwsh`) for child scripts. In this environment the
  Windows PowerShell 5.1 executable returned 0 without producing results.
  Verify output files, decoded content and manifests, not only return codes.
- Krita MCP: prefer native tools when exposed. The tested
  `tools/media/krita_mcp_client.py` also connects to the existing version-pinned
  server using standard STDIO MCP, offline. It does not download another server.
  Bridge is authenticated loopback 127.0.0.1:17641, allowed root C:\AI2D.
  Call krita_state before edits and krita_preview after edits. Use explicit
  document IDs and layer UUIDs. Keep all secrets in the existing private config.
  Qt child processes use the Windows platform, not offscreen.
- Graphics masters: C:\AI2D\sprites (.kra and RGBA PNG); animation masters:
  C:\AI2D\animations (.sif and linked PNG). Create unique names. Use Krita
  layers for body/equipment/details; export parts for cutout rigs. The sample
  `create_krita_sprite.py` exercises actual native draw, layer export and KRA save.
- Existing Synfig renderer: C:\AI2D\animations\Render-Synfig.ps1. Default
  production test uses two threads, 60 frames at 60 Hz; it checks every PNG.
  Use `asset_tools.py` for baked lighting and atlas packing. Atlas limit is
  2048 per axis; RGBA8 memory is recorded. Keep fixed cells, padding, foot
  anchors and consistent silhouettes; evaluate whole-body movement and attacks.
- Laigter wrapper: C:\AI2D\Tools\Generate-LaigterMaps.ps1. Inputs from sprites,
  fresh output folders in maps. Current game has no sprite material-map shader:
  bake normal/specular/occlusion lighting into PNG; retain parallax as source.
- LMMS: C:\AI2D\audio\music\lmms_music.py / LMMS-Music.ps1, based on
  szablon.mmp in that folder. Native TripleOscillator/Kicker need no external
  samples. Edit score JSON/MMP; export via render and isolated AudioDummy config.
- Csound: C:\AI2D\audio\sfx\csound_generate.py / Csound-Audio.ps1. Keep CSD
  sources, parameters and seeded variants. Use file output and null real-time
  audio/MIDI modules. Never let CsOptions redirect a test to speakers.
- Audio masters: stereo PCM16 48 kHz WAV. Music delivery: Vorbis OGG quality 5.
  Use the installed Krita FFmpeg for loudnorm and the Synfig FFmpeg for libvorbis;
  check encoder availability. Record LUFS, true peak, samples and loop seam.
  Example ambient target -18 LUFS; SFX peak -2 dBFS. Adjust to the actual mix.
- Reusable import is in `tools/media/game_import.py`. Pack manifests include
  source, relative path, role, SHA-256 and provenance. Runtime exports go to
  `assets/media/<id>`. Native masters stay outside the player package.
  `runtime_atlas.hpp` provides a preloaded DrawTexturePro clip adapter. Existing
  procedural actors require explicit renderer integration for a commissioned
  asset; copying PNG alone does not replace them.
- Existing engine audio maps 22 cues to assets/audio/<name>.wav and streams
  assets/audio/echoes-of-the-ruins.ogg. `bind-audio` maps these exact IDs, validates
  files, backs up targets in art/pipeline/backups and atomically replaces them.
  `restore-audio` verifies hashes before restoration. Demo tests this against
  an isolated fixture; it preserves production audio and the game executable.
- Keep KRA/SIF/MMP/CSD, receipts, logs, manifests and license/provenance details.
  Pipeline runs are in C:\AI2D\pipeline\runs. report.json means complete success;
  failure.json lists completed stages only. resume-integration reuses a produced
  and imported pack without reapplying effects. Never publish or upload the
  project without a separate user request.
- Target 60 fps on Ryzen 5 7530U / 16 GB / integrated Radeon: preload atlases,
  avoid generation/file I/O in frame loops, bound textures/voices and run heavy
  production stages sequentially. A 60 Hz export is not an in-game FPS benchmark.

# Narrative authoring — articy:draft X FREE

Standing user preference (30.10.2026): plan Emberfall's story, lore, characters,
quests and dialogues in **articy:draft X FREE**, operating in the background.
Use the native project as the main narrative workspace and read its current
content before proposing changes. Preserve established canon; distinguish
draft ideas from approved/implemented facts. Save actual native changes and
verify readback rather than treating chat text or a loose JSON as articy work.
Export and validate approved gameplay changes through the documented pipeline.
The current MCP covers dialogues/speakers. When a lore task needs other native
object types, extend the existing free MDK bridge appropriately; do not claim
those objects were saved through tools that cannot create them.

Read `narrative/articy/README.md` and `narrative/articy/QA.json` first. As of
2026-10-30 the real native integration works: plugin 1.0.1, all nine MCP tools
exercised, seven conversations / 41 lines / two speakers / 34 connections saved.
Structural Connection children are handled correctly. `RESUME.md` is historical;
do not repeat its failed import or interpret the old pause as current.

- Client 4.4.0.43891: `C:\Program Files\articy draft X\bin`. The installed
  `ArticyDraft.dll` can be invoked by `C:\Program Files\dotnet\dotnet.exe`;
  the legacy EXE bootstrap produced no window during setup.
- Free official client MDK 2.5.0, .NET10 x64. Source:
  `tools/narrative/Emberfall.DialogueBridge`. Build/install:
  `pwsh -NoProfile -File tools/narrative/Install-ArticyBridge.ps1`.
  Portable SDK10.0.403 is in `C:\AI2D\Tools\articy-dev\dotnet-sdk-10`.
- Start helper: `tools/narrative/Start-ArticyBridge.ps1`, public DebugManifest
  option and minimized launch, no forced closure/termination. **Keep articy in
  the background** (user request, 30.10.2026). After a fresh process, the user
  selects `Emberfall Dialogues` once; no supported single-user auto-open command
  was verified. Native OpenProject is a multi-user/server API, not a free SU
  workaround. A hidden watcher then minimizes the verified owned window.
  `articy_background.py` can minimize an already open worker without input or
  focus. Human enters activation key only when required; never capture/store it.
- Project MCP: `.codex/config.toml`, `emberfall_articy`; Python STDIO server
  `tools/narrative/articy_mcp.py`. Direct CLI `articy_cli.py` uses the same IPC.
  A Codex restart may be needed to expose registered MCP tools in a new chat.
- Start with `articy_status` (must return an actual queued native response).
  Heartbeat/requests/responses are local under
  `%LOCALAPPDATA%\Emberfall\ArticyBridge`; no network listener or paid API.
- Only mutate explicitly identified `Emberfall*` projects with matching GUID,
  `EF_` IDs and revision/expected-text checks. No blind retry after timeout;
  inspect `articy_job_status` and native project. JSON snapshots before writes
  supplement normal native project backups; partial failures need inspection.
- FREE has no time limit, but has 700 objects/project split into categories.
  Respect the native object counter and category limits. Do not bypass limits,
  use paid Articy API licenses, paid AI extensions or upload project content.
- `story_sync.py extract <new-dir>` snapshots current C++ texts;
  `import-plans <plans-dir> --output <new-json>` deliberately imports via native
  MDK only, skips identical existing content, and refuses native/source conflicts;
  `stage <native-export.json> <new-dir>` validates native data and generates
  review copies of `intro_story.hpp`/`tower_story.hpp`; `apply <native-export.json>
  <new-dir>` compiles a hidden/silent C++ probe, preserves BOM/newlines, backs up
  originals, checks hashes, and replaces changed sources only. Current native
  export reproduces both game headers byte for byte, so no game change was needed.
  Source snapshots must never be labelled native exports. Do not overwrite
  native edits with old plans. Preserve current lore, UTF-8 Polish, quest gates
  and gameplay. Current runtime is linear: branches/cycles are rejected.
- Native create/edit/CAS-conflict/restore/save/export, original-prologue reopen,
  duplicate prevention and exact source comparison passed. Apply future edits
  through the mapped export, then run relevant game checks silently/hidden.
  Six newly imported dialogues were not subjected to a second full client
  restart in the finishing session; do not claim that extra restart was tested.
- Repeat offline checks with the supplied Python runtime and
  `tools/narrative/test_articy_bridge.py` (32 passing checks). These include
  simulated transport and real C++ compilation; live evidence is separately in
  QA.json and exports/. Game logic passed 194309 checks, hidden and silent.
  Keep native masters in `C:\AI2D\narrative`.
- Native project: `C:\AI2D\narrative\EmberfallDialogues\ProjectInfo.aph`, name
  `Emberfall Dialogues`. Recheck GUID against the checkpoint before writes.
  Use the background native MCP rather than GUI automation for ordinary work.
  Maintenance-only CLI `shutdown_owned` saves and gracefully stops only the
  exact background-owned GUID/path; never use it on a manual user session.
  No automatic retry after native mutation errors/timeouts.
