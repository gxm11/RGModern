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

namespace rgm::config {
// configs
bool btest = false;
bool debug = false;

const char* game_title = "RGModern";
const char* game_config = "./config.ini";
const char* resource_prefix = "resource://";

// constexprs
constexpr int window_width = 640;
constexpr int window_height = 480;
constexpr bool check_renderstack = false;

#ifndef RGM_BUILDMODE
#define RGM_BUILDMODE 1
#endif

#if RGM_BUILDMODE >= 2
constexpr int output_level = 0;
#define RGM_EMBEDED_ZIP
#else
constexpr int output_level = 1;
#endif

constexpr int build_mode = RGM_BUILDMODE;

constexpr bool asynchornized = true;
}  // namespace rgm::config

#ifndef RGM_FULLVERSION
#define RGM_FULLVERSION "RGM_FULLVERSION"
#endif

#ifndef CC_VERSION
#define CC_VERSION "CC_VERSION"
#endif

#ifndef RGM_USE_OPENGL
#define RGM_USE_OPENGL
#endif