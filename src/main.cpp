#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/MenuLayer.hpp>

#ifdef GEODE_IS_ANDROID
#include <jni.h>
#include <Geode/cocos/platform/android/jni/JniHelper.h>
#endif

#include <cmath>
#include <limits>

using namespace geode::prelude;

#ifdef GEODE_IS_ANDROID

namespace {

JNIEnv* getEnv() {
    auto vm = cocos2d::JniHelper::getJavaVM();

    if (!vm)
        return nullptr;

    JNIEnv* env = nullptr;

    if (vm->GetEnv(
        reinterpret_cast<void**>(&env),
        JNI_VERSION_1_6
    ) != JNI_OK) {
        return nullptr;
    }

    return env;
}

bool clearException(JNIEnv* env) {
    if (!env || !env->ExceptionCheck())
        return false;

    env->ExceptionClear();
    return true;
}

jobject getSurfaceView(JNIEnv* env) {
    auto viewClass = env->FindClass(
        "org/cocos2dx/lib/Cocos2dxGLSurfaceView"
    );

    if (!viewClass || clearException(env))
        return nullptr;

    auto companionField = env->GetStaticFieldID(
        viewClass,
        "Companion",
        "Lorg/cocos2dx/lib/Cocos2dxGLSurfaceView$Companion;"
    );

    if (!companionField || clearException(env)) {
        env->DeleteLocalRef(viewClass);
        return nullptr;
    }

    auto companion = env->GetStaticObjectField(
        viewClass,
        companionField
    );

    if (!companion || clearException(env)) {
        env->DeleteLocalRef(viewClass);
        return nullptr;
    }

    auto companionClass = env->GetObjectClass(companion);

    auto getter = env->GetMethodID(
        companionClass,
        "getCocos2dxGLSurfaceView",
        "()Lorg/cocos2dx/lib/Cocos2dxGLSurfaceView;"
    );

    if (!getter || clearException(env)) {
        env->DeleteLocalRef(companionClass);
        env->DeleteLocalRef(companion);
        env->DeleteLocalRef(viewClass);
        return nullptr;
    }

    auto view = env->CallObjectMethod(
        companion,
        getter
    );

    clearException(env);

    env->DeleteLocalRef(companionClass);
    env->DeleteLocalRef(companion);
    env->DeleteLocalRef(viewClass);

    return view;
}

bool setWindowRefreshRate(
    JNIEnv* env,
    jobject view,
    float hz,
    int modeID
) {
    auto viewClass = env->GetObjectClass(view);

    auto getContext = env->GetMethodID(
        viewClass,
        "getContext",
        "()Landroid/content/Context;"
    );

    if (!getContext || clearException(env)) {
        env->DeleteLocalRef(viewClass);
        return false;
    }

    auto context = env->CallObjectMethod(
        view,
        getContext
    );

    if (!context || clearException(env)) {
        env->DeleteLocalRef(viewClass);
        return false;
    }

    auto contextClass = env->GetObjectClass(context);

    auto getWindow = env->GetMethodID(
        contextClass,
        "getWindow",
        "()Landroid/view/Window;"
    );

    if (!getWindow || clearException(env)) {
        env->DeleteLocalRef(contextClass);
        env->DeleteLocalRef(context);
        env->DeleteLocalRef(viewClass);
        return false;
    }

    auto window = env->CallObjectMethod(
        context,
        getWindow
    );

    if (!window || clearException(env)) {
        env->DeleteLocalRef(contextClass);
        env->DeleteLocalRef(context);
        env->DeleteLocalRef(viewClass);
        return false;
    }

    auto windowClass = env->GetObjectClass(window);

    auto getAttributes = env->GetMethodID(
        windowClass,
        "getAttributes",
        "()Landroid/view/WindowManager$LayoutParams;"
    );

    auto setAttributes = env->GetMethodID(
        windowClass,
        "setAttributes",
        "(Landroid/view/WindowManager$LayoutParams;)V"
    );

    if (!getAttributes || !setAttributes ||
        clearException(env)) {

        env->DeleteLocalRef(windowClass);
        env->DeleteLocalRef(window);
        env->DeleteLocalRef(contextClass);
        env->DeleteLocalRef(context);
        env->DeleteLocalRef(viewClass);

        return false;
    }

    auto attributes = env->CallObjectMethod(
        window,
        getAttributes
    );

    if (!attributes || clearException(env)) {
        env->DeleteLocalRef(windowClass);
        env->DeleteLocalRef(window);
        env->DeleteLocalRef(contextClass);
        env->DeleteLocalRef(context);
        env->DeleteLocalRef(viewClass);

        return false;
    }

    auto attributesClass =
        env->GetObjectClass(attributes);

    auto refreshField = env->GetFieldID(
        attributesClass,
        "preferredRefreshRate",
        "F"
    );

    auto modeField = env->GetFieldID(
        attributesClass,
        "preferredDisplayModeId",
        "I"
    );

    if (refreshField) {
        env->SetFloatField(
            attributes,
            refreshField,
            hz
        );
    }

    if (modeField && modeID != 0) {
        env->SetIntField(
            attributes,
            modeField,
            modeID
        );
    }

    env->CallVoidMethod(
        window,
        setAttributes,
        attributes
    );

    bool success = !clearException(env);

    env->DeleteLocalRef(attributesClass);
    env->DeleteLocalRef(attributes);
    env->DeleteLocalRef(windowClass);
    env->DeleteLocalRef(window);
    env->DeleteLocalRef(contextClass);
    env->DeleteLocalRef(context);
    env->DeleteLocalRef(viewClass);

    return success;
}

bool setSurfaceRefreshRate(
    JNIEnv* env,
    jobject view,
    float hz
) {
    auto viewClass = env->GetObjectClass(view);

    auto getHolder = env->GetMethodID(
        viewClass,
        "getHolder",
        "()Landroid/view/SurfaceHolder;"
    );

    if (!getHolder || clearException(env)) {
        env->DeleteLocalRef(viewClass);
        return false;
    }

    auto holder = env->CallObjectMethod(
        view,
        getHolder
    );

    if (!holder || clearException(env)) {
        env->DeleteLocalRef(viewClass);
        return false;
    }

    auto holderClass = env->GetObjectClass(holder);

    auto getSurface = env->GetMethodID(
        holderClass,
        "getSurface",
        "()Landroid/view/Surface;"
    );

    if (!getSurface || clearException(env)) {
        env->DeleteLocalRef(holderClass);
        env->DeleteLocalRef(holder);
        env->DeleteLocalRef(viewClass);
        return false;
    }

    auto surface = env->CallObjectMethod(
        holder,
        getSurface
    );

    if (!surface || clearException(env)) {
        env->DeleteLocalRef(holderClass);
        env->DeleteLocalRef(holder);
        env->DeleteLocalRef(viewClass);
        return false;
    }

    auto surfaceClass =
        env->GetObjectClass(surface);

    bool success = false;

    auto setFrameRate3 = env->GetMethodID(
        surfaceClass,
        "setFrameRate",
        "(FII)V"
    );

    if (setFrameRate3 && !env->ExceptionCheck()) {

        constexpr jint DEFAULT_COMPATIBILITY = 0;
        constexpr jint CHANGE_ALWAYS = 1;

        env->CallVoidMethod(
            surface,
            setFrameRate3,
            static_cast<jfloat>(hz),
            DEFAULT_COMPATIBILITY,
            CHANGE_ALWAYS
        );

        success = !clearException(env);
    }
    else {
        clearException(env);

        auto setFrameRate2 = env->GetMethodID(
            surfaceClass,
            "setFrameRate",
            "(FI)V"
        );

        if (setFrameRate2 && !env->ExceptionCheck()) {

            constexpr jint DEFAULT_COMPATIBILITY = 0;

            env->CallVoidMethod(
                surface,
                setFrameRate2,
                static_cast<jfloat>(hz),
                DEFAULT_COMPATIBILITY
            );

            success = !clearException(env);
        }
        else {
            clearException(env);
        }
    }

    env->DeleteLocalRef(surfaceClass);
    env->DeleteLocalRef(surface);
    env->DeleteLocalRef(holderClass);
    env->DeleteLocalRef(holder);
    env->DeleteLocalRef(viewClass);

    return success;
}

void applyRefreshRate() {

    float hz = static_cast<float>(
        Mod::get()->getSettingValue<int64_t>("target-hz")
    );

    auto env = getEnv();

    if (!env)
        return;

    auto view = getSurfaceView(env);

    if (!view)
        return;

    int closestMode = 0;

    float closestDifference =
        std::numeric_limits<float>::max();

    auto viewClass = env->GetObjectClass(view);

    auto getDisplay = env->GetMethodID(
        viewClass,
        "getDisplay",
        "()Landroid/view/Display;"
    );

    if (getDisplay && !clearException(env)) {

        auto display = env->CallObjectMethod(
            view,
            getDisplay
        );

        if (display && !clearException(env)) {

            auto displayClass =
                env->GetObjectClass(display);

            auto getModes = env->GetMethodID(
                displayClass,
                "getSupportedModes",
                "()[Landroid/view/Display$Mode;"
            );

            if (getModes && !clearException(env)) {

                auto modes =
                    static_cast<jobjectArray>(
                        env->CallObjectMethod(
                            display,
                            getModes
                        )
                    );

                if (modes && !clearException(env)) {

                    auto modeClass =
                        env->FindClass(
                            "android/view/Display$Mode"
                        );

                    if (modeClass) {

                        auto getRate =
                            env->GetMethodID(
                                modeClass,
                                "getRefreshRate",
                                "()F"
                            );

                        auto getID =
                            env->GetMethodID(
                                modeClass,
                                "getModeId",
                                "()I"
                            );

                        if (getRate && getID) {

                            auto count =
                                env->GetArrayLength(modes);

                            for (jsize i = 0; i < count; i++) {

                                auto mode =
                                    env->GetObjectArrayElement(
                                        modes,
                                        i
                                    );

                                if (!mode)
                                    continue;

                                float rate =
                                    env->CallFloatMethod(
                                        mode,
                                        getRate
                                    );

                                int id =
                                    env->CallIntMethod(
                                        mode,
                                        getID
                                    );

                                float difference =
                                    std::fabs(rate - hz);

                                if (difference <
                                    closestDifference) {

                                    closestDifference =
                                        difference;

                                    closestMode = id;
                                }

                                env->DeleteLocalRef(mode);
                            }
                        }

                        env->DeleteLocalRef(modeClass);
                    }

                    env->DeleteLocalRef(modes);
                }
            }

            env->DeleteLocalRef(displayClass);
        }
    }

    clearException(env);

    setWindowRefreshRate(
        env,
        view,
        hz,
        closestMode
    );

    setSurfaceRefreshRate(
        env,
        view,
        hz
    );

    env->DeleteLocalRef(viewClass);
    env->DeleteLocalRef(view);
}

}

#endif

$on_mod(Loaded) {

#ifdef GEODE_IS_ANDROID

    applyRefreshRate();

    listenForSettingChanges<int64_t>(
        "target-hz",
        [](int64_t) {
            applyRefreshRate();
        }
    );

#endif
}

class $modify(FPSUnlockerMenuLayer, MenuLayer) {

    bool init() {

        if (!MenuLayer::init())
            return false;

#ifdef GEODE_IS_ANDROID
        applyRefreshRate();
#endif

        return true;
    }
};
