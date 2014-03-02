#version 330 core

in vec2 UV;
in vec4 gl_FragCoord;
layout(location = 0) out vec3 color;

uniform sampler2D renderedTexture;
uniform vec2 coord;
uniform float radius;

void main(void) {
    float dx = gl_FragCoord.x - coord.x;
    float dy = gl_FragCoord.y - coord.y;
    float dist_squared = (dx * dx) + (dy * dy);
    float b = radius / pow(dist_squared, 0.5 + dist_squared / 10000);
    vec3 newcol = vec3(b, b, b);
    color = texture2D(renderedTexture, UV).rgb + newcol;
}