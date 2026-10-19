# Figma connection diagnostic — 2026-10-16

The user requires all new game artwork and animation assets to be made in Figma.
The user explicitly rejected generating replacement artwork locally.

## Verified state

- `figma@openai-curated-remote` is installed and enabled, version 16.0.0.
- Re-registering it with `codex plugin add figma@openai-curated-remote --json` succeeded.
- The current chat still exposes no callable Figma tools after that operation.
- The separately configured `figma` MCP server points to `https://mcp.figma.com/mcp`.
- `codex mcp login figma` fails with `OAuth metadata discovery failed ... error decoding response body`.
- The discovery response at `https://mcp.figma.com/.well-known/oauth-protected-resource/mcp`
  advertises chunked encoding but returns a JSON object where a chunk-length line is expected.
  curl independently reports `chunk hex-length char not a hex digit: 0x7b`.
- The root protected-resource metadata and authorization-server metadata can be read normally.
- Figma browser login succeeds, but the in-app browser cannot open the editor because WebGL
  is unsupported. Browser editing was abandoned; no artwork was generated outside Figma.
- The supported local app-server proxy is unavailable on this desktop instance. A separate
  read-only diagnostic app-list request timed out; no new agent or chat was created.

## Next check

Fully quit and relaunch Codex, return to this existing chat, and check the tool catalog.
If Figma tools appear, run `whoami` and read the existing design before making changes.
Existing design observed in the signed-in file browser:
`https://www.figma.com/design/aZi7WQlVPBceS0sQMBgqQB/`
(`Emberfall — Fantasy Art v48`). Its contents have not been inspected.
Reinstallation alone is not proof that tool execution works.

## Game work preserved

- Four persistent treasure chests: two forest and two desert.
- Three distinct random equipment items per chest, common or rare only, 75/25 rarity rolls.
- Loot, collision, realm persistence, drag-and-drop and interaction code implemented.
- Logic tests passed: 68,339 gameplay/animation checks. Player UI tests passed: 43/43.
- Chest artwork still consists of temporary text markers using existing UI primitives.
- Prologue artwork and animation changes remain pending Figma access.
- Developer EXE rebuilt. The attempted fullscreen benchmark failed during window creation;
  existing benchmark reports are stale and must not be reported as current performance.
- Player ZIP has not yet been rebuilt for these unfinished changes.
- All initiated tests and game launches passed `--no-audio`.
