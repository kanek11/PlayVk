#pragma once

#include <functional>
#include <vector>

#include "MMath.h"

#include "Gameplay/SceneComponent.h"

#include "Controller.h"

namespace Anim {

    template<typename T>
    inline T Lerp(const T& a, const T& b, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return a + (b - a) * t;
    }

    //inline DirectX::XMVECTOR Slerp(const DirectX::XMVECTOR& qa, const DirectX::XMVECTOR& qb, float t) {
    //    return DirectX::XMQuaternionSlerp(qa, qb, t);
    //} 

    // normalized T -> value
    using FCurve = std::function<float(float)>;

    using FDuring = std::function<void(float)>;
    using FThen = std::function<void()>;

    namespace Easing {
        inline float Linear(float t) { return t; }
        inline float QuadInOut(float t) { return (t < 0.5f) ? 2 * t * t : 1 - std::pow(-2 * t + 2, 2.0f) / 2; }

        inline float CubicInOut(float t) { return (t < 0.5f) ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3.0f) / 2; }


        inline float SmoothPulse(float t) {
            return std::sin(t * MMath::PI);
        }
    }


    struct Tween {
        std::function<void(float)> onApply;
        std::function<void()> onComplete;

        float elapsed = 0.f;
        //float delay = 0.f;
        float duration = 1.f;
        bool  finished = false;
        //bool  yoyo = false;
        //int   loops = 0; 

        //open to customm;
        FCurve ease = Easing::Linear;

        Tween& SetEase(const FCurve& e) { ease = e; return *this; }
        Tween& OnComplete(std::function<void()> cb) { onComplete = std::move(cb); return *this; }
    };

    // singleton;
    class TweenRunner {
    public:
        static TweenRunner& Get() { static TweenRunner inst; return inst; }

        Tween* Add(std::unique_ptr<Tween> tw) {
            auto ptr = tw.get();
            tweens.emplace_back(std::move(tw));
            return ptr;
        }

        void Update(float dt) {
            //std::cout << "tweens num: " << tweens.size() << '\n';
            std::vector<std::function<void()>> deferredComplete;

            for (auto& t : tweens) {
                if (!t || t->finished) continue;

                t->elapsed += dt;
                float nt = std::clamp(t->elapsed / std::max(1e-6f, t->duration), 0.f, 1.f);
                float et = t->ease ? t->ease(nt) : nt;
                if (t->onApply) t->onApply(et);

                if (t->elapsed >= t->duration) {
                    t->finished = true;
                    if (t->onComplete)
                        deferredComplete.push_back(t->onComplete);  
                }
            }

            // GC
            tweens.erase(std::remove_if(tweens.begin(), tweens.end(),
                [](const std::unique_ptr<Tween>& t) {
                    return t->finished;
                }), tweens.end());

 
            for (auto& fn : deferredComplete)
                fn();
        }

        void Clear() {
            tweens.clear();
        }

    private:
        std::vector<std::unique_ptr<Tween>> tweens;
    };



    inline Tween* WaitFor(
        float duration,
        std::function<void()> then = {})
    {
        auto tw = std::make_unique<Tween>();

        tw->duration = std::max(0.0f, duration);

        if (then) tw->OnComplete(std::move(then));

        return TweenRunner::Get().Add(std::move(tw));
    }



    inline Tween* MoveTo(Gameplay::USceneComponent* comp,
        const MMath::Float3& target,
        float duration,
        FCurve ease = Easing::CubicInOut)
    {
        if (!comp) return nullptr;
        MMath::Float3 start = comp->GetRelativePosition();
        auto tw = std::make_unique<Tween>();

        tw->duration = duration;
        tw->ease = std::move(ease);
        tw->onApply = [comp, start, target](float e) {
            auto v = Lerp(start, target, e);
            comp->SetRelativePosition(v);
            comp->UpdateWorldTransform();
            };

        return TweenRunner::Get().Add(std::move(tw));
    }

    //rotate to:
    inline Tween* RotateTo(Gameplay::USceneComponent* comp,
        const Quaternion& target,
        float duration,
        FCurve ease = Easing::CubicInOut)
    {
        if (!comp) return nullptr;
        auto start = comp->GetRelativeRotation();
        auto tw = std::make_unique<Tween>();
        tw->duration = duration;
        tw->ease = std::move(ease);
        tw->onApply = [comp, start, target](float e) {
            auto q = Slerp(start, target, e);
            comp->SetRelativeRotation(q);
            comp->UpdateWorldTransform();
            };
        return TweenRunner::Get().Add(std::move(tw));
    }

    // scale to:
    inline Tween* ScaleTo(Gameplay::USceneComponent* comp,
        const MMath::Float3& target,
        float duration,
        FCurve ease = Easing::CubicInOut)
    {
        if (!comp) return nullptr;
        MMath::Float3 start = comp->GetRelativeScale();
        auto tw = std::make_unique<Tween>();
        tw->duration = duration;
        tw->ease = std::move(ease);
        tw->onApply = [comp, start, target](float e) {
            auto v = Lerp(start, target, e);
            comp->SetRelativeScale(v);
            comp->UpdateWorldTransform();
            };
        return TweenRunner::Get().Add(std::move(tw));
    }

    
    inline void VibrateFor(float leftMotor, float rightMotor, float time) {
        VibrateController(0, leftMotor, rightMotor);

        Anim::WaitFor(time, []() {
            VibrateController(0, 0.0f, 0.0f);
            });
    }


}