// Copyright (c) 2022 Xiaomi Guo
// Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan
// PSL v2. You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
// Mulan PSL v2 for more details.

#pragma once
#include "init_sdl2.hpp"
#include <time.h>

#if defined(_WIN32)
#include <Windows.h>

#define TIME_BEGIN_PERIOD(p) timeBeginPeriod(p)
#define TIME_END_PERIOD(p) timeEndPeriod(p)
#else
#define TIME_BEGIN_PERIOD(p) (p)
#define TIME_END_PERIOD(p) (p)
#endif // _WIN32

namespace rgm::base {
struct timer {
  uint64_t counter;
  uint64_t frequency;
  int64_t error_counter;
  uint32_t period_min;
#if defined(_WIN32)
  HANDLE waitable_timer;
#endif // _WIN32

  timer() {
    counter = SDL_GetPerformanceCounter();
    frequency = SDL_GetPerformanceFrequency();
    error_counter = 0;
    period_min = 1;
#if defined(_WIN32)
    // query min period
    TIMECAPS tc = { 0 };
    timeGetDevCaps(&tc, sizeof(tc));
    period_min = std::max(period_min, tc.wPeriodMin);
    // create timer
    waitable_timer = CreateWaitableTimer(
      nullptr,
      TRUE,
      nullptr
    );
    if (!waitable_timer)
      cen::log_warn("[timer] CreateWaitableTimer FAILED with %08x", HRESULT_FROM_WIN32(GetLastError()));
  }

  ~timer() {
    if (waitable_timer) CloseHandle(waitable_timer);
#endif // _WIN32
  }

  /// @param[in] delay  单位：SDL performance counter
  /// @return           单位：SDL performance counter
  uint64_t predict_delay(uint64_t delay) {
    return delay;
  }

  /// @param[in] delay      传递给延时函数的时间，单位同上
  /// @param[in] real_delay 实际延时的时间，单位同上
  void train_model(uint64_t delay, uint64_t real_delay)
  {}

  void tick(double interval) {
    uint64_t next_counter;

    next_counter = counter + round(frequency * interval);
    uint64_t before_counter = SDL_GetPerformanceCounter();

    if (before_counter < next_counter) {
      uint64_t delta_counter = (next_counter - before_counter) - error_counter;
      time_t delay_ns = 
        static_cast<time_t>(predict_delay(delta_counter) * (1E9 / frequency));

      TIME_BEGIN_PERIOD(period_min);
#if defined(_WIN32)
      bool waited = false;
      if (waitable_timer) {
        // WaitableTimer
        LARGE_INTEGER dt = { 0 };
        dt.QuadPart = delay_ns / -100;
        HRESULT hr = SetWaitableTimer(
          waitable_timer,
          &dt,
          0,
          nullptr, nullptr,
          FALSE
        );
        if (FAILED(hr)) {
          cen::log_warn("[timer] SetWaitableTimer FAILED with %08x", hr);
        }
        else {
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
      timespec dt = { 0 };
      dt.tv_sec = delay_ns / long(1E6);
      dt.tv_nsec = delay_ns % long(1E6);
      nanosleep(&dt, nullptr);
#endif // _WIN32
      TIME_END_PERIOD(period_min);

      counter = SDL_GetPerformanceCounter();
      uint64_t real_delay_counter = counter - before_counter;
      error_counter = static_cast<int64_t>(real_delay_counter - delta_counter);
      train_model(delta_counter, real_delay_counter);
    }
    else {
      counter = before_counter;
      error_counter = 0;
    }
  }

  void reset() { counter = SDL_GetPerformanceCounter(); }
};
}  // namespace rgm::base
