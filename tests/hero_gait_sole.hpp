#pragma once

namespace walk_checks {
using namespace ember::hero_motion;

inline float soleHeight(const Pose& p, int foot) {
    float lowest = 100;
    auto x = p.vector(foot, {1, 0, 0});
    auto y = p.vector(foot, {0, 1, 0});
    auto z = p.vector(foot, {0, 0, 1});
    // Three overlapping ellipsoids form the rendered boot, from heel to toe.
    for (auto shape : {std::array<float, 5>{3, -1, 5, 8.5f, 4},
                       std::array<float, 5>{1, 4, 4.5f, 5, 4.5f},
                       std::array<float, 5>{5, -3, 5, 7, 1}}) {
        Vec center = p.point(foot, restHead(foot) + Vec{0, shape[0], shape[1]});
        float radius = std::sqrt(std::pow(x.z * shape[2], 2) + std::pow(y.z * shape[3], 2)
                                 + std::pow(z.z * shape[4], 2));
        lowest = std::min(lowest, center.z - radius);
    }
    return lowest;
}
}
