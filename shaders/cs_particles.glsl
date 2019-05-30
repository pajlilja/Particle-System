#version 450

uniform float delta_time;
uniform float time;
uniform float ttl;
uniform vec3 prev_position;
uniform vec3 curr_position;
uniform float prev_time;

uniform float gravity;
uniform float resistance;
uniform float noise_resolution;

struct particle {
  vec3 position;
  vec3 velocity;
  vec3 start_velocity;
  float ttl;
};

layout (std140, binding = 1) buffer particle_storage {
  particle particles[];
};

layout (std430, binding = 2) buffer dead_list {
  uint dead[];
};

layout (std430, binding = 3) buffer alive_list {
  uint alive[];
};

layout (std430, binding = 4) buffer alive_list_prev {
  uint alive_prev[];
};

layout (binding = 0, offset = 0) uniform atomic_uint alive_count;
layout (binding = 0, offset = 4) uniform atomic_uint alive_count_prev;
layout (binding = 0, offset = 8) uniform atomic_uint dead_count;

layout (local_size_x = 128) in;


/**
 * The code is taken from Stefan Gustavssons github.
*/
//
// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
//
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
  }
/* End of borrowed code */


  // Found in a scientifically approved paper
vec3 curl_noise(vec3 p) {
  vec2 e = vec2(0.0, 0.001);
  float c1 = (snoise(p + 10000.0 + e.xyx) - snoise(p + 10000.0)) -
             (snoise(p + 100.0 + e.xxy) - snoise(p + 100.0));
  float c2 = (snoise(p + e.xxy) - snoise(p)) -
             (snoise(p + 10000.0 + e.yxx) - snoise(p + 10000.0));
  float c3 = (snoise(p + 100.0 + e.yxx) - snoise(p + 100.0)) -
             (snoise(p + e.xyx) - snoise(p));
  return vec3(c1, c2, c3)/e.y;

}

void main(){
  uint g_idx = atomicCounterDecrement(alive_count);
  /* If underflow occur we know that this thread should be killed*/
  if (g_idx > uint(10000000)) {
    return;
  }

  uint idx = alive[g_idx];

  /* Update particle */
  particles[idx].position += particles[idx].velocity * delta_time;
  particles[idx].ttl -= delta_time;
  vec3 p = particles[idx].position;
  particles[idx].velocity.y -= gravity*delta_time;
  particles[idx].velocity -= curl_noise(p*noise_resolution + time * 0.4)*delta_time*0.2;
  particles[idx].velocity *= resistance;
  particles[idx].position = p;
  float d = length(p);

  /* kill particle if ttl is 0 or less */
  if(particles[idx].ttl <= 0.0) {
    dead[atomicCounterIncrement(dead_count)] = idx;
  } else {
    alive_prev[atomicCounterIncrement(alive_count_prev)] = idx;
  }

}
