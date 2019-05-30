#pragma once

#include <ogl.h>
#include <iostream>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gl/program.hpp"
#include <glm/gtx/rotate_vector.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include <framebuffer.hpp>
#include <texture.hpp>

#include "camera.hpp"

class particle_system {
private:
    /* Opengl objects and buffers*/
    GLuint m_vao;
    GLuint m_particle_buffer;
    GLuint m_dead_buffer;
    GLuint m_alive_buffer;
    GLuint m_alive_buffer_prev;
    GLuint m_atomic_counter_buffer;

    /* maximum number of particles */
    int m_particle_count = 1 << 20;
    /* maximum number of particles emitted each frame */
    GLuint m_emission = 512;
    glm::vec3 m_curr_pos;
    glm::vec3 m_prev_pos;

    /* Exposed parameters */
    GLuint m_alive_count = 0;
    float m_ttl = 2.;
    float m_gravity = 0;
    float m_resistance = 1.;
    float m_noise_resolution = 10.;
    float m_noise_strength = 0.1;
    glm::vec3 m_start_color = glm::vec3(1, .6, 0.);
    glm::vec3 m_end_color = glm::vec3(.2, .8, .2);

    struct particle_format {
	glm::vec3 position;
	float pad1;
	glm::vec3 velocity;
	float start_ttl;
	glm::vec3 start_velocity;
	float ttl;
    };

    std::shared_ptr<program> m_compute_program;
    std::shared_ptr<program> m_draw_program;
    std::shared_ptr<program> m_emission_program;

public:
    particle_system();
    void update(float time, float dt);
    void draw(camera& cam);
    void set_position(glm::vec3 pos);
    glm::vec3 get_position();
    float get_gravity() { return m_gravity; }
    void set_gravity(float gravity) {
	m_gravity = gravity;
    }
    size_t get_particle_count() {
	return m_alive_count;
    }
    void set_noise_resolution(float noise_resolution) {
	m_noise_resolution = noise_resolution;
    }
    float get_noise_resolution() {
	return m_noise_resolution;
    }
    void set_resistance(float resistance) {
	m_resistance = resistance;
    }
    float get_resistance() {
	return m_resistance;
    }

    void set_max_ttl(float ttl) {
	m_ttl = ttl;
    }
    float get_max_ttl() {
	return m_ttl;
    }

    GLuint get_emission() {
	return m_emission;
    }

    void set_emission(GLuint e) {
	m_emission = e;
    }

    glm::vec3 get_start_color() {
	return m_start_color;
    }

    void set_start_color(glm::vec3 color) {
	m_start_color = color;
    }

    glm::vec3 get_end_color() {
	return m_end_color;
    }

    void set_end_color(glm::vec3 color) {
	m_end_color = color;
    }
};
