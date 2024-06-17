#version 330 core
#define Thickness 4.0

#ifdef GL_ES
precision mediump float;
#endif

in vec4 fragment_color;
uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;
uniform int is_point;
out float dist;
out vec4 fragColor;

void main() {
    if (is_point > 0) {
        float r = 0.0;
        vec2 cxy = 2.0 * gl_PointCoord - 1.0;
        r = dot(cxy, cxy);
        if (r > 1.0) {
            discard;
        }
    }
    fragColor = fragment_color;
}