#include "particle_system.hpp"

#include <numeric>

particle_system::particle_system() :
	m_curr_pos(0), m_prev_pos(0)
{
    /* TODO explain the the overall idea. */
    /* Setup programs */
    /* Emission program setup */
    auto emission_shader = shader::compile("shaders/cs_emission.glsl", GL_COMPUTE_SHADER);
    m_emission_program = program::create();
    m_emission_program->attachShader(emission_shader);
    m_emission_program->link();

    /* This is the shader responsible for updating the particle variables. */
    auto compute_shader = shader::compile("shaders/cs_particles.glsl", GL_COMPUTE_SHADER);
    m_compute_program = program::create();
    m_compute_program->attachShader(compute_shader);
    m_compute_program->link();

    /* Program used for drawing the particles. */
    auto vertex_shader = shader::compile("shaders/vs_particles.glsl", GL_VERTEX_SHADER);
    auto fragment_shader = shader::compile("shaders/fs_particles.glsl", GL_FRAGMENT_SHADER);
    m_draw_program = program::create();
    m_draw_program->attachShader(vertex_shader);
    m_draw_program->attachShader(fragment_shader);
    m_draw_program->link();

    /* Creating the buffer containing all the particles */
    glCreateVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glCreateBuffers(1, &m_particle_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_particle_buffer);
    std::vector<particle_format> particles(m_particle_count);
    srand(1337);
    for (auto &p : particles) {
	auto f_rand = []()->float {
	    return (float(rand()) / RAND_MAX)*2.0f - 1.0f;
	};
	p.velocity = glm::vec3(f_rand(), f_rand(), f_rand());
	p.position = glm::vec3(0.0f);
	p.ttl = f_rand() * m_ttl;
	p.start_ttl = p.ttl;
	p.start_velocity = p.velocity;
    }
    glBufferData(GL_ARRAY_BUFFER, m_particle_count * sizeof(particle_format),
		 particles.data(), GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_particle_buffer);

    /* Setup vertex attribs for drawing */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(particle_format), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(particle_format),
			  (GLvoid*)offsetof(particle_format, velocity));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(particle_format),
			  (GLvoid*)offsetof(particle_format, ttl));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(particle_format),
			  (GLvoid*)offsetof(particle_format, start_ttl));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* Create buffer containing all dead particles. We also initialize
       the buffer with all possible indexes. This could possibly be
       optimized, but at the moment we see no reason in doing it. */
    glCreateBuffers(1, &m_dead_buffer);
    std::vector<GLuint> indexes(m_particle_count);
    std::iota(indexes.begin(), indexes.end(), 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_dead_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_particle_count * sizeof(GLuint), indexes.data(),
	         GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_dead_buffer);
    /* Create buffer containing all alive particles */
    glCreateBuffers(1, &m_alive_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_alive_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_particle_count * sizeof(GLuint), (void*) 0,
	         GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_alive_buffer);
    /* When killing of particles this will create holes in our alive list,
       we therefore need a second alive list which is written to in the
       second step. */
    glCreateBuffers(1, &m_alive_buffer_prev);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_alive_buffer_prev);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_particle_count * sizeof(GLuint), (void*) 0,
		 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_alive_buffer_prev);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    /* We also need some atomic counters to keep track of how many
       particles are dead or alive */
    glCreateBuffers(1, &m_atomic_counter_buffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomic_counter_buffer);
    GLuint counters[] = {0, 0, GLuint(m_particle_count)};
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint) * 3, counters, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_atomic_counter_buffer);
}

glm::vec3 particle_system::get_position() {
	return m_curr_pos;
}

void particle_system::set_position(glm::vec3 pos) {
	m_prev_pos = m_curr_pos;
	m_curr_pos = pos;
}

/**
 * Updated particle system
 */
