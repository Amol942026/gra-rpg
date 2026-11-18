#define EMBERFALL_PLAYER_BUILD 1
#define main emberfallApplicationMain
#include "../src/main.cpp"
#undef main
#include "enemy_ability_fixture.hpp"
#include "ember_drake_turn_qa.hpp"

// Render the same cutouts, special effect and death path used in an encounter.
int main(int argc, char** argv) {
    if ((argc != 3 && argc != 4) || std::string(argv[1]) != "--no-audio" ||
        std::string(argv[2]) != "--background-test") return 3;
    const bool turnQA = argc == 4 && std::string(argv[3]) == "--turn-qa";
    const bool baseline = argc == 4 && std::string(argv[3]) == "--turn-baseline";
    const bool crawlerQA = argc == 4 && std::string(argv[3]) == "--crawler-qa";
    const bool crawlerBaseline = argc == 4 && std::string(argv[3]) == "--crawler-baseline";
    if (argc == 4 && !turnQA && !baseline && !crawlerQA && !crawlerBaseline) return 3;
    fs::current_path(fs::absolute(argv[0]).parent_path());
    fs::create_directories("frames");
    SetTraceLogLevel(LOG_ERROR);
    SetConfigFlags(FLAG_WINDOW_HIDDEN | FLAG_WINDOW_ALWAYS_RUN);
    sync_present::useOpenGL();
    InitWindow(400, 240, "Emberfall drake QA");
    if (!IsWindowReady() || !IsWindowHidden() || IsAudioDeviceReady()) return 4;
    volcano_actors::load();
    caldera_attack_art::load();
    constexpr int type = EmberDrakeType;
    const auto& atlas = volcano_actors::atlases[type - VolcanoFirstType];
    if (!atlas.art.id || !atlas.authoredRest || !caldera_attack_art::atlas.id ||
        !drake_mouth::art.id || !drake_mouth::mask.id) return 5;
    game = enemyAbilityArena(type);
    auto& drake = game.enemies.front();
    const auto& breath = enemyAbilityProfile(type);
    const auto contact = enemy_death::groundContact(type);
    auto target = render_targets::colorOnly(640, 392);
    constexpr Vector2 anchor{320, 215};
    int checks = 0, failures = 0, frames = 0;
    std::ofstream log("drake-checks.txt");
    auto check = [&](bool ok, const char* label) {
        ++checks;
        if (!ok) ++failures;
        log << (ok ? "PASS " : "FAIL ") << label << '\n';
    };
    enum class View { Live, Breath, Death };
    std::vector<unsigned char> exhalePixels;
    const auto started = GetTime();
    auto render = [&](View view, float fall = 0.f, const std::string& file = "") {
        BeginTextureMode(target);
        ClearBackground(rgb(53, 48, 53));
        DrawEllipse(int(anchor.x), int(anchor.y + 1), 35, 6, rgb(41, 35, 39));
        if (view == View::Death) {
            volcano_actors::death(drake, anchor.x, anchor.y, fall);
        } else {
            const auto drawFlame = [&] {
                const auto world = screen(drake.pos);
                rlPushMatrix();
                rlTranslatef(anchor.x - world.x, anchor.y - world.y, 0);
                enemy_ability_art::drakeBreath(drake);
                rlPopMatrix();
            };
            const bool rear = enemy_ability_art::breathBehind(drake);
            if (view == View::Breath && rear) drawFlame();
            actor_art::drawEnemy(drake, anchor.x, anchor.y);
            if (view == View::Breath && !rear) drawFlame();
        }
        EndTextureMode();
        Image shot = LoadImageFromTexture(target.texture);
        ImageFlipVertical(&shot);
        if (!file.empty()) {
            check(ExportImage(shot, ("frames/" + file + ".png").c_str()), "exported an actual game frame");
            ++frames;
        }
        const auto* pixels = static_cast<const unsigned char*>(shot.data);
        const int bytes = shot.width * shot.height * 4;
        if (view == View::Live && drake.abilityBusy())
            exhalePixels.assign(pixels, pixels + bytes);
        if (file.find("fire-direction-") == 0) {
            int left = shot.width, top = shot.height, right = -1, bottom = -1, lit = 0;
            for (int y = 0; y < shot.height; ++y) for (int x = 0; x < shot.width; ++x) {
                const int i = (y * shot.width + x) * 4;
                if (int(pixels[i]) - exhalePixels[i] < 45 || pixels[i + 1] < 65) continue;
                left = std::min(left, x); right = std::max(right, x);
                top = std::min(top, y); bottom = std::max(bottom, y); ++lit;
            }
            check(lit > 340 && right - left > 22 && bottom - top > 22,
                  "fire covers a visible area even on a slope facing away from the camera");
        }
        std::uint64_t hash = 1469598103934665603ull;
        for (int i = 0; i < bytes; ++i) hash = (hash ^ pixels[i]) * 1099511628211ull;
        UnloadImage(shot);
        return hash;
    };
    auto clearPose = [&]() {
        drake.animation = {};
        drake.animation.motionInitialized = true;
        drake.abilityAge = -1;
        drake.windup = drake.strike = 0;
    };
    for (int side = 0; side < 8; ++side) {
        clearPose();
        drake.animation.direction = Direction(side);
        const float heading = volcano_actors::facingAngles[side];
        const float f = std::cos(heading), s = 2 * std::sin(heading);
        drake.abilityDirection = normalized({f + s, s - f});
        const auto idle = render(View::Live, 0, "direction-" + std::to_string(side));
        drake.animation.idleTime = .64f;
        check(render(View::Live) != idle, "idle shows a breathing pose");
        drake.animation.moveBlend = 1;
        const auto planted = render(View::Live);
        drake.animation.phase = .31f;
        check(render(View::Live) != planted, "step advances talons and wings");
        drake.animation.phase = 1;
        check(render(View::Live) == planted, "stride wraps smoothly to the initial sample");
        drake.animation.moveBlend = 0;
        drake.windup = .12f;
        const auto coiled = render(View::Live);
        drake.windup = 0;
        drake.strike = .21f;
        check(render(View::Live) != coiled, "bite changes from wind-up to extension");
        drake.strike = 0;
        drake.abilityAge = breath.windup * .68f;
        const auto inhale = render(View::Live);
        drake.abilityAge = breath.windup + breath.active * .4f;
        const auto exhale = render(View::Live);
        check(exhale != inhale, "fire release changes the body's silhouette");
        check(render(View::Breath, 0, "fire-direction-" + std::to_string(side)) != exhale,
              "projected flame remains visible in the correct depth pass");
        const auto muzzle = volcano_actors::drake_art::mouth(drake, anchor, game.volcanoSpecialPose(drake));
        bool touchesMuzzle = false;
        for (int dy = -4; dy <= 4 && !touchesMuzzle; dy += 2) {
            for (int dx = -4; dx <= 4 && !touchesMuzzle; dx += 2) {
                const Vector2 point{muzzle.x + dx, muzzle.y + dy};
                touchesMuzzle = actor_art::drawEnemy(drake, anchor.x, anchor.y, BLANK, &point);
            }
        }
        check(touchesMuzzle, "breath originates at the animated sprite");
        drake.abilityAge = -1;
        drake.deathMotion = drake.animation;
        drake.deathAction = drake.deathSpecial = -1;
        check(render(View::Death, contact * .62f) != render(View::Death, contact),
              "fall is distinct from the grounded corpse");
        check(render(View::Death, contact) == render(View::Death, 1),
              "body holds its resting pose after ground contact");
    }
    clearPose();
    drake.animation.direction = Direction::SE;
    drake.abilityDirection = {1, 0};
    auto filename = [](const char* clip, int frame) {
        char suffix[16];
        std::snprintf(suffix, sizeof(suffix), "-%03d", frame);
        return std::string(clip) + suffix;
    };
    for (int f = 0; f < 24; ++f) {
        drake.animation.moveBlend = 1;
        drake.animation.phase = f / 24.f;
        render(View::Live, 0, filename("walk", f));
    }
    drake.animation.phase = drake.animation.moveBlend = 0;
    for (int f = 0; f < 16; ++f) {
        const float time = f / 24.f;
        drake.windup = std::max(0.f, .42f - time);
        drake.strike = time < .42f ? 0 : std::max(0.f, .64f - time);
        render(View::Live, 0, filename("bite", f));
    }
    drake.windup = drake.strike = 0;
    for (int f = 0; f <= int(std::ceil(breath.duration() * 24)); ++f) {
        drake.abilityAge = std::min(breath.duration(), f / 24.f);
        render(View::Breath, 0, filename("breath", f));
    }
    drake.abilityAge = -1;
    drake.deathMotion = drake.animation;
    drake.deathAction = drake.deathSpecial = -1;
    for (int f = 0; f < 25; ++f)
        render(View::Death, contact * std::min(1.f, f / 21.f), filename("fall", f));
    check(IsWindowHidden() && !IsAudioDeviceReady(), "no visible window or audio device");
    std::ofstream report("drake-summary.json");
    report << "{\"checks\":" << checks << ",\"failures\":" << failures << ",\"frames\":" << frames
           << ",\"directions\":8,\"render_readback_export_seconds\":" << GetTime() - started << "}\n";
    UnloadRenderTexture(target);
    if ((turnQA || baseline) && !runCalderaTurnQA(drake, baseline)) ++failures;
    if (crawlerBaseline || crawlerQA) {
        if (crawlerQA) {
            const auto folder = fs::current_path();
            fs::create_directories("drake-regression");
            fs::current_path(folder/"drake-regression");
            if (!runCalderaTurnQA(drake, false)) ++failures;
            fs::current_path(folder);
        }
        game = enemyAbilityArena(VolcanoFirstType);
        if (!runCalderaTurnQA(game.enemies.front(), crawlerBaseline)) ++failures;
    }
    caldera_attack_art::unload();
    volcano_actors::unload();
    CloseWindow();
    return failures ? 6 : 0;
}
