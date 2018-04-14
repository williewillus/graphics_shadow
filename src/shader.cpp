#include <debuggl.h>
#include <iostream>
#include "shader.h"

ShaderProgram& ShaderProgram::addVsh(const char* src) {
    GLuint vertex_shader_id = 0;
    CHECK_GL_ERROR(vertex_shader_id = glCreateShader(GL_VERTEX_SHADER));
    CHECK_GL_ERROR(glShaderSource(vertex_shader_id, 1, &src, nullptr));
    glCompileShader(vertex_shader_id);
    CHECK_GL_SHADER_ERROR(vertex_shader_id);
    shaders_temp.push_back(vertex_shader_id);
    return *this;
}

ShaderProgram& ShaderProgram::addGsh(const char* src) {
    GLuint geometry_shader_id = 0;
    CHECK_GL_ERROR(geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER));
    CHECK_GL_ERROR(glShaderSource(geometry_shader_id, 1, &src, nullptr));
    glCompileShader(geometry_shader_id);
    CHECK_GL_SHADER_ERROR(geometry_shader_id);
    shaders_temp.push_back(geometry_shader_id);
    return *this;
}

ShaderProgram& ShaderProgram::addFsh(const char* src) {
    GLuint fragment_shader_id = 0;
    CHECK_GL_ERROR(fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER));
    CHECK_GL_ERROR(glShaderSource(fragment_shader_id, 1, &src, nullptr));
    glCompileShader(fragment_shader_id);
    CHECK_GL_SHADER_ERROR(fragment_shader_id);
    shaders_temp.push_back(fragment_shader_id);
    return *this;
}

ShaderProgram& ShaderProgram::addTsc(const char* src) {
    GLuint id = 0;
    CHECK_GL_ERROR(id = glCreateShader(GL_TESS_CONTROL_SHADER));
    CHECK_GL_ERROR(glShaderSource(id, 1, &src, nullptr));
    glCompileShader(id);
    CHECK_GL_SHADER_ERROR(id);
    shaders_temp.push_back(id);
    return *this;
}

ShaderProgram& ShaderProgram::addTse(const char* src) {
    GLuint id = 0;
    CHECK_GL_ERROR(id = glCreateShader(GL_TESS_EVALUATION_SHADER));
    CHECK_GL_ERROR(glShaderSource(id, 1, &src, nullptr));
    glCompileShader(id);
    CHECK_GL_SHADER_ERROR(id);
    shaders_temp.push_back(id);
    return *this;
}

ShaderProgram& ShaderProgram::build(const std::vector<std::string>& uniforms) {
    CHECK_GL_ERROR(program_id = glCreateProgram());
    for (auto shader : shaders_temp) {
        CHECK_GL_ERROR(glAttachShader(program_id, shader));
    }
    shaders_temp.clear();

    // Bind attributes (assume all of our programs have these attributes)
    CHECK_GL_ERROR(glBindAttribLocation(program_id, 0, "vertex_position"));
    CHECK_GL_ERROR(glBindFragDataLocation(program_id, 0, "fragment_color"));
    glLinkProgram(program_id);
    CHECK_GL_PROGRAM_ERROR(program_id);

    for (auto& u : uniforms) {
        GLuint location;
        CHECK_GL_ERROR(location = glGetUniformLocation(program_id, u.c_str()));
        uniforms_locations[u] = location;
    }

    return *this;
}

void ShaderProgram::activate() {
    CHECK_GL_ERROR(glUseProgram(program_id));
}

GLuint ShaderProgram::getUniform(const std::string& name) const {
    return uniforms_locations.at(name);
}
