#pragma once

#include "ogl.h"
#include <string>
#include <memory>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>

class shader {
private:
    GLuint shader_id;    
    shader(const char* shader_src, GLuint shader_type)
    {
        shader_id = glCreateShader(shader_type);
        glShaderSource(shader_id, 1, &shader_src, NULL);
        glCompileShader(shader_id);

        GLint success;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);

        if( !success ) {
            shader_id = 0;
            char info_log[1024];
            glGetShaderInfoLog(shader_id, 1024, NULL, info_log);
            fprintf(stderr, "Failed compiling shader %s : %s\n", shader_src, info_log);
        }
    }

public:
    static std::shared_ptr<shader> compile(std::string source_path, GLuint shader_type)
    {        
        std::ifstream shader_file(source_path.c_str());
        std::stringstream shader_sstream;
        std::string shader_string;
        shader_sstream << shader_file.rdbuf();
        shader_string = shader_sstream.str();
        const char* shader_src = shader_string.c_str();
        shader_file.close();
        
        return std::shared_ptr<shader>(new shader(shader_src, shader_type),
            [=](shader* s) {
                glDeleteShader(s->shader_id);                                           
                delete s;
        });
    }

    GLuint get_id() {
        return shader_id;
    }
};

class program {
private:
    GLuint program_id;
    std::unordered_map<std::string, GLuint> uniform_location_cache;
    
    program()
    {        
        program_id = glCreateProgram();
    }

public:
    static std::shared_ptr<program> create() {
        return std::shared_ptr<program>(new program(),
                                        [=](program* p) {
                                            glDeleteProgram(p->program_id);                                          
                                            delete p;
                                        });
    }

    void attachShader(std::shared_ptr<shader> shader) {
        glAttachShader(program_id, shader->get_id());
    }

    void transformFeedbackVaryings(GLsizei count, const char **varyings, GLenum bufferMode) {
        glTransformFeedbackVaryings(program_id, count, varyings, bufferMode);
    }
              
    void link() {
        glLinkProgram(program_id);
    }

    void check_status() {
        GLint success;
        glGetProgramiv(program_id, GL_LINK_STATUS, &success);
        if( !success ){
            char info_log[1024];
            glGetProgramInfoLog(program_id, 1024, NULL, info_log);
            fprintf(stderr, "Failed linking program (id : %d) : %s \n", program_id,
                    info_log);
            //exit(0);
        }
    }
    
    void use() {
        glUseProgram(program_id);
    }

    GLuint get_id() {
        return program_id;
    }

    GLuint getUniformLocation(std::string name) {
        if (uniform_location_cache.find(name) == uniform_location_cache.end()) {
            uniform_location_cache[name] = glGetUniformLocation(program_id, name.c_str());
        }
        return uniform_location_cache[name];
    }
};
