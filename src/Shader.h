#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

class Shader {
public:
    unsigned int ID;

    Shader(const std::string& vertPath, const std::string& fragPath) {
        std::string vCode = readFile(vertPath);
        std::string fCode = readFile(fragPath);
        const char* v = vCode.c_str();
        const char* f = fCode.c_str();

        unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &v, nullptr);
        glCompileShader(vs);
        check(vs, "VERTEX");

        unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &f, nullptr);
        glCompileShader(fs);
        check(fs, "FRAGMENT");

        ID = glCreateProgram();
        glAttachShader(ID, vs);
        glAttachShader(ID, fs);
        glLinkProgram(ID);
        check(ID, "PROGRAM");

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    void use() const { glUseProgram(ID); }

    // Funções para enviar valores (uniforms) ao shader
    void setInt(const std::string& n, int v) const { glUniform1i(glGetUniformLocation(ID, n.c_str()), v); }
    void setFloat(const std::string& n, float v) const { glUniform1f(glGetUniformLocation(ID, n.c_str()), v); }
    void setVec3(const std::string& n, const glm::vec3& v) const { glUniform3fv(glGetUniformLocation(ID, n.c_str()), 1, &v[0]); }
    void setMat4(const std::string& n, const glm::mat4& m) const { glUniformMatrix4fv(glGetUniformLocation(ID, n.c_str()), 1, GL_FALSE, glm::value_ptr(m)); }
    void setVec2(const std::string& n, const glm::vec2& v) const { glUniform2fv(glGetUniformLocation(ID, n.c_str()), 1, &v[0]); }

private:
    static std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file) std::cerr << "Nao consegui abrir: " << path << "\n";
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    static void check(unsigned int obj, const std::string& type) {
        int ok; char log[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(obj, GL_COMPILE_STATUS, &ok);
            if (!ok) { glGetShaderInfoLog(obj, 1024, nullptr, log); std::cerr << "Erro shader " << type << ":\n" << log << "\n"; }
        } else {
            glGetProgramiv(obj, GL_LINK_STATUS, &ok);
            if (!ok) { glGetProgramInfoLog(obj, 1024, nullptr, log); std::cerr << "Erro link:\n" << log << "\n"; }
        }
    }
};