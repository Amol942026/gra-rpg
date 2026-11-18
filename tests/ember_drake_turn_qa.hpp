#pragma once
#include <cstring>

// Included by the silent, hidden art probe. Both builds use the same camera,
// pose and angle path; changes in gait phase cannot masquerade as a smoother turn.
inline bool runCalderaTurnQA(Enemy& creature, bool baseline) {
    const int type = creature.type;
    const bool crawler = type == VolcanoFirstType;
    constexpr int width = 256, height = 214;
    constexpr Vector2 anchor{128, 160};
    fs::create_directories("turn-walk");
    RenderTexture2D target = render_targets::colorOnly(width, height);
    world_shadows::init();
    const auto started = GetTime();
    std::ofstream metrics("turn-samples.csv");
    metrics << "pose,degree,changed,pixel_delta,width,height,visible_pixels\n";
    int frames = 0, checks = 0, failures = 0;
    std::ofstream checksLog("turn-checks.txt");
    auto check = [&](bool ok, const char* what) {
        ++checks;
        failures += !ok;
        checksLog << (ok ? "PASS " : "FAIL ") << what << '\n';
    };
    const auto displacement = [](float radians, float screenPixels) {
        const float x = std::cos(radians) * screenPixels / 40.f;
        const float y = std::sin(radians) * screenPixels / 20.f;
        return V{(y + x) * .5f, (y - x) * .5f};
    };
#if __has_include("../src/ember_drake_turn.hpp")
    if (!baseline && !crawler) check(volcano_actors::drake_turn::filtered(), "continuous direction shader linked on this GPU");
#endif
#if __has_include("../src/volcano_turn.hpp")
    if (!baseline && crawler) check(volcano_actors::turn_art::crawler.ready() && volcano_actors::turn_art::filtered(),
                                    "crawler directional sheets and shared shader are active");
#endif
    if (crawler) check(volcano_actors::bodyScale(type - VolcanoFirstType) == 1.f, "crawler retains its gameplay render scale");
    auto render = [&](bool shadow, Vector2 at, const std::string& capture = "", float fall = -1.f, Color outline = BLANK) {
        BeginTextureMode(target);
        ClearBackground(capture.rfind("turn-walk/", 0) == 0 ? rgb(49, 44, 47) : BLANK);
        if (capture.rfind("turn-walk/", 0) == 0) {
            DrawEllipse(int(at.x), int(at.y), 42, 9, rgb(42, 37, 39));
            DrawEllipse(int(at.x), int(at.y), 32, 6, rgb(37, 31, 34));
        }
        if (shadow) {
            BeginShaderMode(world_shadows::maskShader);
            world_shadows::beginMaskBlend();
            rlDisableBackfaceCulling(); // The ground projection reverses winding.
        }
        actor_art::shadowBuilding = shadow;
        if (fall < 0) actor_art::drawEnemy(creature, at.x, at.y, outline);
        else volcano_actors::death(creature, at.x, at.y, fall);
        actor_art::shadowBuilding = false;
        if (shadow) {
            EndBlendMode();
            EndShaderMode();
            rlEnableBackfaceCulling();
        }
        EndTextureMode();
        Image shot = LoadImageFromTexture(target.texture);
        ImageFlipVertical(&shot);
        if (!capture.empty()) check(ExportImage(shot, capture.c_str()), "captured renderer frame");
        const auto* data = static_cast<const Color*>(shot.data);
        std::vector<Color> pixels(data, data + width * height);
        UnloadImage(shot);
        ++frames;
        return pixels;
    };
    const auto spin = [&](int degree) {
        const float angle = degree * DEG2RAD;
        const auto movement = displacement(angle, 1);
        creature.animation.facingAngle = creature.animation.targetAngle = std::remainder(angle, 2*PI);
        creature.animation.facingInitialized = true;
        creature.animation.direction = directionFor(movement.x, movement.y, Direction::E);
    };
    creature.deathAction = creature.deathSpecial = creature.abilityAge = -1;
    creature.windup = creature.strike = 0;
    for (const std::string pose : {"idle", "walk", "reverse", "shadow", "bite"}) {
        creature.animation = {};
        creature.animation.motionInitialized = true;
        creature.animation.phase = .37f;
        creature.animation.moveBlend = pose == "walk" ? 1.f : 0.f;
        creature.animation.idleTime = .6f;
        creature.strike = pose == "bite" ? .18f : 0.f;
        std::vector<Color> first, previous;
        int distinctSteps = 0, pickCorrect = 0, pickSamples = 0;
        bool withinHoverBounds = true;
        for (int frame = 0; frame <= 360; ++frame) {
            const int degree = pose == "reverse" ? -frame : frame;
            spin(degree);
            char file[64]{};
            if (pose == "idle" && degree % 15 == 0 && degree < 360)
                std::snprintf(file, sizeof(file), "turn-idle-%03d.png", degree);
            auto current = render(pose == "shadow", anchor, file);
            unsigned long sum = 0;
            int visible = 0, united = 0, left = width, top = height, right = -1, bottom = -1;
            for (int i = 0; i < width * height; ++i) {
                const auto c = current[i];
                if (c.a > 50) {
                    ++visible;
                    left = std::min(left, i % width); right = std::max(right, i % width);
                    top = std::min(top, i / width); bottom = std::max(bottom, i / width);
                }
                if (previous.empty()) continue;
                const auto p = previous[i];
                if (p.a || c.a) ++united;
                sum += pose == "shadow" ? 3 * std::abs(int(p.a) - c.a) :
                    std::abs(int(p.r) - c.r) + std::abs(int(p.g) - c.g) + std::abs(int(p.b) - c.b);
            }
            if (!previous.empty()) distinctSteps += sum > 0;
            metrics << pose << ',' << degree << ',' << (sum > 0) << ',' << sum / (3. * std::max(1, united))
                    << ',' << right-left+1 << ',' << bottom-top+1 << ',' << visible << '\n';
            if (crawler && pose != "shadow")
                withinHoverBounds &= left >= anchor.x-75 && right < anchor.x+75 &&
                                     top >= anchor.y-enemyInfo(type).visualHeight && bottom < anchor.y+14;
            if (pose != "shadow" && frame % 15 == 0 && frame != 360) {
                for (int y = top; y <= bottom; y += 7) for (int x = left; x <= right; x += 7) {
                    const auto opacity = current[y*width+x].a;
                    if (opacity > 0 && opacity < 238) continue;
                    const Vector2 point{x+.5f, y+.5f};
                    bool picked = actor_art::drawEnemy(creature, anchor.x, anchor.y, BLANK, &point);
                    ++pickSamples;
                    pickCorrect += picked == (opacity >= 238);
                }
            }
            if (frame == 0) first = current;
            previous = std::move(current);
        }
        // At 360 degrees the float input can be a few ulps above zero.
        int differing = 0;
        for (size_t i = 0; i < first.size(); ++i)
            differing += std::abs(int(first[i].a) - previous[i].a) > 4;
        check(differing < 6, "full circle is seamless");
        check(distinctSteps >= (baseline ? 8 : 24), "many headings are distinguishable at a fixed gait phase");
        if (pickSamples) check(pickCorrect > pickSamples * .97f, "cursor agrees with the drawn opaque body and empty pixels");
        if (crawler && pose != "shadow") check(withinHoverBounds, "body, legs and spikes fit the live hover envelope");
    }

    if (crawler) {
        // Canonical atlas keys have nonuniform screen angles in the isometric projection.
        creature.animation = {};
        creature.animation.motionInitialized = creature.animation.facingInitialized = true;
        creature.strike = 0;
        for (int i = 0; i < 8; ++i) {
            creature.animation.direction = Direction(i);
            creature.animation.facingAngle = creature.animation.targetAngle = volcano_actors::facingAngles[i];
            const auto idle = render(false, anchor, "orientation-" + std::to_string(i) + ".png");
            if (!baseline) {
                const auto selected = render(false, anchor, "", -1, YELLOW);
                creature.hitDirection = {};
                creature.flash = EnemyHitRecovery;
                const auto flash = render(false, anchor);
                creature.flash = 0;
                int rim = 0, brighter = 0, stray = 0;
                for (size_t j = 0; j < idle.size(); ++j) {
                    rim += idle[j].a < 16 && selected[j].a > 30;
                    brighter += idle[j].a > 240 && flash[j].r > idle[j].r + 10;
                    stray += idle[j].a == 0 && flash[j].a > 10;
                }
                check(rim > 20, "selection outline traces the painted contour");
                check(brighter > 100 && stray == 0, "damage flash visibly lights only the body");
            }
            creature.animation.moveBlend = 1;
            creature.animation.phase = 0;
            const auto planted = render(false, anchor);
            creature.animation.phase = .31f;
            const auto step = render(false, anchor);
            check(std::memcmp(planted.data(), step.data(), step.size()*sizeof(Color)) != 0,
                  "stride moves visible limbs");
            creature.animation.moveBlend = creature.animation.phase = 0;
            creature.windup = .12f;
            const auto coil = render(false, anchor);
            creature.windup = 0; creature.strike = .16f;
            const auto reach = render(false, anchor, "attack-" + std::to_string(i) + ".png");
            check(std::memcmp(coil.data(), reach.data(), reach.size()*sizeof(Color)) != 0,
                  "melee wind-up and strike have visibly different poses");
            creature.strike = 0;
        }
    }
    if (!baseline) {
        const auto& breath = enemyAbilityProfile(type);
        const auto contact = enemy_death::groundContact(type);
        bool savedMismatch = false;
        creature.strike = creature.windup = 0;
        for (int degree = 4; degree < 360; degree += 10) {
            checksLog << "Intermediate angle " << degree << '\n';
            spin(degree);
            creature.abilityAge = crawler ? -1.f : breath.windup + breath.active * .35f;
            creature.strike = crawler ? .18f : 0.f;
            creature.abilityDirection = normalized(displacement(degree * DEG2RAD, 1));
            const auto live = render(false, anchor);
            if (!crawler) {
                const auto mouth = volcano_actors::drake_art::mouth(creature, anchor, game.volcanoSpecialPose(creature));
                bool jawNearby = false;
                for (int y = -6; y <= 6 && !jawNearby; ++y) for (int x = -6; x <= 6; ++x) {
                    if (x*x + y*y > 36) continue;
                    const int i = int(mouth.y+y)*width + int(mouth.x+x);
                    if (i >= 0 && i < width*height && live[i].a > 90) jawNearby = true;
                }
                check(jawNearby, "continuous view keeps the fire origin at the painted jaw");
            }
            creature.deathAction = creature.volcanoAction();
            creature.deathSpecial = game.volcanoSpecialPose(creature);
            creature.deathMotion = creature.animation;
            const auto start = render(false, anchor, "", 0);
            unsigned alphaDiff = 0, visible = 0;
            for (size_t i = 0; i < live.size(); ++i) {
                visible += live[i].a > 30 || start[i].a > 30;
                alphaDiff += std::abs(int(live[i].a) - start[i].a);
            }
            check(visible > 1600 && alphaDiff < visible * 2, "death begins in the captured intermediate orientation and pose");
            const auto landed = render(false, anchor, "", contact);
            const auto rested = render(false, anchor, "", 1);
            if (crawler) {
                unsigned eligible = 0, agreement = 0;
                for (int y = 65; y < 190; y += 4) for (int x = 52; x < 204; x += 4) {
                    const int alpha = rested[y*width+x].a;
                    if (alpha > 0 && alpha < 238) continue;
                    const Vector2 point{x+.5f, y+.5f};
                    const bool picked = volcano_actors::rest(creature, anchor.x, anchor.y, BLANK, &point);
                    ++eligible;
                    agreement += picked == (alpha >= 238);
                }
                check(eligible && agreement >= .99f*eligible, "loot picking follows the fallen silhouette");
            }
            const bool stable = std::memcmp(landed.data(), rested.data(), landed.size()*sizeof(Color)) == 0;
            if (!stable && !savedMismatch) {
                auto snapshot = [&](const std::vector<Color>& pixels, const char* name) {
                    Image sample{const_cast<Color*>(pixels.data()), width, height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
                    ExportImage(sample, name);
                };
                snapshot(landed, "first-corpse-mismatch-a.png");
                snapshot(rested, "first-corpse-mismatch-b.png");
                savedMismatch = true;
            }
            check(stable,
                  "intermediate-angle corpse holds after ground contact");
        }
        creature.abilityAge = -1;
    }

    // Walk a closed curve. The camera follows most of the displacement,
    // keeping the creature readable at its normal game size.
    creature.animation = {};
    creature.strike = 0;
    const auto startPos = creature.pos;
    constexpr int cycle = 132;
    constexpr float dt = 1.f / 60;
    const float travel = 6.f * VolcanoStrides[type - VolcanoFirstType] / cycle;
    float maxSteerStep = 0;
    for (int f = -cycle; f < cycle; ++f) {
        const float a = 2 * PI * f / cycle;
        const V move = displacement(a, travel);
        const float before = creature.animation.facingAngle;
        Game::advanceVolcanoAnimation(creature, move, dt, true);
        creature.pos = creature.pos + move;
        if (f < 0) continue;
        maxSteerStep = std::max(maxSteerStep, std::abs(std::remainder(creature.animation.facingAngle - before, 2 * PI)));
        if (f % 2) continue;
        char file[64];
        std::snprintf(file, sizeof(file), "turn-walk/%03d.png", f/2);
        const V delta = creature.pos - startPos;
        render(false, {anchor.x + (delta.x-delta.y)*16, anchor.y + (delta.x+delta.y)*8 - 22}, file);
    }
    creature.pos = startPos;
    creature.animation = {};
    Game::advanceVolcanoAnimation(creature, {1,-1}, dt, true);
    float initial = creature.animation.facingAngle;
    for (int f = 0; f < 64; ++f) {
        const float before = creature.animation.facingAngle;
        Game::advanceVolcanoAnimation(creature, {-.02f,.02f}, dt, true);
        maxSteerStep = std::max(maxSteerStep, std::abs(std::remainder(creature.animation.facingAngle-before, 2*PI)));
    }
    check(std::abs(std::remainder(creature.animation.facingAngle-initial,2*PI)) > 3.12f, "abrupt reversal completes a half turn");
    if (!baseline) check(maxSteerStep*RAD2DEG < 8.f, "game steering limits the largest single-frame turn at 60 Hz");
    check(IsWindowHidden() && !IsAudioDeviceReady(), "all frames were hidden and silent");
    constexpr int batchFrames = 120;
    RenderTexture2D batch = render_targets::colorOnly(480, 360);
    auto drawBatch = [&](int f) {
        BeginTextureMode(batch);
        ClearBackground(BLANK);
        for (int i = 0; i < 4; ++i) {
            spin((f * 3 + i * 73) % 360);
            creature.animation.moveBlend = 1;
            creature.animation.phase = std::fmod((f + i*17) / 31.f, 1.f);
            actor_art::drawEnemy(creature, 120.f + i%2*240, 147.f + i/2*180);
        }
        EndTextureMode();
    };
    for (int f = 0; f < 8; ++f) drawBatch(f);
    UnloadImage(LoadImageFromTexture(batch.texture));
    const double batchStart = GetTime();
    for (int f = 0; f < batchFrames; ++f) drawBatch(f);
    Image last = LoadImageFromTexture(batch.texture);
    const double batchSeconds = GetTime() - batchStart;
    unsigned occupied = 0;
    if (last.data) {
        const auto* pixels = static_cast<const Color*>(last.data);
        for (int i = 0; i < last.width*last.height; ++i) occupied += pixels[i].a>30;
    }
    check(occupied > 4*1500, "four rendered actors present at the end of the timed burst");
    UnloadImage(last);
    UnloadRenderTexture(batch);
    UnloadRenderTexture(target);
    world_shadows::unload();
    std::ofstream summary("turn-summary.json");
    summary << "{\"checks\":" << checks << ",\"failures\":" << failures << ",\"rendered_frames\":" << frames
            << ",\"simulation_max_step_degrees\":" << maxSteerStep * RAD2DEG
            << ",\"four_" << (crawler ? "crawler" : "drake") << "_batch_frames\":" << batchFrames
            << ",\"four_" << (crawler ? "crawler" : "drake") << "_batch_seconds\":" << batchSeconds
            << ",\"render_readback_export_and_check_seconds\":" << GetTime()-started << "}\n";
    return !failures;
}
