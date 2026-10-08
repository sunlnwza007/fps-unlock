#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

class $modify(VeylixTPS, GJBaseGameLayer) {
    void update(float dt) {
        static float accumulator = 0.f;

        auto tps = Mod::get()->getSettingValue<int64_t>("target-hz");

        if (tps < 60)
            tps = 60;

        if (tps > 240)
            tps = 240;

        float step = 1.f / static_cast<float>(tps);

        accumulator += dt;

        int steps = 0;
        while (accumulator >= step && steps < 10) {
            GJBaseGameLayer::update(step);
            accumulator -= step;
            steps++;
        }
    }
};

$on_mod(Loaded) {
    log::info("Veylix TPS Unlocker loaded");
}
