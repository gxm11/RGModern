// zlib License

// copyright (C) 2023 Guoxiaomi and Krimiston

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
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

#include "shader_base.hpp"

INCBIN(shader_default_vs, "./src/shader/opengl/default.vs");
INCBIN(shader_gray_fs, "./src/shader/opengl/gray.fs");
INCBIN(shader_hue_fs, "./src/shader/opengl/hue.fs");
INCBIN(shader_tone_fs, "./src/shader/opengl/tone.fs");
INCBIN(shader_transition_fs, "./src/shader/opengl/transition.fs");

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
  static void setup(cen::renderer&) { initGLExtensions(); }

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

  static GLuint load_shader(GLenum shaderType, const GLchar* source,
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
template <template <config::driver_type> class T_shader>
struct shader_dynamic<opengl, T_shader> : shader_base<opengl> {
  using T = T_shader<opengl>;

  static GLint program_id;
  GLint last_program_id;

  static void setup(cen::renderer&) {
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
    GLint fragment_shader = load_shader(
        GL_FRAGMENT_SHADER, reinterpret_cast<const char*>(T::fragment),
        T::fragment_size);
    program_id = compile_program(vertex_shader, fragment_shader);
    cen::log_info("Successfully compiling opengl program, id = %d\n",
                  program_id);
  }

  explicit shader_dynamic() {
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_program_id);
    glUseProgram(program_id);
  }

  ~shader_dynamic() { glUseProgram(last_program_id); }
};
template <template <config::driver_type> class T_shader>
GLint shader_dynamic<opengl, T_shader>::program_id;

template <>
struct shader_gray<opengl> : shader_dynamic<opengl, shader_gray> {
  static constexpr const unsigned char* fragment = rgm_shader_gray_fs_data;
  static const int fragment_size;
};
const int shader_gray<opengl>::fragment_size = rgm_shader_gray_fs_size;

template <>
struct shader_hue<opengl> : shader_dynamic<opengl, shader_hue> {
  static constexpr const unsigned char* fragment = rgm_shader_hue_fs_data;
  static const int fragment_size;

  explicit shader_hue(int hue) {
    constexpr double pi = 3.141592653589793;
    constexpr double r3 = 1.7320508075688772;
    double angle = (pi / 180.0f) * hue;

    float k1 = (1.0 - cos(angle) - r3 * sin(angle)) / 3.0;
    float k2 = (1.0 - cos(angle) + r3 * sin(angle)) / 3.0;
    float k0 = 1.0 - k1 - k2;

    static const auto location = glGetUniformLocation(program_id, "k");
    glUniform4f(location, k0, k1, k2, 0);
  }
};
const int shader_hue<opengl>::fragment_size = rgm_shader_hue_fs_size;

template <>
struct shader_tone<opengl> : shader_dynamic<opengl, shader_tone> {
  static constexpr const unsigned char* fragment = rgm_shader_tone_fs_data;
  static const int fragment_size;

  explicit shader_tone(rmxp::tone t) {
    float red = t.red / 255.0f;
    float green = t.green / 255.0f;
    float blue = t.blue / 255.0f;
    float gray = t.gray / 255.0f;

    static const auto location = glGetUniformLocation(program_id, "tone");
    glUniform4f(location, red, green, blue, gray);
  }
};
const int shader_tone<opengl>::fragment_size = rgm_shader_tone_fs_size;

template <>
struct shader_transition<opengl> : shader_dynamic<opengl, shader_transition> {
  static constexpr const unsigned char* fragment =
      rgm_shader_transition_fs_data;
  static const int fragment_size;

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
    glUniform4f(location, k0, k1, k2, k3);
  }
};
const int shader_transition<opengl>::fragment_size =
    rgm_shader_transition_fs_size;

}  // namespace rgm::shader