# Safe pause — 2026-10-29

**Resolved on 2026-10-30 after the user's resume request.** This is a historical
checkpoint, not an instruction to repeat imports. The connection-child bug was
fixed in plugin 1.0.1; all seven conversations / 41 lines / 34 connections now
exist and export correctly. Current instructions/results are in README.md and
QA.json. Articy is operated minimized. After a new client process the user still
needs to select the project once; then native MCP does all authoring in the
background. Do not replay historical failed mutation jobs.

User explicitly requested a pause: "potem dokończysz". Resume only on request.

## Verified current state

- FREE activation verified in the real client. Native project created through
  the client UI in `C:\AI2D\narrative\EmberfallDialogues\ProjectInfo.aph`.
- Name `Emberfall Dialogues`, GUID `cb99e03a-ae90-4443-b79f-7f8da1539c34`.
- Local plugin loaded successfully (native log), then replied to a queued
  `status` request with the correct native project name/GUID.
  Evidence: `C:\AI2D\narrative\status-live-20261029.json`.
- The native FREE counter was inspected before import: Flow/documents 0/500,
  Entities 0/50, Assets 0/140, Locations 0/10. These are the allocations shown
  by this installed client. Native limits were not modified.
- Native `save` succeeded. Full blank-project ZIP backup was written and ZIP
  integrity checked: `C:\AI2D\narrative\blank-native-project-backup-20261029.zip`.
- First real `upsert_dialogue` job `75dc4970985443b59b3bb70a471f30a8`
  created the prologue and reached `Session.SaveProject()` successfully, then
  failed during `ReadDialogue()` with **Unsupported native flow child; extend
  the game adapter before exporting**. No automatic retry was issued.
  Native Flow partition contains the EF_IntroDialogue and line technical IDs.
  The saved project was not replaced/restored or deleted.
- Current UI is the owned project's Flow view; articy stays open. No further
  native write job is running. No production game header was changed.
- 28 offline checks pass, including the new complete-batch/stale-source guards.

## Next steps

1. Read current AGENTS/README/QA and this checkpoint. Recheck live status and
   current project GUID/path before any write. Do not replay the first import.
2. Fix native adapter handling of `GetChildren()`: native Connection objects
   are parented in their containing dialogue (confirmed official MDK property
   documentation). Current guard wrongly rejects these structural children,
   and the shortening check currently counts all children rather than only
   DialogueFragments. Inspect real child types; handle Connection/Pin objects
   separately, preserve actual connection export, and still reject unsupported
   narrative nodes/conditions. Do not blindly remove the guard.
3. Fix ReadDialogue line enumeration to include only DialogueFragments after
   validating structural children. Read-only typed diagnostics may be useful.
4. Save/restart the owned client safely to load the repaired plugin. Current
   import already saved before its response failed; mutation helpers currently
   take a ReadDialogue backup first and can hit the same bug even for `save`.
   Avoid repeated failing writes. Keep native project backups.
5. Read actual prologue, compare all 12 texts, speakers and 11 links with source
   plans. Resolve any mismatch deliberately. Then import the remaining plans,
   using revisions for existing content; verify seven dialogues / 41 lines.
6. Test native edit/CAS conflict, save, reopen and export. Verify no duplicate
   objects/connections. Check the native object counter again.
7. Stage validated C++ headers, compare exact content/gameplay behavior, and
   integrate only the reviewed result with backups and appropriate silent,
   hidden game checks. Update README/QA/AGENTS from evidence.

## UI notes

Concurrent desktop use caused wrong/occluded screenshots and helper focus
errors. The user granted a short exclusive interval and project creation then
completed. Articy's path control cannot be treated as a normal text field by
the helper: use its folder button and native folder tree. The native folder
selector worked with screenshot-relative clicks. WPF modal keyboard focus is
unreliable; always reobserve after errors, and inspect screenshot metadata as
tooltips can change screenshot-array ordering. Do not touch unrelated apps.

## Useful files / references

- Sources: `tools/narrative/Emberfall.DialogueBridge/Plugin.cs`
- Error response:
  `%LOCALAPPDATA%\Emberfall\ArticyBridge\responses\75dc4970985443b59b3bb70a471f30a8.json`
- Native logs: `%APPDATA%\Articy Software\articy\4.x\Logs\articy.log`
  (filter relevant plugin/error lines; do not dump license/configuration data).
- Official MDK Parent property documents Connection/Pin ownership:
  https://www.articy.com/adxdevkit/html/5c85dc7b-efc3-ef45-ae86-e5e1293057b6.htm

No license key, activation token or password is stored in this checkpoint.
