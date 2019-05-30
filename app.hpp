#pragma once

#include <ogl.h>
#include <iostream>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include <framebuffer.hpp>

#include "gl/program.hpp"
#include "gl/framebuffer.hpp"
#include "gl/texture.hpp"

#include "camera.hpp"
#include "particle_system.hpp"

class app {
private:
    bool m_res_changed = false;
    glm::ivec2 m_res;
    /* exposed parameters */
    float m_time = 0.;
    float m_exposure = 1.0f;
    float m_gamma = 2.2f;
    glm::vec2 m_mouse_pos = glm::vec3(0.0f);
    float global_speed = 1.0f;
    camera m_camera;

    int m_animation = 0;

    /* full screen quad */
    GLuint m_plane_vao;
    GLuint m_plane_buffer;

    bool m_should_blur = true;
    bool m_pause_simulation = false;

    std::shared_ptr<program> m_texture_program;
    std::shared_ptr<program> m_blur_program;

    /* output from render */
    std::shared_ptr<texture> m_color_texture;
    /* these textures is used for the bloom effect */
    std::shared_ptr<texture> m_blur_texture;
    std::shared_ptr<texture> m_blur_texture2;

    /* framebuffer */
    std::shared_ptr<texture> m_depth_texture;
    std::shared_ptr<framebuffer> m_framebuffer;

    particle_system m_particle_system;
public:
    app(glm::ivec2 res)
        : m_res(res)
    {
	/* Setup textures
	 */
	setup_textures();

	/* setup shader programs */
	auto vertex_shader2 = shader::compile("shaders/vs_simple.glsl",
					      GL_VERTEX_SHADER);
	auto fragment_shader2 = shader::compile("shaders/fs_simple.glsl",
						GL_FRAGMENT_SHADER);
	m_texture_program = program::create();
	m_texture_program->attachShader(vertex_shader2);
	m_texture_program->attachShader(fragment_shader2);
	m_texture_program->link();

	auto blur_shader = shader::compile("shaders/cs_blur.glsl",
					   GL_COMPUTE_SHADER);
	m_blur_program = program::create();
	m_blur_program->attachShader(blur_shader);
	m_blur_program->link();

	/* Full screen quad setup, which is use in the final stage */
	glUseProgram(m_texture_program->get_id());
	GLuint loc = glGetUniformLocation(m_texture_program->get_id(), "res");
	glUniform2f(loc, res.x, res.y);
	loc = glGetUniformLocation(m_texture_program->get_id(), "color_texture");
	glUniform1i(loc, 0);
	loc = glGetUniformLocation(m_texture_program->get_id(), "bloom_texture");
	glUniform1i(loc, 1);
	glUseProgram(0);
	GLfloat plane[] = {-1, 1, -1, -1, 1, 1, -1, -1, 1, -1, 1, 1};
	glCreateVertexArrays(1, &m_plane_vao);
	glGenBuffers(1, &m_plane_buffer);
	glBindVertexArray(m_plane_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_plane_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0);
	glEnableVertexAttribArray(0);
    }

    void setup_textures() {
	m_color_texture = texture::genTexture(GL_TEXTURE_2D);
	m_color_texture->texStorage2D(1, GL_RGBA32F, m_res.x, m_res.y);
	m_color_texture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	m_color_texture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_color_texture->texParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_color_texture->texParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_blur_texture = texture::genTexture(GL_TEXTURE_2D);
	m_blur_texture->texStorage2D(1, GL_RGBA32F, m_res.x, m_res.y);
	m_blur_texture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_blur_texture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_blur_texture->texParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_blur_texture->texParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_blur_texture2 = texture::genTexture(GL_TEXTURE_2D);
	m_blur_texture2->texStorage2D(1, GL_RGBA32F, m_res.x, m_res.y);
	m_blur_texture2->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_blur_texture2->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_blur_texture2->texParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_blur_texture2->texParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_depth_texture = texture::genTexture(GL_TEXTURE_2D);
	m_depth_texture->texStorage2D(1, GL_DEPTH_COMPONENT24, m_res.x, m_res.y);
	m_depth_texture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_depth_texture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	m_framebuffer = framebuffer::genFramebuffer();
	m_framebuffer->framebufferTexture(m_color_texture, GL_COLOR_ATTACHMENT0);
	m_framebuffer->framebufferTexture(m_depth_texture, GL_DEPTH_ATTACHMENT);

	if (GL_FRAMEBUFFER_COMPLETE != m_framebuffer->checkFramebufferStatus()) {
	    fprintf(stderr, "Framebuffer is not complete\n");
	    exit(0);
	}

	//GLuint loc = glGetUniformLocation(m_texture_program->get_id(), "res");
	//glUniform2f(loc, m_res.x, m_res.y);
    }

