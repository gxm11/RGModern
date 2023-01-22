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
#include "builtin.hpp"
// #include "shader_empty.hpp"
#include "base/base.hpp"

INCTXT(shader_test_f, "./src/shader/opengl/f.glsl");
INCTXT(shader_test_v, "./src/shader/opengl/v.glsl");

namespace rgm::rmxp {

struct shader_gray {};

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
  static GLuint programId;
  shader_hue(int) { glUseProgram(programId); }
};
GLuint shader_hue::programId = 0;
template <>
struct init_shader<shader_hue> {
#if 0
  static GLuint compileShader(const char* source, GLuint shaderType) {
    std::cout << "Compilando shader:" << std::endl << source << std::endl;
    // Create ID for shader
    GLuint result = glCreateShader(shaderType);
    // Define shader text
    glShaderSource(result, 1, &source, NULL);
    // Compile shader
    glCompileShader(result);

    // Check vertex shader for errors
    GLint shaderCompiled = GL_FALSE;
    glGetShaderiv(result, GL_COMPILE_STATUS, &shaderCompiled);
    if (shaderCompiled != GL_TRUE) {
      std::cout << "Error en la compilación: " << result << "!" << std::endl;
      GLint logLength;
      glGetShaderiv(result, GL_INFO_LOG_LENGTH, &logLength);
      if (logLength > 0) {
        GLchar* log = (GLchar*)malloc(logLength);
        glGetShaderInfoLog(result, logLength, &logLength, log);
        std::cout << "Shader compile log:" << log << std::endl;
        free(log);
      }
      glDeleteShader(result);
      result = 0;
    } else {
      std::cout << "Shader compilado correctamente. Id = " << result
                << std::endl;
    }
    return result;
  }

  static GLuint compileProgram(const char* vtxFile, const char* fragFile) {
    GLuint programId = 0;
    GLuint vtxShaderId, fragShaderId;

    programId = glCreateProgram();

    std::ifstream f(vtxFile);
    std::string source((std::istreambuf_iterator<char>(f)),
                       std::istreambuf_iterator<char>());
    vtxShaderId = compileShader(source.c_str(), GL_VERTEX_SHADER);

    f = std::ifstream(fragFile);
    source = std::string((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    fragShaderId = compileShader(source.c_str(), GL_FRAGMENT_SHADER);

    if (vtxShaderId && fragShaderId) {
      // Associate shader with program
      glAttachShader(programId, vtxShaderId);
      glAttachShader(programId, fragShaderId);
      glLinkProgram(programId);
      glValidateProgram(programId);

      // Check the status of the compile/link
      GLint logLen;
      glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLen);
      if (logLen > 0) {
        char* log = (char*)malloc(logLen * sizeof(char));
        // Show any errors as appropriate
        glGetProgramInfoLog(programId, logLen, &logLen, log);
        std::cout << "Prog Info Log: " << std::endl << log << std::endl;
        free(log);
      }
    }
    if (vtxShaderId) {
      glDeleteShader(vtxShaderId);
    }
    if (fragShaderId) {
      glDeleteShader(fragShaderId);
    }
    return programId;
  }
#endif
  static GLuint loadShader(GLenum shaderType, const GLchar* source) {
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
        // GLchar* log = (GLchar*)malloc(logLength);
        glGetShaderInfoLog(shaderID, logLength, &logLength, log.data());
        std::cout << "Shader compile log:" << log.data() << std::endl;
      }
      glDeleteShader(shaderID);
      return 0;
    }
    return shaderID;
  }

  static GLuint compileProgram(GLuint vertexShaderID, GLuint fragmentShaderId) {
    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexShaderID);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderId);
    return programId;
  }

  static void before(auto&) {
    glewInit();
    auto vertexShaderID = loadShader(GL_VERTEX_SHADER, rgm_shader_test_v_data);
    auto fragmentShaderID =
        loadShader(GL_FRAGMENT_SHADER, rgm_shader_test_f_data);
    auto programID = compileProgram(vertexShaderID, fragmentShaderID);
    printf("compile program, id = %d\n.", programID);
    shader_hue::programId = programID;
  }
};
}  // namespace rgm::rmxp