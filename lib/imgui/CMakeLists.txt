set (IMGUI_SOURCE_FILES
  imgui.cpp
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_widgets.cpp
  imgui_impl_glfw_gl3.cpp
  )
add_library(imgui STATIC ${IMGUI_SOURCE_FILES})