void particle_system::update(float time, float dt) {
    /* First pass is the emission pass */
    glUseProgram(m_emission_program->get_id());
    GLuint loc = glGetUniformLocation(m_emission_program->get_id(), "delta_time");
    glUniform1f(loc, dt);
    loc = glGetUniformLocation(m_emission_program->get_id(), "time");
    glUniform1f(loc, time);
    loc = glGetUniformLocation(m_emission_program->get_id(), "ttl");
    glUniform1f(loc, m_ttl);
    loc = glGetUniformLocation(m_emission_program->get_id(), "prev_position");
    glUniform3fv(loc, 1, &m_prev_pos.x);
    loc = glGetUniformLocation(m_emission_program->get_id(), "curr_position");
    glUniform3fv(loc, 1, &m_curr_pos.x);
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    glDispatchCompute(m_emission, 1, 1);
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomic_counter_buffer);
    glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
		       &m_alive_count);
    GLuint dead_count;
    glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint)*2, sizeof(GLuint),
		       &dead_count);
    /* The deadcount can overflow and then need to be reset */
    if (dead_count > m_particle_count) {
	dead_count = 0;
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint)*2, sizeof(GLuint),
			&dead_count);
    }
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    /* Second pass updates particle values and kill dead particles */
    glUseProgram(m_compute_program->get_id());
    loc = glGetUniformLocation(m_compute_program->get_id(), "delta_time");
    glUniform1f(loc, dt);
    loc = glGetUniformLocation(m_compute_program->get_id(), "time");
    glUniform1f(loc, time);
    loc = glGetUniformLocation(m_compute_program->get_id(), "ttl");
    glUniform1f(loc, m_ttl);
    loc = glGetUniformLocation(m_compute_program->get_id(), "gravity");
    glUniform1f(loc, m_gravity);
    loc = glGetUniformLocation(m_compute_program->get_id(), "prev_position");
    glUniform3fv(loc, 1, &m_prev_pos.x);
    loc = glGetUniformLocation(m_compute_program->get_id(), "curr_position");
    glUniform3fv(loc, 1, &m_curr_pos.x);
    loc = glGetUniformLocation(m_compute_program->get_id(), "resistance");
    glUniform1f(loc, m_resistance);
    loc = glGetUniformLocation(m_compute_program->get_id(), "noise_resolution");
    glUniform1f(loc, m_noise_resolution);
    glDispatchCompute(GLuint(std::ceil(double(m_alive_count)/128)) , 1, 1);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); //why this line?
    m_prev_pos = m_curr_pos;
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    /* Now we have to swap the new alive buffer with the old one*/
    std::swap(m_alive_buffer, m_alive_buffer_prev);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_alive_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_alive_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_alive_buffer_prev);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_alive_buffer_prev);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    /* We also have to swap counters and set prev to 0 */
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomic_counter_buffer);
    GLuint counters[3];
    glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint)*3, &counters);
    counters[0] = 0u;
    std::swap(counters[0], counters[1]);
    assert(counters[0] + counters[2] == m_particle_count);
    m_alive_count = counters[0];
    glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint)*3, &counters);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    /* we also have to set the current indices used for drawing to the new alive buffer */
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_alive_buffer);
    glBindVertexArray(0);

    /* This is stupid, we already have this because delta_time and time,
       lets call it an optimization. */
    loc = glGetUniformLocation(m_compute_program->get_id(), "prev_time");
    glUniform1f(loc, time);
}

/**
 * Draws the particle system to the current framebuffer
 */
void particle_system::draw(camera& cam) {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //bind vao
    glBindVertexArray(m_vao);
    //start shader program
    glUseProgram(m_draw_program->get_id());
    //Set camera matrix uniforms to vertex shader
    GLuint loc = glGetUniformLocation(m_draw_program->get_id(), "projection_matrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &cam.proj_matrix()[0][0]);
    loc = glGetUniformLocation(m_draw_program->get_id(), "view_matrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, &cam.view_matrix()[0][0]);
    loc = glGetUniformLocation(m_draw_program->get_id(), "start_color");
    glUniform3f(loc, m_start_color.x, m_start_color.y, m_start_color.z);
    loc = glGetUniformLocation(m_draw_program->get_id(), "end_color");
    glUniform3f(loc, m_end_color.x, m_end_color.y, m_end_color.z);
    //Draw particles, the alive buffer will be used as indices
    glDrawElements(GL_POINTS, m_alive_count, GL_UNSIGNED_INT, 0);
    //Unbind vao
    glBindVertexArray(0);
}
