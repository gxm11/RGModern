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
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

#include "shader_base.hpp"

INCTXT(shader_default_vs, "./src/shader/opengl/default.vs");
INCTXT(shader_gray_fs, "./src/shader/opengl/gray.fs");
INCTXT(shader_hue_fs, "./src/shader/opengl/hue.fs");
INCTXT(shader_tone_fs, "./src/shader/opengl/tone.fs");
INCTXT(shader_transition_fs, "./src/shader/opengl/transition.fs");

PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLVALIDATEPROGRAMPROC glValidateProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocation;
PFNGLUNIFORM4FPROC glUniform4f;

namespace rgm::shader {
template <>
struct shader_base<opengl> {
  static void setup(cen::renderer&) {
    bool ret = initGLExtensions();
    printf("gl init: %d\n", ret);
  }

  static bool initGLExtensions() {
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

  static GLuint load_shader(GLenum shaderType, const GLchar* source) {
    GLuint shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &source, NULL);
    glCompileShader(shaderID);
    GLint result = GL_FALSE;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
    if (result) {
      printf("Successfully compiling shader.\n");
    } else {
      printf("Error in compiling shader.\n");
      GLint logLength;
      glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
      if (logLength > 0) {
        std::string log;
        log.resize(logLength);
        glGetShaderInfoLog(shaderID, logLength, &logLength, log.data());
        std::cout << "Shader compile log:" << log.data() << std::endl;
      }
      glDeleteShader(shaderID);
      return 0;
    }
    return shaderID;
  }

  static GLuint compile_program(GLuint vertexShaderID,
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

// opengl 的 static 和 dynamic 一样，不需要特化，也不会用上。
template <template <size_t> class T_shader>
struct shader_dynamic<opengl, T_shader> : shader_base<opengl> {
  using T = T_shader<opengl>;

  static GLint program_id;
  GLint last_program_id;

  static void setup(cen::renderer&) {
    GLint vertex_shader;
    if constexpr (requires { T::vertex; }) {
      vertex_shader = load_shader(GL_VERTEX_SHADER, T::vertex);
    } else {
      vertex_shader = load_shader(GL_VERTEX_SHADER, rgm_shader_default_vs_data);
    }
    GLint fragment_shader = load_shader(GL_FRAGMENT_SHADER, T::fragment);
    program_id = compile_program(vertex_shader, fragment_shader);
    printf("program id = %d\n", program_id);
  }

  explicit shader_dynamic() {
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_program_id);
    glUseProgram(program_id);
  }

  ~shader_dynamic() { glUseProgram(last_program_id); }
};
template <template <size_t> class T_shader>
GLint shader_dynamic<opengl, T_shader>::program_id;

template <>
struct shader_gray<opengl> : shader_dynamic<opengl, shader_gray> {
  static constexpr const char* fragment = rgm_shader_gray_fs_data;
};

template <>
struct shader_hue<opengl> : shader_dynamic<opengl, shader_hue> {
  static constexpr const char* fragment = rgm_shader_hue_fs_data;

  explicit shader_hue(int hue) {
    constexpr double pi = 3.141592653589793;
    constexpr double r3 = 1.7320508075688772;
    double angle = (pi / 180.0f) * hue;

    float k1 = (1.0 - cos(angle) - r3 * sin(angle)) / 3.0;
    float k2 = (1.0 - cos(angle) + r3 * sin(angle)) / 3.0;
    float k0 = 1.0 - k1 - k2;

    static const auto location = glGetUniformLocation(program_id, "k");
    if (location >= 0) {
      glUniform4f(location, k0, k1, k2, 0);
    }
  }
};

template <>
struct shader_tone<opengl> : shader_dynamic<opengl, shader_tone> {
  static constexpr const char* fragment = rgm_shader_tone_fs_data;

  explicit shader_tone(rmxp::tone t) {
    float red = t.red / 255.0f;
    float green = t.green / 255.0f;
    float blue = t.blue / 255.0f;
    float gray = t.gray / 255.0f;

    static const auto location = glGetUniformLocation(program_id, "tone");
    if (location >= 0) {
      glUniform4f(location, red, green, blue, gray);
    }
  }
};

template <>
struct shader_transition<opengl> : shader_dynamic<opengl, shader_transition> {
  static constexpr const char* fragment = rgm_shader_transition_fs_data;

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

    static const auto location = glGetUniformLocation(program_id, "k");
    if (location >= 0) {
      glUniform4f(location, k0, k1, k2, k3);
    }
  }
};
}  // namespace rgm::shader