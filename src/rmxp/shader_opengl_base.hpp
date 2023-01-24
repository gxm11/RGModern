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

#include "base/base.hpp"

INCTXT(shader_default_vs, "./src/shader/opengl/default.vs");

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

namespace rgm::rmxp {
struct shader_base {
  static bool init;

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
bool shader_base::init = false;

template <typename T>
struct shader_static : shader_base {
  static GLint program_id;
  GLint last_program_id;

  explicit shader_static() {
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_program_id);
    glUseProgram(program_id);
  }

  ~shader_static() { glUseProgram(last_program_id); }

  static void setup() {
    GLint vertex_shader;
    if constexpr (requires { T::vertex; }) {
      vertex_shader = load_shader(GL_VERTEX_SHADER, T::vertex);
    } else {
      vertex_shader = load_shader(GL_VERTEX_SHADER, rgm_shader_default_vs_data);
    }
    GLint fragment_shader = load_shader(GL_FRAGMENT_SHADER, T::fragment);
    program_id = compile_program(vertex_shader, fragment_shader);
  }
};
template <typename T>
GLint shader_static<T>::program_id = 0;

template <typename T>
struct init_shader {
  static void before(auto&) {
    if (!shader_base::init) {
      shader_base::initGLExtensions();
    }
    if constexpr (requires { T::setup(); }) {
      T::setup();
    }
  }
};
}  // namespace rgm::rmxp