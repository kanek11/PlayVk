#pragma once
#pragma once

#include <chrono>
#include <iostream>

using namespace std::chrono;

template <typename T>
concept ChronoDuration = std::derived_from<
    T, std::chrono::duration<typename T::rep, typename T::period>>;

template <ChronoDuration T = std::chrono::milliseconds>
class Timer {
 public:
  using unit = T;
  Timer() : m_start(steady_clock::now()), m_duration(T::zero()) {}

  ~Timer() {
    if (!m_stopped) stop();
  }

  void stop() {
    auto _end = steady_clock::now();
    m_duration = std::chrono::duration_cast<T>(_end - m_start);
    print<T>();
    m_stopped = true;
  }

  template <typename DurationType = T>
  double getElapsedTime() const {
    return static_cast<double>(
        std::chrono::duration_cast<DurationType>(m_duration).count());
  }

  template <typename DurationType = T>
  void print() const {
    double time = getElapsedTime<DurationType>();
    std::string_view suffix;

    if constexpr (std::is_same_v<DurationType, std::chrono::milliseconds>) {
      suffix = " ms";
    } else if constexpr (std::is_same_v<DurationType,
                                        std::chrono::microseconds>) {
      suffix = " ¦Ìs";
    } else if constexpr (std::is_same_v<DurationType, std::chrono::seconds>) {
      suffix = " s";
    } else if constexpr (std::is_same_v<DurationType,
                                        std::chrono::nanoseconds>) {
      suffix = " ns";
    } else {
      suffix = " (unknown unit)";
    }
    std::cout << "Elapsed time: " << time << suffix << '\n';
  }

 private:
  time_point<steady_clock> m_start;

  // duration<double> m_duration;
  bool m_stopped = false;
  T m_duration;
};
