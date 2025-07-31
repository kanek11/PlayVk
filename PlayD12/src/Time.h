#pragma once
#include <chrono>
#include <vector>
#include <atomic>
#include <cstdint>
#include <string>
#include <functional>
#include <optional>

struct FrameTimeInfo {
    double realDelta;    // Real/WALL delta (seconds)
    double engineDelta;  // after timeScale & pause
    double engineTime;   // accumulated engine time
    double simTime;      // accumulated fixed-sim time
    bool   paused;
    double timeScale;
};

struct FixedStepPolicy {
    double fixedDt = 1.0 / 60.0;  // simulation tick size
    int    maxCatchupSteps = 4;   // prevent spiral-of-death
};

struct ITickFixed {
    virtual ~ITickFixed() = default;
    virtual void TickFixed(double fixedDt) = 0;
};

struct ITickFrame {
    virtual ~ITickFrame() = default;
    virtual void TickFrame(double dt) = 0;
};

class TimeSystem {
public:
    TimeSystem();

    // frame boundaries
    void BeginFrame();
    void EndFrame();

    // per-frame pump (physics)
    void PumpFixedSteps();

    // registration
    void RegisterFixed(ITickFixed* t, int order = 0);
    void RegisterFrame(ITickFrame* t, int order = 0);
    void UnregisterFixed(ITickFixed* t);
    void UnregisterFrame(ITickFrame* t);

    // debug controls
    void SetPaused(bool p);
    void TogglePaused();
    void SetTimeScale(double s); // [0..]
    void StepFrames(int frames = 1);     // advance Engine/Frame N frames when paused
    void StepFixedSteps(int steps = 1);  // advance Sim N fixed-steps when paused

    // policy
    void SetFixedStepPolicy(const FixedStepPolicy& p);

    // query
    const FrameTimeInfo& GetTimeInfo() const { return m_info; }

private:
    using clock = std::chrono::steady_clock;

    // clocks
    clock::time_point m_lastRealTP;
    double m_accumulator = 0.0;

    // policy & state
    FixedStepPolicy m_policy;
    FrameTimeInfo   m_info;

    // stepping in pause mode
    int m_pendingFrameSteps = 0;
    int m_pendingFixedSteps = 0;

    // registries
    struct EntryF { ITickFixed* t; int order; };
    struct EntryV { ITickFrame* t; int order; };
    std::vector<EntryF> m_fixed;
    std::vector<EntryV> m_frame;

    // helpers
    void sortRegistriesIfNeeded();
    bool m_dirtyOrder = false;
};


#include "TimeSystem.h"
#include <algorithm>

TimeSystem::TimeSystem() {
    m_lastRealTP = clock::now();
    m_info = { 0,0,0,0,false,1.0 };
}

void TimeSystem::BeginFrame() {
    auto now = clock::now();
    std::chrono::duration<double> realDt = now - m_lastRealTP;
    m_lastRealTP = now;

    m_info.realDelta = realDt.count();

    double engDt = (m_info.paused ? 0.0 : m_info.realDelta * m_info.timeScale);
    if (m_pendingFrameSteps > 0) {      // paused step-by-frame
        engDt = m_policy.fixedDt;       // ?????“??? dt”????
        --m_pendingFrameSteps;
    }
    m_info.engineDelta = engDt;
    m_info.engineTime += engDt;

    // ?????????“engine delta”????? realDelta?
    double simFeed = (m_info.paused ? 0.0 : m_info.engineDelta);
    m_accumulator += simFeed;
}

void TimeSystem::PumpFixedSteps() {
    sortRegistriesIfNeeded();

    int steps = 0;
    // ?? pause ?“?? fixed-step”
    if (m_pendingFixedSteps > 0) {
        steps = std::min(m_pendingFixedSteps, m_policy.maxCatchupSteps);
        m_pendingFixedSteps -= steps;
    }
    else {
        while (m_accumulator + 1e-9 >= m_policy.fixedDt && steps < m_policy.maxCatchupSteps) {
            m_accumulator -= m_policy.fixedDt;
            ++steps;
        }
    }

    for (int i = 0; i < steps; ++i) {
        for (auto& e : m_fixed) {
            e.t->TickFixed(m_policy.fixedDt);
        }
        m_info.simTime += m_policy.fixedDt;
    }
}

void TimeSystem::EndFrame() {
    // variable-step update AFTER fixed-step
    for (auto& e : m_frame) {
        e.t->TickFrame(m_info.engineDelta);
    }
}

void TimeSystem::RegisterFixed(ITickFixed* t, int order) {
    m_fixed.push_back({ t, order });
    m_dirtyOrder = true;
}
void TimeSystem::RegisterFrame(ITickFrame* t, int order) {
    m_frame.push_back({ t, order });
    m_dirtyOrder = true;
}
void TimeSystem::UnregisterFixed(ITickFixed* t) {
    m_fixed.erase(std::remove_if(m_fixed.begin(), m_fixed.end(),
        [&](auto& e) {return e.t == t;}), m_fixed.end());
}
void TimeSystem::UnregisterFrame(ITickFrame* t) {
    m_frame.erase(std::remove_if(m_frame.begin(), m_frame.end(),
        [&](auto& e) {return e.t == t;}), m_frame.end());
}

void TimeSystem::sortRegistriesIfNeeded() {
    if (!m_dirtyOrder) return;
    std::sort(m_fixed.begin(), m_fixed.end(), [](auto& a, auto& b) {return a.order < b.order;});
    std::sort(m_frame.begin(), m_frame.end(), [](auto& a, auto& b) {return a.order < b.order;});
    m_dirtyOrder = false;
}

void TimeSystem::SetPaused(bool p) { m_info.paused = p; }
void TimeSystem::TogglePaused() { m_info.paused = !m_info.paused; }
void TimeSystem::SetTimeScale(double s) { m_info.timeScale = std::max(0.0, s); }
void TimeSystem::SetFixedStepPolicy(const FixedStepPolicy& p) { m_policy = p; }

void TimeSystem::StepFrames(int frames) { if (m_info.paused) m_pendingFrameSteps += std::max(0, frames); }
void TimeSystem::StepFixedSteps(int st) { if (m_info.paused) m_pendingFixedSteps += std::max(0, st); }


#pragma once
#include <chrono>
#include <string>
#include <vector>

class Profiler {
public:
    static void BeginFrame();
    static void EndFrame();
    static void BeginEvent(const char* name);
    static void EndEvent(); 
};

class ProfileScope {
public:
    explicit ProfileScope(const char* name) { Profiler::BeginEvent(name); }
    ~ProfileScope() { Profiler::EndEvent(); }
};
 
#define PROFILE_SCOPE(name) ProfileScope _prof_scope_##__LINE__(name)

