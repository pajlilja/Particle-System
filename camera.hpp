#pragma once

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

class camera {

private:
    glm::vec3 m_camera_pos;
    glm::vec3 m_camera_lookat;
    glm::vec3 m_camera_up;
    float m_fov = 45.0f;
    float m_aspect_ratio = 16.0f / 9.0f;

public:
    camera() {
	m_camera_pos = glm::vec3(0.0f, 0.0f, 1.0f);
	m_camera_lookat = glm::vec3(0.0f, 0.0f, -1.0f);
	m_camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    camera(glm::vec3 pos, glm::vec3 lookat, glm::vec3 up) {
	m_camera_pos = pos;
	m_camera_lookat = lookat;
	m_camera_up = up;
    }

    void move_forward(float dist) {
	m_camera_pos += m_camera_lookat * dist;
    }
    void move_backward(float dist) {
	m_camera_pos -= m_camera_lookat * dist;
    }
    void move_right(float dist) {
	auto camera_right = glm::cross(m_camera_lookat, m_camera_up);
	m_camera_pos += camera_right * dist;
    }
    void move_left(float dist) {
	auto camera_right = glm::cross(m_camera_lookat, m_camera_up);
	m_camera_pos -= camera_right * dist;
    }
    void rotate(glm::vec2 delta_mouse) {
	auto camera_right = glm::cross(m_camera_lookat, m_camera_up);
	m_camera_up = glm::rotate(m_camera_up, delta_mouse.y, camera_right);
	m_camera_lookat = glm::rotate(m_camera_lookat, delta_mouse.y, camera_right);
	m_camera_lookat = glm::rotate(m_camera_lookat, delta_mouse.x, m_camera_up);
    }
    void move(glm::vec3 vec) {
	m_camera_pos += vec;
    }

    void roll(float angle) {
	m_camera_up = glm::rotate(glm::normalize(m_camera_lookat), angle,
				  glm::normalize(m_camera_up));
    }
    glm::mat4x4 proj_matrix() {
	return glm::perspective(m_fov, m_aspect_ratio, 0.1f, 20.0f);
    }
    glm::mat4x4 view_matrix() {
	return glm::lookAt(m_camera_pos, m_camera_pos + m_camera_lookat, m_camera_up);
    }
};
