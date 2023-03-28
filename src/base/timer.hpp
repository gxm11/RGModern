// zlib License

// copyright (C) 2023 Xiaomi Guo and Krimiston

// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once
#include <time.h>

#include "init_sdl2.hpp"

#if defined(_WIN32)
#include <Windows.h>

#define TIME_BEGIN_PERIOD(p) timeBeginPeriod(p)
#define TIME_END_PERIOD(p) timeEndPeriod(p)
#else
#define TIME_BEGIN_PERIOD(p) (p)
#define TIME_END_PERIOD(p) (p)
#endif  // _WIN32

namespace rgm::base {
struct timer {
  uint64_t counter;
  uint64_t frequency;
  int64_t error_counter;
  uint32_t period_min;
#if defined(_WIN32)
  HANDLE waitable_timer;
#endif  // _WIN32

  timer() {
    counter = SDL_GetPerformanceCounter();
    frequency = SDL_GetPerformanceFrequency();
    error_counter = 0;
    period_min = 1;
#if defined(_WIN32)
    // query min period
    TIMECAPS tc;
    timeGetDevCaps(&tc, sizeof(tc));
    period_min = std::max(period_min, tc.wPeriodMin);
    // create timer
    waitable_timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
    if (!waitable_timer)
      cen::log_warn("[timer] CreateWaitableTimer FAILED with %08x",
                    HRESULT_FROM_WIN32(GetLastError()));
  }

  ~timer() {
    if (waitable_timer) CloseHandle(waitable_timer);
#endif  // _WIN32
  }

  /// @param[in] delay  单位：SDL performance counter
  /// @return           单位：SDL performance counter
  uint64_t predict_delay(uint64_t delay) { return delay; }

  /// @param[in] delay      传递给延时函数的时间，单位同上
  /// @param[in] real_delay 实际延时的时间，单位同上
  void update_model([[maybe_unused]] uint64_t delay,
                    [[maybe_unused]] uint64_t real_delay) {}

  void tick(double interval) {
    uint64_t next_counter;

    next_counter = counter + round(frequency * interval);
    uint64_t before_counter = SDL_GetPerformanceCounter();

    if (before_counter < next_counter) [[likely]] {
      uint64_t delta_counter = (next_counter - before_counter) - error_counter;
      time_t delay_ns =
          static_cast<time_t>(predict_delay(delta_counter) * (1E9 / frequency));

      TIME_BEGIN_PERIOD(period_min);
#if defined(_WIN32)
      bool waited = false;
      if (waitable_timer) {
        // WaitableTimer
        LARGE_INTEGER dt;
        dt.QuadPart = delay_ns / -100;
        HRESULT hr =
            SetWaitableTimer(waitable_timer, &dt, 0, nullptr, nullptr, FALSE);
        if (FAILED(hr)) [[unlikely]] {
          cen::log_warn("[timer] SetWaitableTimer FAILED with %08x", hr);
        } else [[likely]] {
          WaitForSingleObject(waitable_timer, INFINITE);
          waited = true;
        }
      }
      if (!waited) {
        // system sleep
        Sleep(lroundl(delta_counter / 1E6));
      }
#else
      // POSIX sleep
      timespec dt;
      dt.tv_sec = delay_ns / long(1E6);
      dt.tv_nsec = delay_ns % long(1E6);
      nanosleep(&dt, nullptr);
#endif  // _WIN32
      TIME_END_PERIOD(period_min);

      counter = SDL_GetPerformanceCounter();
      uint64_t real_delay_counter = counter - before_counter;
      error_counter = static_cast<int64_t>(real_delay_counter - delta_counter);
      update_model(delta_counter, real_delay_counter);
    } else [[unlikely]] {
      counter = before_counter;
      error_counter = 0;
    }
  }

  void reset() { counter = SDL_GetPerformanceCounter(); }
};
}  // namespace rgm::base
