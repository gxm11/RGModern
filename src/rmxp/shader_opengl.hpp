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
#include "GL/glew.h"
#include "base/base.hpp"
#include "builtin.hpp"

INCTXT(shader_default_vs, "./src/shader/opengl/default.vs");
INCTXT(shader_gray_fs, "./src/shader/opengl/gray.fs");

namespace rgm::rmxp {
struct shader_base {
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

template <typename T>
struct shader_static : shader_base {
  static GLint program_id;
  GLint last_program_id;

  shader_static() {
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

struct shader_gray : shader_static<shader_gray> {
  static constexpr const char* fragment = rgm_shader_gray_fs_data;
};

struct shader_tone {
  shader_tone(tone) {}
};

struct shader_transition {
  shader_transition(double, int) {}
};

template <typename T>
struct init_shader {};
// ----------------------------------------------------------------
// TESTING
// ----------------------------------------------------------------
struct shader_hue {
  shader_hue(int) {}
};

template <>
struct init_shader<shader_gray> {
  static void before(auto&) {
    glewInit();
    shader_gray::setup();
  }
};
}  // namespace rgm::rmxp