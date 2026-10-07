#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

#ifdef GEODE_IS_ANDROID

#include <cocos2d.h>

static void setFPS() {
    auto fps = Mod::get()->getSettingValue<int64_t>("target-hz");

    if (fps < 60)
        fps = 60;

    auto interval = 1.0 / static_cast<double>(fps);

    cocos2d::CCApplication::sharedApplication()
        ->setAnimationInterval(interval);

    log::info("FPS Unlocker: animation interval set to {} FPS", fps);
}

#endif

$on_mod(Loaded) {

#ifdef GEODE_IS_ANDROID

    setFPS();

    listenForSettingChanges<int64_t>(
        "target-hz",
        [](int64_t) {
            setFPS();
        }
    );

#endif
}

class $modify(FPSUnlockerMenuLayer, MenuLayer) {

    bool init() {
        if (!MenuLayer::init())
            return false;

#ifdef GEODE_IS_ANDROID
        setFPS();
#endif

        return true;
    }
};
