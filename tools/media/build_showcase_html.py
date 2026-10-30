"""Build self-contained interactive Generative UI showcase with embedded base64 GIFs."""
import base64, json
from pathlib import Path

ARTIFACT_DIR = Path(r"C:\Users\Amadeusz\.gemini\antigravity\brain\7eed67d0-c66c-4472-8822-5a18cb706a2b")

ENEMIES_DATA = [
    {
        "id": "slag_crawler",
        "name": "Żużlowy pełzacz",
        "tag": "Pancernik kaldery",
        "hp": 138,
        "atk": 26,
        "def": 10,
        "speed": "1.9",
        "type": "Fizyczny / Wręcz",
        "gif_file": "slag_crawler_animated.gif",
        "desc": "Ciężka, opancerzona bestia o skorupie z porowatego pumeksu i bazaltowego żużlu. Między płytami pancerza pulsują strumienie płynnej magmy, zasilając potężne żuwaczki.",
        "design_highlights": [
            "Wielowarstwowy pancerz skorupowy z ciemnego bazaltu i pumeksu",
            "Pulsujące pomarańczowo-złote szczeliny lawowe wzdłuż grzbietu",
            "Przegubowe odnóża o pazurach rozgrzanych do białości",
            "Zaciskające się żuwaczki z mikro-erupcjami iskier"
        ],
        "anim_notes": "Ciężki, rytmiczny chód o 6 odnóżach, unoszenie pancerza przy oddechu, rozbłyski lawy."
    },
    {
        "id": "lava_salamander",
        "name": "Salamandra lawowa",
        "tag": "Ognisty gad",
        "hp": 120,
        "atk": 28,
        "def": 5,
        "speed": "2.9",
        "type": "Magiczny / Szybki wręcz",
        "gif_file": "lava_salamander_animated.gif",
        "desc": "Zwinny i drapieżny płaz wulkaniczny. Jego łuski chłodzą się na grzbiecie do twardego andezytu, podczas gdy brzuch i kryza płoną żywym płomieniem.",
        "design_highlights": [
            "Falująca, wężowata sylwetka z rozżarzonym brzuchem",
            "Płonąca kryza grzbietowa z dynamicznymi językami ognia",
            "Długi biczowaty ogon zakończony żarzącym się płomieniem",
            "Bursztynowo-rubinowe oczy z pionową źrenicą gada"
        ],
        "anim_notes": "Szybki, niski ślizg drapieżnika, falowanie ogona i pulsujące światło brzucha."
    },
    {
        "id": "fire_imp",
        "name": "Ognisty chochlik",
        "tag": "Demon żaru",
        "hp": 104,
        "atk": 27,
        "def": 3,
        "speed": "2.35",
        "type": "Magiczny / Dystansowy",
        "gif_file": "fire_imp_animated.gif",
        "desc": "Złośliwy chochlik zrodzony ze skondensowanych gazów siarkowych i płomieni. Lewituje nad ziemią na skórzastych skrzydłach, żonglując kulami niestabilnej lawy.",
        "design_highlights": [
            "Cieniste, rogato-demoniczne lico ze złośliwym, gorejącym uśmiechem",
            "Skórzaste skrzydła nietoperza z rozżarzonymi żyłkami błon",
            "Lewitująca w dłoni wielowarstwowa kula ognia z wirem iskier",
            "Długi ogon z grotem w kształcie żelaznego szpicu"
        ],
        "anim_notes": "Lewitujące kołysanie góra-dół, trzepot skrzydeł, rotująca ognista kula w dłoni."
    },
    {
        "id": "ash_wraith",
        "name": "Popielny upiór",
        "tag": "Duch pogorzeliska",
        "hp": 135,
        "atk": 30,
        "def": 4,
        "speed": "1.75",
        "type": "Magiczny / Dystansowy mag",
        "gif_file": "ash_wraith_animated.gif",
        "desc": "Zwiewny cień zaklęty w chmurę gorącego popiołu i dymu. Posiada niezwykle wysoką obronę magiczną. Nosi rzeźbioną obsydianową maskę z płonącymi eterycznymi ślepiami.",
        "design_highlights": [
            "Płynne, poszarpane szaty z popiołu rozpraszające się ku dołowi w czarny dym",
            "Ciosana obsydianowa maska o geometrycznych krawędziach",
            "Upiorne, błękitno-białe ogniki duszy spoglądające z głębi kaptura",
            "Orbitalne runy żaru krążące wokół kościstych dłoni"
        ],
        "anim_notes": "Eteryczny dryf, powolne falowanie dymnych smug, rotacja run wokół dłoni."
    },
    {
        "id": "obsidian_sentinel",
        "name": "Obsydianowy wojownik",
        "tag": "Strażnik kuźni",
        "hp": 205,
        "atk": 33,
        "def": 14,
        "speed": "1.65",
        "type": "Fizyczny / Ciężki piechur",
        "gif_file": "obsidian_sentinel_animated.gif",
        "desc": "Starożytny gwardzista Obsydianowych Kuźni. Jego pancerz z polerowanego szkła wulkanicznego odbija światło niczym lustro, a w spoinach płynie zasilająca go magma.",
        "design_highlights": [
            "Zwierciadlane fasety z czarnego obsydianu z fioletowo-metalicznym połyskiem",
            "Szczelina wizjera hełmu jarząca się oślepiającym żarem",
            "Monumentalny ząbkowany wielki miecz z wyrytą runą ognia",
            "Masywne, najeżone kolcami naramienniki i opancerzone nagolenniki"
        ],
        "anim_notes": "Ciężki, zdyscyplinowany krok bojowy, gotowość miecza, pulsowanie wizjera."
    },
    {
        "id": "ember_drake",
        "name": "Żarowy drak",
        "tag": "Skrzydlata bestia",
        "hp": 155,
        "atk": 31,
        "def": 7,
        "speed": "2.65",
        "type": "Magiczny / Szybki łowca",
        "gif_file": "ember_drake_animated.gif",
        "desc": "Agresywny gad skrzydlaty krążący nad kominami siarkowymi. Potrafi błyskawicznie zanurkować na ofiarę, smagając ją ognistym oddechem i kolczastym ogonem.",
        "design_highlights": [
            "Szerokie smocze skrzydła z siecią ognistych naczyń krwionośnych",
            "Rozżarzona klatka piersiowa z widocznym zarysem żeber niczym palenisko",
            "Rogata smocza głowa z rozchylonymi szczękami pełnymi sztyletowatych kłów",
            "Ciężki ogon ze sternikiem z ostrej skały wulkanicznej"
        ],
        "anim_notes": "Potężny wymach skrzydeł, drapieżna gotowość do skoku, buchające z paszczy iskry."
    },
    {
        "id": "varkhor",
        "name": "Varkhor, Serce Kaldery",
        "tag": "Władca wulkanu — Trzeci Boss",
        "hp": 940,
        "atk": 42,
        "def": 17,
        "speed": "1.3",
        "type": "Boss / Ciosy i erupcje",
        "gif_file": "varkhor_animated.gif",
        "desc": "Pierwotny tytan wulkanu, którego ciało tworzy żywa góra bazaltu. W jego piersi bije potężne Serce Świtu — źródło erupcji wstrząsających całą krainą.",
        "design_highlights": [
            "Płonący tygiel w klatce piersiowej («Serce Kaldery») pulsujący bielą i złotem",
            "Dymne kominy wulkaniczne na barkach wyrzucające gęste kłęby sadzy",
            "Korona z potrójnych bazaltowych iglic na głowie tytana",
            "Mocarne głazowe pięści z żarzącymi się kostkami niszczącymi skałę"
        ],
        "anim_notes": "Potężne sejsmiczne kołysanie, oddech wzdymający rdzeń lawy, unoszące się pióropusze dymu."
    }
]