    /**
     * Should be called when widnow is resized.
     */
    void resize(int width, int height) {
		m_res = glm::ivec2(width, height);
		m_res_changed = true;
		setup_textures();
		GLuint loc = glGetUniformLocation(m_texture_program->get_id(), "res");
		glUniform2f(loc, m_res.x, m_res.y);

    }

    /**
     * Handles input
     */
    void handle_input(ImGuiIO& io, float dt) {
	float speed = global_speed * dt;
	if (io.KeysDown[GLFW_KEY_W]) {
	    m_camera.move_forward(speed);
	}
	if (io.KeysDown[GLFW_KEY_S]) {
	    m_camera.move_backward(speed);
	}
	if (io.KeysDown[GLFW_KEY_A]) {
	    m_camera.move_left(speed);
	}
	if (io.KeysDown[GLFW_KEY_D]) {
	    m_camera.move_right(speed);
	}
	if (io.KeysDown[GLFW_KEY_E]) {
	    m_camera.roll(.1f * dt);
	}
	if (io.KeysDown[GLFW_KEY_Q]) {
	    m_camera.roll(-.1f * dt);
	}

	if (!io.WantCaptureMouse) {
		glm::vec2 new_mouse_pos = glm::vec2(float(io.MousePos.x) / m_res.x,
			float(io.MousePos.y) / m_res.y);
		if (io.MouseDown[0]) {
			if (new_mouse_pos.x >= 0.0f && new_mouse_pos.x <= 1.0f &&
				new_mouse_pos.y >= 0.0f && new_mouse_pos.y <= 1.0f) {
				glm::vec2 delta_mouse = new_mouse_pos - m_mouse_pos;
				delta_mouse *= M_PI;
				m_camera.rotate(delta_mouse);
			}
		}
		m_mouse_pos = new_mouse_pos;
	}
    }

