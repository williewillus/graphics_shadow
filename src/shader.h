#pragma once

#include <GL/glew.h>
#include <string>
#include <unordered_map>
#include <vector>

class ShaderProgram {
public:
    // Building (these require a GL context)
    ShaderProgram& addVsh(const char* source);
    ShaderProgram& addGsh(const char* source);
    ShaderProgram& addFsh(const char* source);
    ShaderProgram& addTsc(const char* source);
    ShaderProgram& addTse(const char* source);
    ShaderProgram& build(const std::vector<std::string>& uniforms);

    // Using
    void activate();
    GLuint getUniform(const std::string& name) const;
    GLuint get_uniform_direct(const std::string& name) const; // todo rename me
private:
    GLuint program_id = 0;
    std::unordered_map<std::string, GLuint> uniforms_locations;
    std::vector<GLuint> shaders_temp; // used while building
};
