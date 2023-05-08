// zlib License
//
// copyright (C) 2023 Guoxiaomi and Krimiston
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

#include "shader_base.hpp"

/*
 * opengl 的 shader 并不是预编译的，而是在运行时编译，将 glsl 代码内嵌
 * 于 exe 中。对于 RGSS 来说，不需要 vertex shader，故所有的 vertex
 * shader 都是相同的，并且没有任何效果。
 */
INCBIN(shader_default_vs, "./src/shader/opengl/default.vs");
INCBIN(shader_gray_fs, "./src/shader/opengl/gray.fs");
INCBIN(shader_hue_fs, "./src/shader/opengl/hue.fs");
INCBIN(shader_tone_fs, "./src/shader/opengl/tone.fs");
INCBIN(shader_transition_fs, "./src/shader/opengl/transition.fs");

/* gl 系列的函数声明 */
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLVALIDATEPROGRAMPROC glValidateProgram = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocation = nullptr;
PFNGLUNIFORM4FPROC glUniform4f = nullptr;

namespace rgm::shader {
/// @brief 基本的 shader 类对 direct3d9 渲染器的特化
template <>
struct shader_base<opengl> {
  /// @brief 初始化 OpenGL
  static void setup(cen::renderer&) {
    bool ret = init_gl_functions();
    if (!ret) throw std::system_error{};
  }