    /**
     * Draw ImGui options window.
     */
    void draw_option_window() {
	/* Draw window containing sliders and stuff */
	ImGui::Begin("Info", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
		     ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Checkbox("Bloom", &m_should_blur);
	ImGui::Checkbox("Pause simulation", &m_pause_simulation);
	float gravity = m_particle_system.get_gravity();
	ImGui::SliderFloat("Gravity", &gravity, -2, 2);
	m_particle_system.set_gravity(gravity);
	auto pos = m_particle_system.get_position();
	ImGui::SliderFloat("Position x", &pos.x, -1, 1);
	m_particle_system.set_position(pos);
	ImGui::SliderFloat("Gamma", &m_gamma, 0.1f, 5.0f);
	ImGui::SliderFloat("Exposure", &m_exposure, 0.1f, 5.0f);

	auto res = m_particle_system.get_resistance();
	ImGui::SliderFloat("Resistance", &res, 0.0f, 1.0f);
	m_particle_system.set_resistance(res);

	auto noise_res = m_particle_system.get_noise_resolution();
	ImGui::SliderFloat("Noise resolution", &noise_res, 0.f, 60.f);
	m_particle_system.set_noise_resolution(noise_res);
	ImGui::Text("Particle Count: %u", m_particle_system.get_particle_count());
	auto max_ttl = m_particle_system.get_max_ttl();
	ImGui::SliderFloat("Max TTL", &max_ttl, 0.5f, 10.f);
	m_particle_system.set_max_ttl(max_ttl);

	int emission = m_particle_system.get_emission();
	ImGui::SliderInt("Emission", &emission, 1, 5000);
	m_particle_system.set_emission(emission);

	glm::vec3 start_color = m_particle_system.get_start_color();
	ImGui::ColorEdit3("start color", &start_color.x);
	m_particle_system.set_start_color(start_color);

	glm::vec3 end_color = m_particle_system.get_end_color();
	ImGui::ColorEdit3("end color", &end_color.x);
	m_particle_system.set_end_color(end_color);

	/* choose animation */
	bool active = m_animation == 0;
	if (ImGui::RadioButton("Sine", active)) {
	    m_animation = 0;
	}
	active = m_animation == 1;
	if (ImGui::RadioButton("Circle", active)) {
	    m_animation = 1;
	}
	active = m_animation == 4;
	if (ImGui::RadioButton("None", active)) {
	    m_animation = 4;
	}

	ImGui::End();
    }

    /**
     * Draw scene.
     */
    void draw(float dt) {
	draw_option_window();
	//Enable resize, additive alpha blending, point size
	glViewport(0, 0, m_res.x, m_res.y);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glEnable(GL_PROGRAM_POINT_SIZE);

	if (!m_pause_simulation) {
	    //m_particle_system.set_position(glm::vec3(.1*sin(m_time*3.), 0, .1*cos(m_time*3.)));
	    if (m_animation == 0) {
		m_particle_system.set_position(glm::vec3(sin(m_time*.7), 0., 0.));
	    } else if (m_animation == 1) {
		m_particle_system.set_position(glm::vec3(sin(m_time), cos(m_time), 0.));
	    }
	    m_particle_system.update(m_time, dt);
	}
	m_framebuffer->bind(); //g-buffer
	m_particle_system.draw(m_camera);
	m_framebuffer->unbind(); //unbind g-buffer

	/* Gaussian blur */
	if (m_should_blur) {
	    size_t work_groups_x = m_color_texture->get_width() / 8;
	    size_t work_groups_y = m_color_texture->get_height() / 8;;
	    //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	    glUseProgram(m_blur_program->get_id());
	    /* horizontal component of the blur */
	    glBindImageTexture(0, m_color_texture->get_id(), 0, GL_FALSE, 0, GL_READ_WRITE,
			       GL_RGBA32F);
	    glBindImageTexture(1, m_blur_texture->get_id(), 0, GL_FALSE, 0, GL_READ_WRITE,
			       GL_RGBA32F);
	    GLuint loc = m_blur_program->getUniformLocation("in_image");
	    glUniform1i(loc, 0);
	    loc = m_blur_program->getUniformLocation("out_image");
	    glUniform1i(loc, 1);
	    loc = m_blur_program->getUniformLocation("horizontal");
	    glUniform1i(loc, 1);
	    glDispatchCompute(work_groups_x, work_groups_y, 1);

	    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	    /* vertical blur */
	    loc = m_blur_program->getUniformLocation("horizontal");
	    glUniform1i(loc, 0);
	    glBindImageTexture(0, m_blur_texture->get_id(), 0, GL_FALSE, 0, GL_READ_WRITE,
	    		       GL_RGBA32F);
	    glBindImageTexture(1, m_blur_texture2->get_id(), 0, GL_FALSE, 0, GL_READ_WRITE,
			       GL_RGBA32F);
	    glDispatchCompute(work_groups_x, work_groups_y, 1);
	    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
	/* End Gaussian blur */

	/* Finalize the render, by combining everything */
	glDisable(GL_DEPTH_TEST);
	glUseProgram(m_texture_program->get_id());
	GLuint loc = m_texture_program->getUniformLocation("exposure");
	glUniform1f(loc, m_exposure);
	loc = m_texture_program->getUniformLocation("gamma");
	glUniform1f(loc, m_gamma);
	loc = m_texture_program->getUniformLocation("has_bloom");
	glUniform1i(loc, m_should_blur);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_color_texture->get_id());
	glActiveTexture(GL_TEXTURE0+1);
	glBindTexture(GL_TEXTURE_2D, m_blur_texture2->get_id());
	glBindVertexArray(m_plane_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	m_time += dt;
    }

};
