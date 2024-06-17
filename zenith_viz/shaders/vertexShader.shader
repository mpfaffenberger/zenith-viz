#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertex_color;

uniform vec3 picking_point;
uniform mat4 MVP;
uniform vec4 color;
uniform float point_size;
uniform float use_color_data;

out vec4 fragment_color;

void main() {

    gl_Position =  MVP * vec4(vertexPosition_modelspace, 1);
    gl_PointSize = point_size;

    if (use_color_data > 0.0f) {
        fragment_color.xyz = vertex_color;
        fragment_color.w = color.w;
    } else {
        fragment_color = color;
        float eps = 1e-3;
        float picking_dist = distance(picking_point, vertexPosition_modelspace.xyz);
        if (picking_dist < eps) {
            fragment_color.x = 0.8;
            fragment_color.y = 0.8;
            fragment_color.z = 0.0;
            fragment_color.w = 1.0;
        }
    }
}