  /// @brief 设置 gl 系列函数的地址
  [[nodiscard]] static bool init_gl_functions() {
    glCreateShader =
        (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
    glShaderSource =
        (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
    glCompileShader =
        (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
    glGetShaderiv =
        (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
    glGetShaderInfoLog =
        (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
    glDeleteShader =
        (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
    glAttachShader =
        (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
    glCreateProgram =
        (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
    glLinkProgram =
        (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
    glValidateProgram =
        (PFNGLVALIDATEPROGRAMPROC)SDL_GL_GetProcAddress("glValidateProgram");
    glGetProgramiv =
        (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress(
        "glGetProgramInfoLog");
    glUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
    glGetUniformLocation =
        (PFNGLGETUNIFORMLOCATIONARBPROC)SDL_GL_GetProcAddress(
            "glGetUniformLocation");
    glUniform4f = (PFNGLUNIFORM4FPROC)SDL_GL_GetProcAddress("glUniform4f");

    return glCreateShader && glShaderSource && glCompileShader &&
           glGetShaderiv && glGetShaderInfoLog && glDeleteShader &&
           glAttachShader && glCreateProgram && glLinkProgram &&
           glValidateProgram && glGetProgramiv && glGetProgramInfoLog &&
           glUseProgram && glGetUniformLocation && glUniform4f;
  }

  /// @brief 读取 shader
  /// @param shaderType shader 的类型，分 GL_VERTEX_SHADER 和 GL_FRAGMENT_SHADER
  /// @param source glsl 的源码路径
  /// @param source_size glsl 的源码长度
  /// @return 编译成功后的 shader 对应的唯一 ID
  [[nodiscard]] static GLuint load_shader(GLenum shaderType,
                                          const GLchar* source,
                                          const GLint source_size) {
    GLuint shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &source, &source_size);
    glCompileShader(shaderID);
    GLint result = GL_FALSE;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
    if (result) {
      cen::log_debug("Successfully compiling shader.\n");
    } else {
      cen::log_error("Error in compiling shader.\n");
      GLint logLength;
      glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
      if (logLength > 0) {
        std::string log;
        log.resize(logLength);
        glGetShaderInfoLog(shaderID, logLength, &logLength, log.data());
        cen::log_error("Shader compile log: %s", log.data());
      }
      glDeleteShader(shaderID);
      return 0;
    }
    return shaderID;
  }

  /// @brief 编译 shader 程序
  /// @param vertexShaderID
  /// @param fragmentShaderId
  /// @return 编译成功后的程序对应的唯一 ID
  [[nodiscard]] static GLuint compile_program(GLuint vertexShaderID,
                                              GLuint fragmentShaderId) {
    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexShaderID);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderId);
    return programId;
  }
};

/// @brief 动态 shader 类对 opengl 渲染器的特化
/// @tparam T_shader shader 的类型
/// 利用 RAII 机制切换 shader，渲染效果执行结束后再切回来。
/// opengl 的 static 和 dynamic 一样，不需要特化，也不会用上。
template <template <config::driver_type> class T_shader>
struct shader_dynamic<opengl, T_shader> : shader_base<opengl> {
  /* T_shader 对不同的渲染器也有特化 */
  using T = T_shader<opengl>;

  /* T_shader 对应的程序 ID */
  inline static GLint program_id = 0;

  /* 存储调用此 shader 之前的程序 ID，用于后续恢复 */
  GLint last_program_id;

  /// @brief 构造函数
  explicit shader_dynamic() {
    /* 存储旧的 shader */
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_program_id);

    /* 应用 T_shader */
    glUseProgram(program_id);
  }

  ~shader_dynamic() {
    /* 还原旧的 shader */
    glUseProgram(last_program_id);
  }

  /// @brief 初始化 T_shader
  /// @param 渲染器
  static void setup(cen::renderer&) {
    /* 编译 VERTEX SHADER */
    GLint vertex_shader;
    if constexpr (requires { T::vertex; }) {
      vertex_shader =
          load_shader(GL_VERTEX_SHADER,
                      reinterpret_cast<const char*>(T::vertex), T::vertex_size);
    } else {
      vertex_shader =
          load_shader(GL_VERTEX_SHADER,
                      reinterpret_cast<const char*>(rgm_shader_default_vs_data),
                      rgm_shader_default_vs_size);
    }

    /* 编译 FRAGMENT SHADER */
    GLint fragment_shader = load_shader(
        GL_FRAGMENT_SHADER, reinterpret_cast<const char*>(T::fragment),
        T::fragment_size);

    /* 编译程序 */
    program_id = compile_program(vertex_shader, fragment_shader);
    cen::log_info("Successfully compiling opengl program, id = %d\n",
                  program_id);
  }
};

/// @brief 用于实现灰度的 shader 类对 opengl 渲染器的特化
template <>
struct shader_gray<opengl> : shader_dynamic<opengl, shader_gray> {
  static constexpr const unsigned char* fragment = rgm_shader_gray_fs_data;
  inline static const int fragment_size = rgm_shader_gray_fs_size;
};

/// @brief 用于实现色相的 shader 类对 opengl 渲染器的特化
template <>
struct shader_hue<opengl> : shader_dynamic<opengl, shader_hue> {
  static constexpr const unsigned char* fragment = rgm_shader_hue_fs_data;
  inline static const int fragment_size = rgm_shader_hue_fs_size;

  explicit shader_hue(int hue) {
    constexpr double pi = 3.141592653589793;
    constexpr double r3 = 1.7320508075688772;
    double angle = (pi / 180.0f) * hue;

    float k1 = (1.0 - cos(angle) - r3 * sin(angle)) / 3.0;
    float k2 = (1.0 - cos(angle) + r3 * sin(angle)) / 3.0;
    float k0 = 1.0 - k1 - k2;

    /* 设置 GL Uniform */
    static const auto location = glGetUniformLocation(program_id, "k");
    glUniform4f(location, k0, k1, k2, 0);
  }
};

/// @brief 用于实现色调的 shader 类对 opengl 渲染器的特化
template <>
struct shader_tone<opengl> : shader_dynamic<opengl, shader_tone> {
  static constexpr const unsigned char* fragment = rgm_shader_tone_fs_data;
  inline static const int fragment_size = rgm_shader_tone_fs_size;

  explicit shader_tone(rmxp::tone t) {
    float red = t.red / 255.0f;
    float green = t.green / 255.0f;
    float blue = t.blue / 255.0f;
    float gray = t.gray / 255.0f;

    /* 设置 GL Uniform */
    static const auto location = glGetUniformLocation(program_id, "tone");
    glUniform4f(location, red, green, blue, gray);
  }
};

/// @brief 用于实现渐变的 shader 类对 opengl 渲染器的特化
template <>
struct shader_transition<opengl> : shader_dynamic<opengl, shader_transition> {
  static constexpr const unsigned char* fragment =
      rgm_shader_transition_fs_data;
  inline static const int fragment_size = rgm_shader_transition_fs_size;

  explicit shader_transition(double rate, int vague) {
    float k0, k1, k2, k3;

    if (vague == 0) {
      k0 = 0;
      k1 = rate;
      k2 = 0;
      k3 = 0;
    } else {
      k0 = 1;
      k1 = 0;
      k2 = rate - vague / 255.0;
      k3 = 255.0 / vague;
    }

    /* 设置 GL Uniform */
    static const auto location = glGetUniformLocation(program_id, "k");
    glUniform4f(location, k0, k1, k2, k3);
  }
};
}  // namespace rgm::shader