#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;

uniform sampler2D renderedTexture;
uniform float seed;

float rand(vec2 seed) {
    return fract(sin(dot(seed.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

void main(void) {
    vec2 seed_vec = UV;
    seed_vec.x *= seed;
    seed_vec.y *= seed;
    float x = rand(seed_vec);
    if(x < 0.999) {
        color = vec3(0, 0, 0); //texture2D(renderedTexture, UV).rgb;
    } else {
        color = vec3(x, x, x);
    }
}