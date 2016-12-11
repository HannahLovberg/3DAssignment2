#version 430
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;
layout(location = 2) in vec2 vertex_uv;

out vec4 geo_position;
out vec3 geo_color;
out vec2 geo_uv;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 worldMatrix;

void main()
{	
  geo_position = vec4(vertex_position, 1.0f);
  geo_color = vertex_color;
  geo_uv = vertex_uv;
}

