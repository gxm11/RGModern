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
#include <chrono>
#include <thread>

namespace rgm::base {
#if 1
struct timer {
  uint64_t counter;
  uint64_t frequency;
  int64_t error_counter;

  timer() {
    counter = SDL_GetPerformanceCounter();
    frequency = SDL_GetPerformanceFrequency();
    error_counter = 0;
  }

  void tick(double interval) {
    uint64_t next_counter;

    next_counter = counter + round(frequency * interval);
    counter = SDL_GetPerformanceCounter();

    if (counter < next_counter) {
      long double delta_counter = (next_counter - counter) - error_counter;
      long long delay_ns = static_cast<long long>(delta_counter * (1E9 / frequency));
      std::this_thread::sleep_for(std::chrono::nanoseconds(delay_ns));
      
      long double before_counter = counter;
      counter = SDL_GetPerformanceCounter();
      error_counter = static_cast<int64_t>((counter - before_counter) - delta_counter);
    }
    else {
      counter = SDL_GetPerformanceCounter();
      error_counter = 0;
    }
  }

  void reset() { counter = SDL_GetPerformanceCounter(); }
};
#else
// Author: Ryan M. Geiss
// http://www.geisswerks.com/ryan/FAQS/timing.html
struct timer {
  timer() {
    QueryPerformanceFrequency(&freq_);
    QueryPerformanceCounter(&time_);
  }

  void tick(double interval) {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);

    if (time_.QuadPart != 0) {
      int ticks_to_wait =
          static_cast<int>(static_cast<double>(freq_.QuadPart) * interval);
      int done = 0;
      while (!done) {
        QueryPerformanceCounter(&t);

        int ticks_passed =
            static_cast<int>(static_cast<__int64>(t.QuadPart) -
                             static_cast<__int64>(time_.QuadPart));
        int ticks_left = ticks_to_wait - ticks_passed;

        if (t.QuadPart < time_.QuadPart)  // time wrap
        {
          done = 1;
        }
        if (ticks_passed >= ticks_to_wait) {
          done = 1;
        }
        if (!done) {
          // if > 0.002s left, do Sleep(1), which will actually sleep some
          //   steady amount, probably 1-2 ms,
          //   and do so in a nice way (cpu meter drops; laptop battery spared).
          // otherwise, do a few Sleep(0)'s, which just give up the timeslice,
          //   but don't really save cpu or battery, but do pass a tiny
          //   amount of time.
          if (ticks_left > static_cast<int>((freq_.QuadPart * 2) / 1000)) {
            Sleep(1);
          } else {
            for (int i = 0; i < 10; ++i) {
              Sleep(0);  // causes thread to give up its timeslice
            }
          }
        }
      }
    }

    time_ = t;
  }

  void reset() {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    time_ = t;
  }

  // private:
  LARGE_INTEGER freq_;
  LARGE_INTEGER time_;
};
#endif
}  // namespace rgm::base