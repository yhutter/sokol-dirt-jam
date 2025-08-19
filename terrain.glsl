@ctype mat4 HMM_Mat4

@block noise_functions
// https://thebookofshaders.com/edit.php#11/2d-snoise-clear.frag

// Some useful functions
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }

//
// Description : GLSL 2D simplex noise function
//      Author : Ian McEwan, Ashima Arts
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License :
//  Copyright (C) 2011 Ashima Arts. All rights reserved.
//  Distributed under the MIT License. See LICENSE file.
//  https://github.com/ashima/webgl-noise
//
float snoise(vec2 v) {

    // Precompute values for skewed triangular grid
    const vec4 C = vec4(0.211324865405187,
                        // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,
                        // 0.5*(sqrt(3.0)-1.0)
                        -0.577350269189626,
                        // -1.0 + 2.0 * C.x
                        0.024390243902439);
                        // 1.0 / 41.0

    // First corner (x0)
    vec2 i  = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);

    // Other two corners (x1, x2)
    vec2 i1 = vec2(0.0);
    i1 = (x0.x > x0.y)? vec2(1.0, 0.0):vec2(0.0, 1.0);
    vec2 x1 = x0.xy + C.xx - i1;
    vec2 x2 = x0.xy + C.zz;

    // Do some permutations to avoid
    // truncation effects in permutation
    i = mod289(i);
    vec3 p = permute(
            permute( i.y + vec3(0.0, i1.y, 1.0))
                + i.x + vec3(0.0, i1.x, 1.0 ));

    vec3 m = max(0.5 - vec3(
                        dot(x0,x0),
                        dot(x1,x1),
                        dot(x2,x2)
                        ), 0.0);

    m = m*m ;
    m = m*m ;

    // Gradients:
    //  41 pts uniformly over a line, mapped onto a diamond
    //  The ring size 17*17 = 289 is close to a multiple
    //      of 41 (41*7 = 287)

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt(a0*a0 + h*h);
    m *= 1.79284291400159 - 0.85373472095314 * (a0*a0+h*h);

    // Compute final noise value at P
    vec3 g = vec3(0.0);
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * vec2(x1.x,x2.x) + h.yz * vec2(x1.y,x2.y);
    return 130.0 * dot(m, g);
}


// Taken from https://thebookofshaders.com/13/
float turbulence(vec2 x, float hurst_exponent, int num_octaves) {
    float g = exp2(-hurst_exponent);
    float f = 1.0f;
    float a = 1.0f;
    float t = 0.0f;
    for (int i = 0; i < num_octaves; i++) {
        t += a * abs(snoise(f * x));
        f *= 2.0;
        a *= g;
    }
    return t;
}
@end


@vs vs

@include_block noise_functions

#define NOISE_FUNC_TYPE_TURBULENCE 0
#define NOISE_FUNC_TYPE_SIMPLEX_NOISE 1

layout(binding=0) uniform vs_params {
    mat4 mvp;
    float amplitude;
    float hurst_exponent;
    int num_octaves;
    vec3 base_color;
    vec3 peak_color;
    int noise_func_type;
    float peak_color_threshold;
};

layout(location=0) in vec4 position;
out vec4 color;

void main() {
    float displacement = 0.0f;
    float mix_value = 0.0f;
    vec3 mix_color = vec3(0.0f);
    switch (noise_func_type) {
        case NOISE_FUNC_TYPE_TURBULENCE:
            displacement = turbulence(position.xz, hurst_exponent, num_octaves);
            mix_value = displacement;
            mix_color = mix(base_color, peak_color, smoothstep(mix_value, -peak_color_threshold, peak_color_threshold));
            break;
        case NOISE_FUNC_TYPE_SIMPLEX_NOISE: 
            // Simplex noise returns value from -1 to 1 -> remap to range 0 to 1.
            displacement = snoise(position.xz);
            mix_value = (displacement * 0.5f) + 0.5f;
            mix_color = mix(base_color, peak_color, mix_value);
            break;
    }

    displacement *= amplitude;
    vec3 displaced_position = vec3(position.x, position.y + displacement, position.z);

    gl_Position = mvp * vec4(displaced_position, 1.0);
    color = vec4(mix_color, 1.0f);
}
@end

@fs fs
in vec4 color;
out vec4 frag_color;

void main() {
    frag_color = color;
}
@end

@program terrain vs fs
