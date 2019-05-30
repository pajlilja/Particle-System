file (GLOB SHADERS  *.glsl)
foreach (loop_var ${SHADERS})
  get_filename_component(FILE_NAME ${loop_var} NAME)
  configure_file(${loop_var} ${PROJECT_BINARY_DIR}/shaders/${FILE_NAME} COPYONLY)
endforeach()

