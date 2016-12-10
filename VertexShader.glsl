#version 430
in vec3 vertex_position;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 worldMatrix;

void main()
{	
  vec4 world_pos = worldMatrix * vec4(vertex_position, 0.0f);
  vec4 view_pos = viewMatrix * world_pos;
  gl_Position = projectionMatrix * view_pos;
}

