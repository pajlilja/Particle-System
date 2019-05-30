#include <stdlib.h>
#include <stdio.h>
#include <functional>
#include <iostream>

#include <ogl.h>
#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#include "app.hpp"
#include "framebuffer.hpp"
#include "texture.hpp"

#define FULLSCREEN 0
#define FPS 60

int screen_width;
int screen_height;
bool window_resized = false;

void resize_callback(GLFWwindow* window, int width, int height) {
	screen_height = height;
	screen_width = width;
	window_resized = true;
}

void error_callback(int error, const char* description) {
  fprintf(stderr, "Error: %s\n", description);
  exit(1);
}

void debugCallbackFun(GLenum source, GLenum type, GLuint id, GLenum severity,
                      GLsizei length,const GLchar *message, const void *userParam) {
    puts(message);
}

int main(int argc, char *argv[]) {
  int frame;
  double prev_time;
  double curr_time;
  GLFWwindow *window;

  if (!glfwInit()) {
    fprintf(stderr, "glfw failed to initialize\n");
    exit(1);
  }
  glfwSetErrorCallback(error_callback);


  glfwInit();
 
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);

  int width = 1920;
  int height = 1080;
 
  window = glfwCreateWindow(width, height, "window",
	  FULLSCREEN ? glfwGetPrimaryMonitor() : NULL, NULL);  


  if (!window) {
    fprintf(stderr, "Failed to create window!\n");
    exit(1);
  }

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  GLenum err = glewInit();
  if (err != GLEW_OK) {
      fprintf(stderr, "GLEW failed to initialize!\n");
      exit(1);
  }

  printf("Using GL version: %s\n", glGetString(GL_VERSION));
  printf("Using GLSL version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  printf("%dfps, %dx%d, %s\n", FPS, width, height,
    FULLSCREEN ? "fullscreen" : "windowed");

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(debugCallbackFun, NULL); 
  glfwSetWindowSizeCallback(window, resize_callback);

  app m_app(glm::ivec2(width, height));


  /* Setup ImGUI */
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  ImGui_ImplGlfwGL3_Init(window, true);
  ImGui::StyleColorsDark();

  /* setup framebuffer */
  // auto fb_color_texture = texture::genTexture(GL_TEXTURE_2D);
  // fb_color_texture->texImage2D(0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT,
  //                              nullptr);
  // fb_color_texture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // fb_color_texture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // auto fb_depth_texture = texture::genTexture(GL_DEPTH_ATTACHMENT);
  // fb_depth_texture->texImage2D(0, GL_RGBA16F, width, height, 0, GL_DEPTH_COMPONENT,
  //                              GL_FLOAT, nullptr);
  // fb_depth_texture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // fb_depth_texture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // auto fb = framebuffer::genFramebuffer();
  // fb->framebufferTexture(fb_color_texture, GL_COLOR_ATTACHMENT0);
  // fb->framebufferTexture(fb_depth_texture, GL_DEPTH_ATTACHMENT);

  // if (GL_FRAMEBUFFER_COMPLETE != fb->checkFramebufferStatus()) {
  //     fprintf(stderr, "Framebuffer is not complete\n");
  //     exit(0);
  // }
  

  /* game loop */
  frame = 0;
  prev_time = -1.0;
  glfwSetTime(0.0);
  while (!glfwWindowShouldClose(window)) {
      curr_time = glfwGetTime();
      if (curr_time - prev_time >= 1.0 / FPS) {
          glfwPollEvents();
          ImGui_ImplGlfwGL3_NewFrame();
          
          glClearColor(0, 0, 0, 1);
          glClear(GL_COLOR_BUFFER_BIT);
		  if (window_resized) {
			  m_app.resize(screen_width, screen_height);
			  window_resized = false;
		  }
          //fb->bind();
		  m_app.handle_input(io, curr_time-prev_time);
          m_app.draw(curr_time - prev_time);
          //fb->unbind();

          //ImTextureID texID = (ImTextureID) fb_color_texture->get_id();
          
          ImGui::Render();
          ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
      
          glfwSwapBuffers(window);
          if (!(frame % 60)) {
              printf("%d frames rendered, %f s elapsed\n", frame, curr_time);
          }
          frame++;
          prev_time = curr_time; 
      }
  }

  
  ImGui_ImplGlfwGL3_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
  
  return 0;
}
