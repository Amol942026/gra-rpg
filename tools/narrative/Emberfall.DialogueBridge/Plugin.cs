using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;
using System.Windows.Threading;
using System.Windows;
using Articy.Api;
using Articy.Api.Plugins;

namespace Emberfall.DialogueBridge;

// Runs in the normal articy client through its public, FREE-compatible MDK.
// It never changes license checks, object counters, account state or activation.
public sealed class Plugin : MacroPlugin
{
    private readonly string root = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "Emberfall", "ArticyBridge");
    private DispatcherTimer? timer;
    private bool processing;
    private DateTime lastHeartbeat;
    private static readonly JsonSerializerOptions JsonOptions = new() { WriteIndented = true };
    public override string DisplayName => "Emberfall dialogue bridge";
    public override string ContextName => DisplayName;

    public override string Initialize()
    {
        foreach (var folder in new[] { "requests", "processing", "responses", "backups" }) Directory.CreateDirectory(Path.Combine(root, folder));
        timer = new DispatcherTimer(DispatcherPriority.Background, Dispatcher.CurrentDispatcher) { Interval = TimeSpan.FromMilliseconds(200) };
        timer.Tick += Tick;
        timer.Start();
        WriteHeartbeat();
        return null!;
    }

    public override bool Deinitialize() { timer?.Stop(); timer = null; return true; }

    public override bool ProjectLoaded()
    {
        // Only this launcher's owned background session is minimized. No audio,
        // user preference, activation or unrelated project state is changed.
        if (Environment.GetEnvironmentVariable("EMBERFALL_ARTICY_BACKGROUND") == "1" && OwnedProject())
            Application.Current.MainWindow.WindowState = WindowState.Minimized;
        return base.ProjectLoaded();
    }

    private bool OwnedProject() => Session?.ProjectOpen == true &&
        Session.CurrentProjectGuid.ToString() == "cb99e03a-ae90-4443-b79f-7f8da1539c34" &&
        Path.GetFullPath(Session.GetProjectFolderPath()).TrimEnd('\\', '/').Equals(
            @"C:\AI2D\narrative\EmberfallDialogues", StringComparison.OrdinalIgnoreCase);

    private void Tick(object? sender, EventArgs args)
    {
        if (processing) return;
        processing = true;
        try
        {
            if ((DateTime.UtcNow - lastHeartbeat).TotalSeconds > 1) WriteHeartbeat();
            var request = Directory.EnumerateFiles(Path.Combine(root, "requests"), "*.json")
                .Where(p => Regex.IsMatch(Path.GetFileNameWithoutExtension(p), "^[a-f0-9]{32}$"))
                .Where(p => !File.Exists(Path.Combine(root, "responses", Path.GetFileName(p))))
                .OrderBy(File.GetCreationTimeUtc).FirstOrDefault();
            if (request is null) return;
            var id = Path.GetFileNameWithoutExtension(request);
            if (!Regex.IsMatch(id, "^[a-f0-9]{32}$")) return;
            var responsePath = Path.Combine(root, "responses", id + ".json");
            // Previously completed jobs are never applied again.
            if (File.Exists(responsePath)) return;
            var running = Path.Combine(root, "processing", id + ".json");
            File.Move(request, running);
            try
            {
                if (new FileInfo(running).Length > 1024 * 1024) throw new InvalidOperationException("Request exceeds 1 MiB");
                using var document = JsonDocument.Parse(File.ReadAllText(running));
                var data = document.RootElement;
                if (data.GetProperty("id").GetString() != id) throw new InvalidOperationException("Job ID mismatch");
                if (data.GetProperty("expires_utc").GetDateTime().ToUniversalTime() < DateTime.UtcNow) throw new InvalidOperationException("Expired job was not applied; inspect before issuing a new mutation");
                var op = data.GetProperty("operation").GetString()!;
                var parameters = data.GetProperty("parameters");
                var result = Execute(op, parameters, id);
                WriteJson(responsePath, new { id, ok = true, operation = op, completedUtc = DateTime.UtcNow, result });
            }
            catch (Exception error)
            {
                WriteJson(responsePath, new { id, ok = false, error = error.GetType().Name, message = error.Message, retry_mutation = false, inspect_project_before_retry = true });
            }
        }
        catch (Exception error)
        {
            WriteJson(Path.Combine(root, "bridge-error.json"), new { utc = DateTime.UtcNow, error = error.GetType().Name, message = error.Message });
        }
        finally { processing = false; }
    }

    private void WriteHeartbeat()
    {
        lastHeartbeat = DateTime.UtcNow;
        bool open = Session?.ProjectOpen == true;
        WriteJson(Path.Combine(root, "heartbeat.json"), new
        {
            bridge = "Emberfall.DialogueBridge", version = "1.0.1", utc = lastHeartbeat,
            processId = Environment.ProcessId, projectOpen = open,
            projectGuid = open ? Session!.CurrentProjectGuid.ToString() : null,
            projectName = open ? Session!.GetProjectRoot().GetDisplayName() : null,
            projectPath = open ? Session!.GetProjectFolderPath() : null,
            backgroundSession = Environment.GetEnvironmentVariable("EMBERFALL_ARTICY_BACKGROUND") == "1",
            minimized = Application.Current?.MainWindow?.WindowState == WindowState.Minimized,
            objectCounts = open ? new { dialogues = Session!.GetObjectsByType(ObjectType.Dialogue).Count,
                fragments = Session!.GetObjectsByType(ObjectType.DialogueFragment).Count,
                entities = Session!.GetObjectsByType(ObjectType.Entity).Count,
                connections = Session!.GetObjectsByType(ObjectType.Connection).Count } : null,
            transport = "local-file-ipc", articyApi = "public client MDK", paidApiUsed = false,
            licenseLimitsChanged = false, freePublishedMaximum = 700
        });
    }

    private object Execute(string op, JsonElement p, string jobId)
    {
        if (op == "status") { WriteHeartbeat(); using var state = JsonDocument.Parse(File.ReadAllText(Path.Combine(root, "heartbeat.json"))); return state.RootElement.Clone(); }
        if (Session?.ProjectOpen != true) throw new InvalidOperationException("No project is open. Activate the FREE license and open an Emberfall project first.");
        if (op == "list_dialogues") return Session.GetObjectsByType(ObjectType.Dialogue).Where(Owned).Select(ReadDialogue).ToArray();
        if (op == "read_dialogue") return ReadDialogue(GetOwned(Text(p, "dialogue"), ObjectType.Dialogue));
        if (op == "export_story") return new { schema = 1, producer = "articy-mdk-live", projectGuid = Session.CurrentProjectGuid, dialogues = Session.GetObjectsByType(ObjectType.Dialogue).Where(Owned).Select(ReadDialogue).ToArray() };
        if (op is not ("upsert_dialogue" or "set_line" or "connect" or "save" or "shutdown_owned")) throw new InvalidOperationException("Unsupported operation");
        if (Text(p, "project_guid") != Session.CurrentProjectGuid.ToString()) throw new InvalidOperationException("Active project changed; refusing mutation");
        if (!Session.GetProjectRoot().GetDisplayName().StartsWith("Emberfall", StringComparison.OrdinalIgnoreCase)) throw new InvalidOperationException("Only explicitly named Emberfall projects can be edited");
        WriteJson(Path.Combine(root, "backups", jobId + ".json"), new { utc = DateTime.UtcNow, projectGuid = Session.CurrentProjectGuid, dialogues = Session.GetObjectsByType(ObjectType.Dialogue).Where(Owned).Select(ReadDialogue).ToArray() });
        switch (op)
        {
            case "upsert_dialogue": return Upsert(p);
            case "set_line":
                var line = GetOwned(Text(p, "line"), ObjectType.DialogueFragment);
                CheckText(Text(p, "text"));
                if (line.GetText(TextFormat.Plain, null!) != Text(p, "expected_text")) throw new InvalidOperationException("Text changed in the editor; refusing conflicting overwrite");
                line.SetText(Text(p, "text"), TextFormat.Plain, null!); Session.SaveProject(); return ReadLine(line);
            case "connect":
                var from = GetOwned(Text(p, "from"), ObjectType.DialogueFragment); var to = GetOwned(Text(p, "to"), ObjectType.DialogueFragment);
                if (from.GetParent() != to.GetParent()) throw new InvalidOperationException("Nodes must be in the same dialogue");
                ConnectOnce(from, to); Session.SaveProject(); return new { connected = true };
            case "save": Session.SaveProject(); return new { saved = true };
            case "shutdown_owned":
                if (!OwnedProject() || Environment.GetEnvironmentVariable("EMBERFALL_ARTICY_BACKGROUND") != "1") throw new InvalidOperationException("Only the exact launcher-owned background project may be shut down");
                Session.SaveProject();
                // Tick writes the response before the dispatcher performs shutdown.
                Application.Current.Dispatcher.BeginInvoke(DispatcherPriority.Background, new Action(() => Application.Current.Shutdown()));
                return new { saved = true, shutdownScheduled = true };
            default: throw new InvalidOperationException("Unsupported operation");
        }
    }

    private object Upsert(JsonElement p)
    {
        var plan = p.GetProperty("dialogue"); var key = Tech(Text(plan, "id"));
        var rows = plan.GetProperty("lines").EnumerateArray().ToArray();
        if (rows.Length is < 1 or > 120) throw new InvalidOperationException("A dialogue must contain 1–120 real typed nodes");
        for (int i = 0; i < rows.Length; i++)
        {
            if (Text(rows[i], "speaker") is not ("Eldrin" or "Wanderer")) throw new InvalidOperationException("Unknown speaker");
            CheckText(Text(rows[i], "text"));
        }
        var existing = Session.GetObjectByTechName(key);
        if (existing is not null && (!Owned(existing) || existing.ObjectType != ObjectType.Dialogue)) throw new InvalidOperationException("Technical name collision");
        if (existing is not null && (!p.TryGetProperty("expected_revision", out var revision) || revision.GetString() != Revision(existing))) throw new InvalidOperationException("Read dialogue revision first; refusing a conflicting overwrite");
        if (existing is not null && existing.GetChildren().Count(o => o.ObjectType == ObjectType.DialogueFragment) > rows.Length) throw new InvalidOperationException("Shortening requires an explicit editor review; no automatic deletion");
        for (int i = 0; i < rows.Length; i++)
        {
            var node = Session.GetObjectByTechName(LineId(key, i));
            if (node is not null && (existing is null || node.ObjectType != ObjectType.DialogueFragment || node.GetParent() != existing)) throw new InvalidOperationException("Node identity collision (preflight)");
        }
        int missing = existing is null ? rows.Length : Enumerable.Range(0, rows.Length).Count(i => Session.GetObjectByTechName(LineId(key, i)) is null);
        Budget(missing, existing is null ? 1 : 0);
        var speakers = new Dictionary<string, ObjectProxy>();
        foreach (var speaker in new[] { "Eldrin", "Wanderer" })
        {
            string id = "EF_Speaker_" + speaker;var entity = Session.GetObjectByTechName(id);
            if (entity is null) { entity = Session.CreateEntity(Session.GetSystemFolder(SystemFolderNames.Entities), speaker == "Eldrin" ? "Eldrin" : "Wędrowiec"); entity.SetTechnicalName(id); }
            if (entity.ObjectType != ObjectType.Entity) throw new InvalidOperationException("Speaker name collision");
            speakers[speaker] = entity;
        }
        var dialogue = existing ?? Session.CreateDialogue(Session.GetSystemFolder(SystemFolderNames.Flow), Text(plan, "title"));
        dialogue.SetTechnicalName(key); dialogue.SetDisplayName(Text(plan, "title"));
        if (existing is null)
        {
            int index = Session.GetObjectsByType(ObjectType.Dialogue).Count - 1;
            dialogue.SetFlowPosition((index % 3) * 560, (index / 3) * 350);
        }
        ObjectProxy? previous = null;
        for (int i = 0; i < rows.Length; i++)
        {
            var id = LineId(key, i); var node = Session.GetObjectByTechName(id);
            if (node is not null && (node.ObjectType != ObjectType.DialogueFragment || node.GetParent() != dialogue)) throw new InvalidOperationException("Node identity collision");
            node ??= Session.CreateDialogueFragment(dialogue, Text(rows[i], "text"));
            node.SetTechnicalName(id); node.SetText(Text(rows[i], "text"), TextFormat.Plain, null!);node[ObjectPropertyNames.Speaker] = speakers[Text(rows[i], "speaker")];
            node.SetFlowPosition((i % 4) * 360, (i / 4) * 220);
            if (previous is not null) ConnectOnce(previous, node);previous = node;
        }
        Session.SaveProject();return ReadDialogue(dialogue);
    }

    private void Budget(int addedLines, int addedDialogues)
    {
        int lines = Session.GetObjectsByType(ObjectType.DialogueFragment).Count;
        int dialogues = Session.GetObjectsByType(ObjectType.Dialogue).Count;
        int entities = Session.GetObjectsByType(ObjectType.Entity).Count;
        // Conservative workflow budgets, intentionally below advertised FREE totals.
        // articy's own per-category licensing remains authoritative and untouched.
        if (lines + addedLines > 200 || dialogues + addedDialogues > 15 || entities + 2 > 50) throw new InvalidOperationException("Conservative FREE workflow budget reached; review the native object counter before adding content");
    }

    private void ConnectOnce(ObjectProxy from, ObjectProxy to)
    {
        var output = from.GetOutputPin(0);var input = to.GetInputPin(0);
        if (!output.GetOutgoingConnections(null!).Any(c => c[ObjectPropertyNames.TargetPin] is ObjectProxy pin && pin == input)) Session.ConnectPins(output, input);
    }
    private string Revision(ObjectProxy dialogue) => Convert.ToHexString(SHA256.HashData(Encoding.UTF8.GetBytes(JsonSerializer.Serialize(new { title = dialogue.GetDisplayName(), lines = dialogue.GetChildren().Where(o => o.ObjectType == ObjectType.DialogueFragment).OrderBy(o => o.GetTechnicalName()).Select(ReadLine).ToArray() })))).ToLowerInvariant();
    private object ReadDialogue(ObjectProxy dialogue)
    {
        CheckPlainFlow(dialogue);
        var children = dialogue.GetChildren();
        var unsupported = children.Where(o => o.ObjectType is not (ObjectType.DialogueFragment or ObjectType.Connection or ObjectType.Pin)).ToArray();
        if (unsupported.Length > 0) throw new InvalidOperationException("Unsupported native flow children: " + string.Join(", ", unsupported.Select(o => o.ObjectType.ToString())));
        foreach (var pin in children.Where(o => o.ObjectType == ObjectType.Pin)) CheckExpression(pin);
        var connections = children.Where(o => o.ObjectType == ObjectType.Connection).Select(c =>
        {
            CheckExpression(c);
            var source = (c[ObjectPropertyNames.SourcePin] as ObjectProxy)?.GetParent();
            var target = (c[ObjectPropertyNames.TargetPin] as ObjectProxy)?.GetParent();
            if (source is null || target is null) throw new InvalidOperationException("Unresolved native connection endpoint");
            return new { source = source.GetTechnicalName(), target = target.GetTechnicalName() };
        }).ToArray();
        return new { id = dialogue.GetTechnicalName(), title = dialogue.GetDisplayName(), objectId = dialogue.Id.ToString("x"), revision = Revision(dialogue),
            childTypes = children.GroupBy(o => o.ObjectType.ToString()).ToDictionary(g => g.Key, g => g.Count()), connections,
            lines = children.Where(o => o.ObjectType == ObjectType.DialogueFragment).OrderBy(o => o.GetTechnicalName()).Select(ReadLine).ToArray() };
    }
    private static void CheckPlainFlow(ObjectProxy node)
    {
        foreach (var obj in new[] { node }.Concat(node.GetInputPins()).Concat(node.GetOutputPins()))
            CheckExpression(obj);
    }
    private static void CheckExpression(ObjectProxy obj)
    {
        if (obj.GetAvailableProperties().Contains(ObjectPropertyNames.Expression) && !string.IsNullOrWhiteSpace(obj[ObjectPropertyNames.Expression]?.ToString())) throw new InvalidOperationException("Native script/condition needs an explicit game adapter; refusing to discard it");
    }
    private object ReadLine(ObjectProxy line)
    {
        var speaker = line[ObjectPropertyNames.Speaker] as ObjectProxy;
        CheckPlainFlow(line);
        var next = line.GetOutputPins().SelectMany(pin => pin.GetOutgoingConnections(null!)).Select(c => (c[ObjectPropertyNames.TargetPin] as ObjectProxy)?.GetParent()?.GetTechnicalName() ?? throw new InvalidOperationException("Unresolved native connection target")).ToArray();
        string speakerName = speaker?.GetTechnicalName() switch { "EF_Speaker_Eldrin" => "Eldrin", "EF_Speaker_Wanderer" => "Wanderer", _ => "Unknown" };
        return new { id = line.GetTechnicalName(), text = line.GetText(TextFormat.Plain, null!), speaker = speakerName, next };
    }
    private ObjectProxy GetOwned(string name, ObjectType type) { var o = Session.GetObjectByTechName(Tech(name));return o is not null && o.ObjectType == type && Owned(o) ? o : throw new InvalidOperationException("Owned object not found"); }
    private static bool Owned(ObjectProxy o) => o.HasTechnicalName && o.GetTechnicalName().StartsWith("EF_", StringComparison.Ordinal);
    private static string Tech(string value) => Regex.IsMatch(value, "^EF_[A-Za-z0-9_]{1,120}$") ? value : throw new InvalidOperationException("Expected a stable EF_ technical name");
    private static string LineId(string key, int i) => key + "_L" + i.ToString("D4");
    private static void CheckText(string value) { if (value.Length is < 1 or > 8000 || value.Any(c => char.IsControl(c) && c is not ('\n' or '\r' or '\t'))) throw new InvalidOperationException("Invalid line text or control character"); }
    private static string Text(JsonElement obj, string key) => obj.GetProperty(key).GetString() ?? throw new InvalidOperationException("Missing string: " + key);
    private static void WriteJson(string path, object value) { var tmp = path + ".tmp";File.WriteAllText(tmp, JsonSerializer.Serialize(value, JsonOptions));File.Move(tmp, path, true); }
}
