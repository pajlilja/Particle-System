#pragma once

#include <ogl.h>
#include <iostream>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <numeric>
#include <algorithm>

#include "engine/scene.hpp"
#include "gl/program.hpp"

class app {
private:
    bool m_res_changed = false;
    glm::ivec2 m_res;
    GLuint m_buffer;
    std::vector<float> m_numbers;
    std::shared_ptr<program> m_sort_program;
    int pass = 0;
    int stage = 0;
public:    
    app(glm::ivec2 res)
        : m_res(res), m_numbers(1 << 20)
    {
        auto compute_shader = shader::compile("shaders/cs_sort.glsl",
                                              GL_COMPUTE_SHADER);
        m_sort_program = program::create();
        m_sort_program->attachShader(compute_shader);
        m_sort_program->link();

        for (int i = 0; i < m_numbers.size(); i++) {
            m_numbers[i] = (float) i;
        }
        std::random_shuffle(m_numbers.begin(), m_numbers.end());
        
        glGenBuffers(1, &m_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_numbers.size(),
                     m_numbers.data(), GL_STATIC_COPY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void resize(int width, int height) {
        m_res = glm::ivec2(width, height);
        m_res_changed = true;
    }

    void handle_input(ImGuiIO& io) {
    }

    void draw_info_window() {
    }
    
    void draw(float dt) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_buffer);

        glUseProgram(m_sort_program->get_id());

        for (int i = 0; i < 218; i++) {
            if(pass < 0) {
                stage++;
                pass = stage;            
            }
            if (stage >= int(log2(m_numbers.size()))) {
                stage = 0;
                pass = 0;
            } 
        
            int pstage = 1 << stage;
            int ppass = 1 << pass;
            GLuint loc = glGetUniformLocation(m_sort_program->get_id(), "pass");
            glUniform1f(loc, ppass);
            loc = glGetUniformLocation(m_sort_program->get_id(), "two_stage");
            glUniform1f(loc, 2 * pstage);
            loc = glGetUniformLocation(m_sort_program->get_id(),
                                       "pass_mod_stage");
            glUniform1f(loc, ppass % pstage);
            loc = glGetUniformLocation(m_sort_program->get_id(),
                                       "two_stage_pms_1");
            glUniform1f(loc, 2 * pstage - ppass % pstage - 1);
            glDispatchCompute(m_numbers.size() / 1024, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT |
            GL_BUFFER_UPDATE_BARRIER_BIT);

            float data[30];
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 30 * sizeof(float), data);
            for (int i = 0; i < 30; i++) {
                std::cout << data[i] << " ";
            }
            std::cout << std::endl;
            pass--;
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


    }
};
