#version 330 core

in vec2 UV;
in vec4 gl_FragCoord;
layout(location = 0) out vec3 color;

uniform sampler2D renderedTexture;
uniform vec2 coord;
uniform vec3 colour;
uniform float radius;
uniform float radius_squared;

void main(void) {
    float dx = gl_FragCoord.x - coord.x;
    float dy = gl_FragCoord.y - coord.y;
    float dist_squared = (dx * dx) + (dy * dy);
    float raw = dist_squared > 0 ? radius_squared / dist_squared : 99999999999999.f;
    float i = min(1, raw);
    float q = (raw - i) * 2;
    float b = radius / pow(dist_squared, 0.5 + dist_squared / 5000000);
    vec3 source = texture2D(renderedTexture, UV).rgb;
    color = vec3(
        min(1, source.r + min(1, colour.r + q) * b),
        min(1, source.g + min(1, colour.g + q) * b),
        min(1, source.b + min(1, colour.b + q) * b)
    );
}