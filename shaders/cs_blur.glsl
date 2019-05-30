#version 430 core

uniform bool horizontal;
layout (rgba32f) uniform image2D in_image;
layout (rgba32f) uniform image2D out_image;

// #define KERNEL_SIZE 11
// float kernel[KERNEL_SIZE] = {0.000003, 0.000229, 0.005977, 0.060598, 0.24173, 0.382925,
// 			     0.24173, 0.060598, 0.005977, 0.000229, 0.000003};

#define KERNEL_SIZE 25
float kernel[KERNEL_SIZE] = {0.000048, 0.000169, 0.000538, 0.001532, 0.003907, 0.008921,
			     0.018247, 0.033432, 0.054867, 0.080658, 0.106212, 0.125283,
			     0.132372, 0.125283, 0.106212, 0.080658, 0.054867, 0.033432,
			     0.018247, 0.008921, 0.003907, 0.001532, 0.000538, 0.000169,
			     0.000048};

layout (local_size_x = 8, local_size_y = 8) in;
void main() {
  ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

  vec3 sum = vec3(0);
  for (int i = -KERNEL_SIZE/2; i < KERNEL_SIZE/2; i++) {
    float w = kernel[i + KERNEL_SIZE / 2];
    ivec2 p = ivec2(i, 0);
    p = horizontal ? p : p.yx;
    p = pixel + p;
    p = clamp(p, ivec2(0), imageSize(in_image).xy);

    sum += imageLoad(in_image, p).xyz * w;
  }
  //if (pixel.x < imageSize(in_image).x && pixel.y < imageSize(in_image).y)
    imageStore(out_image, pixel, vec4(sum, 1));
}
