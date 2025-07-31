#include "PCH.h"

#include "Time.h"

#include <algorithm>


/*
* intended for fixed update:
* 

*/

namespace System {

    using namespace std::chrono;

    TimeSystem::TimeSystem() {
        m_lastRealTP = clock::now();
    }

    void TimeSystem::BeginFrame() {
        //real
        auto nowTP = clock::now();
        duration<double> realDt = nowTP - m_lastRealTP;
        m_lastRealTP = nowTP;

        m_info.realDelta = realDt.count();

        //engine
        double engDt = (m_info.paused ? 0.0 : m_info.realDelta * m_info.timeScale);
        if (m_pendingFrameSteps > 0) {      // paused step-by-frame
            engDt = m_policy.fixedDt;
            --m_pendingFrameSteps;
        }
        m_info.engineDelta = engDt;
        m_info.engineTime += engDt;

        double feed = (m_info.paused ? 0.0 : m_info.engineDelta);
        m_accumulator += feed;
    }

    void TimeSystem::EndFrame() {
        // variable-step update AFTER fixed-step
    /*    for (auto& e : m_frame) {
            e.t->TickFrame(m_info.engineDelta);
        }*/
    }

    void TimeSystem::PumpFixedSteps() {
        //sortRegistriesIfNeeded();

        int steps = 0;
        //resolve pending state
        if (m_pendingFixedSteps > 0) {
            steps = std::min(m_pendingFixedSteps, m_policy.maxCatchupSteps);
            m_pendingFixedSteps -= steps;
        }
        else
        {
            while (m_accumulator + 1e-9 >= m_policy.fixedDt && steps < m_policy.maxCatchupSteps) {
                m_accumulator -= m_policy.fixedDt;
                ++steps;
            }
        }

        for (int i = 0; i < steps; ++i) {
            for (auto& entry : m_fixedCB) {
                // e.t->TickFixed(m_policy.fixedDt);
                entry((float)m_policy.fixedDt);
            }

            //std::cout << "pump sim step" << '\n';
            m_info.simTime += m_policy.fixedDt;
        }
    }


    void TimeSystem::SetPaused(bool p) { m_info.paused = p; }
    void TimeSystem::TogglePaused() { m_info.paused = !m_info.paused; }
    void TimeSystem::SetTimeScale(double s) { m_info.timeScale = std::max(0.0, s); }
    void TimeSystem::SetFixedStepPolicy(const FixedStepPolicy& p) { m_policy = p; }

    void TimeSystem::RegisterFixedFrame(const EntryFixed& fixed)
    {
         m_fixedCB.push_back(fixed);
    }

    double TimeSystem::GetFixedAlpha() const
    {
        return std::clamp(m_accumulator / m_policy.fixedDt, 0.0, 1.0);
    }

    void TimeSystem::AdvanceFrames(int frames) { if (m_info.paused) m_pendingFrameSteps += std::max(0, frames); }
    void TimeSystem::AdvanceFixedSteps(int st) { if (m_info.paused) m_pendingFixedSteps += std::max(0, st); }


        //void TimeSystem::RegisterFixed(ITickFixed* t, int order) {
    //    m_fixed.push_back({ t, order });
    //    m_dirtyOrder = true;
    //}
    //void TimeSystem::RegisterFrame(ITickFrame* t, int order) {
    //    m_frame.push_back({ t, order });
    //    m_dirtyOrder = true;
    //}
    //void TimeSystem::UnregisterFixed(ITickFixed* t) {
    //    m_fixed.erase(std::remove_if(m_fixed.begin(), m_fixed.end(),
    //        [&](auto& e) {return e.t == t;}), m_fixed.end());
    //}
    //void TimeSystem::UnregisterFrame(ITickFrame* t) {
    //    m_frame.erase(std::remove_if(m_frame.begin(), m_frame.end(),
    //        [&](auto& e) {return e.t == t;}), m_frame.end());
    //}

    //void TimeSystem::sortRegistriesIfNeeded() {
    //    if (!m_dirtyOrder) return;
    //    std::sort(m_fixed.begin(), m_fixed.end(), [](auto& a, auto& b) {return a.order < b.order;});
    //    std::sort(m_frame.begin(), m_frame.end(), [](auto& a, auto& b) {return a.order < b.order;});
    //    m_dirtyOrder = false;
    //}


}