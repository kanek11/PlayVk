#pragma once
#include <chrono>
#include <vector>
#include <atomic>
#include <cstdint>
#include <string>
#include <functional>
#include <optional>

/*
* TP = timepoint


*/
////optional registery;  i don't like interface so i make it callback;
//struct ITickFixed {
//    virtual ~ITickFixed() = default;
//    virtual void TickFixed(double fixedDt) = 0;
//};
//
//struct ITickFrame {
//    virtual ~ITickFrame() = default;
//    virtual void TickFrame(double dt) = 0;
//};

// registration
//void RegisterFixed(ITickFixed* t, int order = 0);
//void RegisterFrame(ITickFrame* t, int order = 0);
//void UnregisterFixed(ITickFixed* t);
//void UnregisterFrame(ITickFrame* t);

// helpers
//void sortRegistriesIfNeeded();
//bool m_dirtyOrder = false;

namespace System {

    struct FrameTimeInfo {
        double realDelta;    // Real/WALL delta (seconds)
        double engineDelta;  // affected by timeScale, pause
        double engineTime;   // accumulated engine time
        double simTime;      // accumulated fixed-sim time
        bool   paused{ false };
        double timeScale{ 1.0 }; //engine scale
    };

    struct FixedStepPolicy {
        double fixedDt = 1.0 / 60.0;
        int    maxCatchupSteps = 4;   // prevent spiral-of-death
    };

    class TimeSystem {

        using EntryFixed = std::function<void(float)>;

    public:
        TimeSystem();

        // frame boundaries
        void BeginFrame();
        void EndFrame();

        // per-frame pump for physics
        void PumpFixedSteps();

        // controls
        void SetPaused(bool p);
        void TogglePaused();
        void SetTimeScale(double s); // [0..]

        void AdvanceFrames(int frames = 1);     // advance Engine/Frame N frames when paused
        void AdvanceFixedSteps(int steps = 1);  // advance Sim N fixed-steps when paused

        // policy
        void SetFixedStepPolicy(const FixedStepPolicy& p);

        // query
        const FrameTimeInfo& GetTimeInfo() const { return m_info; }

        void RegisterFixedFrame(const EntryFixed& fixed);

        double GetFixedAlpha() const;
    private:
        // policy & state
        FixedStepPolicy m_policy;

        // stepping in pause mode
        int m_pendingFixedSteps = 0;
        int m_pendingFrameSteps = 0;

        // registries
        //struct EntryF { ITickFixed* t; int order; };
        //struct EntryV { ITickFrame* t; int order; };
        std::vector<EntryFixed> m_fixedCB;
        //std::vector<EntryV> m_frameCB;

    private:
        using clock = std::chrono::steady_clock;

        // clocks
        clock::time_point m_lastRealTP;
        double m_accumulator = 0.0;

        FrameTimeInfo m_info{};
    };


}