# Convert all GIFs to base64
for e in ENEMIES_DATA:
    gif_p = ARTIFACT_DIR / e["gif_file"]
    b64 = base64.b64encode(gif_p.read_bytes()).decode("ascii")
    e["b64_gif"] = f"data:image/gif;base64,{b64}"

html_content = f"""<!DOCTYPE html>
<html lang="pl">
<head>
  <meta charset="utf-8">
  <title>Kaldera Popiołu — Nowe Projekty Przeciwników</title>
  <script src="https://www.gstatic.com/antigravity/web/dev/tailwindcss.min.js"></script>
</head>
<body class="bg-[var(--background)] text-[var(--foreground)] antialiased p-4 md:p-6 font-sans">
  <div class="max-w-6xl mx-auto space-y-6">
    
    <!-- Header -->
    <header class="border-b border-[var(--border)] pb-4 flex flex-col md:flex-row justify-between items-start md:items-center gap-4">
      <div>
        <div class="flex items-center gap-2">
          <span class="px-2 py-0.5 text-xs font-semibold uppercase tracking-wider rounded bg-amber-500/10 text-amber-500 border border-amber-500/20">Staging Concept</span>
          <span class="text-xs text-[var(--muted-foreground)]">7 z 7 ukończonych</span>
        </div>
        <h1 class="text-2xl font-bold tracking-tight mt-1 text-[var(--foreground)]">Kaldera Popiołu — Nowe Projekty i Animacje Przeciwników</h1>
        <p class="text-sm text-[var(--muted-foreground)] mt-0.5">Podgląd z ulepszonym cieniowaniem 2.5D, efektami żaru i autorskimi pętlami animacji.</p>
      </div>
      <div class="flex items-center gap-2 text-xs text-[var(--muted-foreground)] bg-[var(--card)] px-3 py-2 rounded-lg border border-[var(--border)]">
        <span class="inline-block w-2 h-2 rounded-full bg-emerald-500 animate-pulse"></span>
        <span>Zasoby bezpieczne: nie wdrożone do gry</span>
      </div>
    </header>

    <!-- Main Interactive Grid -->
    <div class="grid grid-cols-1 lg:grid-cols-12 gap-6">
      
      <!-- Left: Enemy Selector Tabs -->
      <div class="lg:col-span-4 space-y-2">
        <h2 class="text-xs font-semibold uppercase tracking-wider text-[var(--muted-foreground)] px-2">Wybierz Przeciwnika</h2>
        <div id="enemy-tabs" class="space-y-1.5">
          <!-- Injected via JS -->
        </div>
      </div>

      <!-- Right: Detailed Showcase Card -->
      <div class="lg:col-span-8">
        <div id="showcase-card" class="bg-[var(--card)] border border-[var(--border)] rounded-2xl p-6 shadow-sm flex flex-col md:flex-row gap-6 items-center md:items-start">
          
          <!-- Animation Preview Box -->
          <div class="w-full md:w-72 shrink-0 flex flex-col items-center">
            <div class="relative w-64 h-64 rounded-xl bg-gradient-to-b from-[#1a1520] to-[#0c0a10] border border-[var(--border)] flex items-center justify-center overflow-hidden shadow-inner group">
              <div class="absolute inset-0 bg-[radial-gradient(circle_at_50%_70%,rgba(245,100,20,0.18),transparent_70%)]"></div>
              <img id="active-gif" src="" alt="Animation" class="relative z-10 w-56 h-56 object-contain drop-shadow-[0_10px_20px_rgba(0,0,0,0.6)]">
              <div class="absolute bottom-2 right-2 px-2 py-0.5 text-[10px] rounded bg-black/60 text-amber-300 font-mono backdrop-blur-sm">
                16 klatek / 15 FPS
              </div>
            </div>
            <div class="w-full mt-3 flex justify-between text-xs text-[var(--muted-foreground)] px-1">
              <span>Rozdzielczość klatki: 256×256</span>
              <span class="text-amber-400">Pętla ciągła</span>
            </div>
          </div>

          <!-- Metadata & Anatomy -->
          <div class="flex-1 space-y-4 w-full">
            <div>
              <div class="flex items-center justify-between">
                <span id="active-tag" class="text-xs font-medium text-amber-500 uppercase tracking-wide"></span>
                <span id="active-type" class="text-xs px-2 py-0.5 rounded bg-[var(--accent)] text-[var(--foreground)] border border-[var(--border)]"></span>
              </div>
              <h3 id="active-name" class="text-2xl font-bold mt-1 text-[var(--foreground)]"></h3>
              <p id="active-desc" class="text-sm text-[var(--muted-foreground)] mt-2 leading-relaxed"></p>
            </div>

            <!-- Stats Bar -->
            <div class="grid grid-cols-4 gap-2 pt-2 border-t border-[var(--border)]">
              <div class="bg-[var(--background)] p-2.5 rounded-lg border border-[var(--border)] text-center">
                <div class="text-[11px] text-[var(--muted-foreground)]">Życie (HP)</div>
                <div id="active-hp" class="text-base font-bold text-red-400 mt-0.5"></div>
              </div>
              <div class="bg-[var(--background)] p-2.5 rounded-lg border border-[var(--border)] text-center">
                <div class="text-[11px] text-[var(--muted-foreground)]">Atak bazowy</div>
                <div id="active-atk" class="text-base font-bold text-amber-400 mt-0.5"></div>
              </div>
              <div class="bg-[var(--background)] p-2.5 rounded-lg border border-[var(--border)] text-center">
                <div class="text-[11px] text-[var(--muted-foreground)]">Pancerz</div>
                <div id="active-def" class="text-base font-bold text-sky-400 mt-0.5"></div>
              </div>
              <div class="bg-[var(--background)] p-2.5 rounded-lg border border-[var(--border)] text-center">
                <div class="text-[11px] text-[var(--muted-foreground)]">Prędkość</div>
                <div id="active-speed" class="text-base font-bold text-emerald-400 mt-0.5"></div>
              </div>
            </div>

            <!-- Design Highlights -->
            <div class="space-y-2 pt-2">
              <h4 class="text-xs font-semibold uppercase tracking-wider text-[var(--foreground)]">Wyróżniki Nowego Projektu</h4>
              <ul id="active-highlights" class="space-y-1 text-xs text-[var(--muted-foreground)]">
                <!-- Injected via JS -->
              </ul>
            </div>

            <!-- Animation Behavior Note -->
            <div class="p-3 rounded-lg bg-amber-500/5 border border-amber-500/15 text-xs text-amber-300/90 flex gap-2 items-start">
              <svg class="w-4 h-4 shrink-0 text-amber-500 mt-0.5" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 10V3L4 14h7v7l9-11h-7z" />
              </svg>
              <div>
                <span class="font-semibold text-amber-400">Dynamika animacji:</span>
                <span id="active-anim" class="ml-1 text-[var(--foreground)]"></span>
              </div>
            </div>

          </div>
        </div>
      </div>
    </div>

    <!-- Gallery of all 7 previews -->
    <div class="mt-8 pt-6 border-t border-[var(--border)]">
      <h2 class="text-base font-bold mb-4 text-[var(--foreground)]">Przegląd Całego Bestiariusza Kaldery</h2>
      <div class="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-7 gap-3">
        <!-- Injected thumbnails via JS -->
        <div id="thumb-grid" class="contents"></div>
      </div>
    </div>

  </div>

  <script>
    const enemies = {json.dumps(ENEMIES_DATA, ensure_ascii=False)};
    let activeIndex = 0;

    function renderTabs() {{
      const container = document.getElementById("enemy-tabs");
      container.innerHTML = "";
      enemies.forEach((e, idx) => {{
        const btn = document.createElement("button");
        const isActive = idx === activeIndex;
        btn.className = `w-full text-left p-3 rounded-xl border transition-all flex items-center justify-between ${{
          isActive 
            ? "bg-[var(--accent)] border-amber-500/50 shadow-sm text-[var(--foreground)] font-semibold"
            : "bg-[var(--card)] hover:bg-[var(--accent)] border-[var(--border)] text-[var(--muted-foreground)] hover:text-[var(--foreground)]"
        }}`;
        btn.onclick = () => selectEnemy(idx);
        btn.innerHTML = `
          <div>
            <div class="text-xs text-amber-500/80">${{e.tag}}</div>
            <div class="text-sm font-medium ${{isActive ? 'text-amber-400 font-bold' : ''}}">${{e.name}}</div>
          </div>
          <div class="text-xs font-mono text-[var(--muted-foreground)]">${{e.hp}} HP</div>
        `;
        container.appendChild(btn);
      }});
    }}

    function renderThumbs() {{
      const grid = document.getElementById("thumb-grid");
      grid.innerHTML = "";
      enemies.forEach((e, idx) => {{
        const card = document.createElement("div");
        const isActive = idx === activeIndex;
        card.className = `p-2 rounded-xl border cursor-pointer transition-all text-center flex flex-col items-center bg-[#15121c] ${{
          isActive ? "border-amber-500 ring-2 ring-amber-500/20" : "border-[var(--border)] hover:border-amber-500/50"
        }}`;
        card.onclick = () => selectEnemy(idx);
        card.innerHTML = `
          <img src="${{e.b64_gif}}" class="w-16 h-16 object-contain my-1">
          <div class="text-[11px] font-medium text-[var(--foreground)] truncate w-full mt-1">${{e.name}}</div>
          <div class="text-[10px] text-[var(--muted-foreground)]">${{e.hp}} HP</div>
        `;
        grid.appendChild(card);
      }});
    }}

    function selectEnemy(index) {{
      activeIndex = index;
      const e = enemies[index];
      
      document.getElementById("active-gif").src = e.b64_gif;
      document.getElementById("active-name").textContent = e.name;
      document.getElementById("active-tag").textContent = e.tag;
      document.getElementById("active-type").textContent = e.type;
      document.getElementById("active-desc").textContent = e.desc;
      document.getElementById("active-hp").textContent = e.hp;
      document.getElementById("active-atk").textContent = e.atk;
      document.getElementById("active-def").textContent = e.def;
      document.getElementById("active-speed").textContent = e.speed;
      document.getElementById("active-anim").textContent = e.anim_notes;

      const hlList = document.getElementById("active-highlights");
      hlList.innerHTML = "";
      e.design_highlights.forEach(h => {{
        const li = document.createElement("li");
        li.className = "flex items-center gap-1.5";
        li.innerHTML = `<span class="w-1.5 h-1.5 rounded-full bg-amber-500"></span> <span>${{h}}</span>`;
        hlList.appendChild(li);
      }});

      renderTabs();
      renderThumbs();
    }}

    // Init
    selectEnemy(0);
  </script>
</body>
</html>
"""

out_html = ARTIFACT_DIR / "caldera_enemies_showcase.html"
out_html.write_text(html_content, encoding="utf-8")
print(f"Saved interactive showcase to {out_html} ({len(html_content)} bytes)")
