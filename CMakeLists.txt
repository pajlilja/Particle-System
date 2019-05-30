cmake_minimum_required (VERSION 3.0)
project(engine)
set (CMAKE_CXX_STANDARD 17)

add_executable(cgi particle_system.cpp main.cpp)

if (UNIX)
  find_package( PkgConfig REQUIRED )
  pkg_search_module( GLFW3 REQUIRED glfw3)  
  set (OpenGL_GL_PREFERENCE "GLVND")
endif()
if(WIN32)  
  include_directories(${PROJECT_SOURCE_DIR}/lib/glfw/include)
  set (GLFW3_STATIC_LIBRARIES ${PROJECT_SOURCE_DIR}/lib/glfw/lib-vc2015/glfw3.lib)
endif()

find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
  include_directories(${OPENGL_INCLUDE_DIRS})
endif()

if (UNIX)
  find_package(GLEW REQUIRED)
  if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
  endif()
endif()
if (WIN32)
  include_directories(${PROJECT_SOURCE_DIR}/lib/glew)
  set (GLEW_LIBRARIES ${PROJECT_SOURCE_DIR}/lib/glew/glew32.lib)
endif()

include_directories(${PROJECT_SOURCE_DIR}/gl)
include_directories(lib/imgui)
include_directories(lib)

add_subdirectory(lib/imgui)

target_link_libraries(cgi
  ${GLFW3_STATIC_LIBRARIES}
  OpenGL::GL
  ${GLEW_LIBRARIES}
  imgui
  )

# Copy DLLs into the correct folder
if (WIN32)
  configure_file (${PROJECT_SOURCE_DIR}/lib/glew/glew32.dll
    ${PROJECT_BINARY_DIR}/glew32.dll
    COPYONLY )
endif()

add_subdirectory(shaders)

