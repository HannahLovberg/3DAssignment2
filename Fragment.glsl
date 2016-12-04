#version 400
in vec3 color;
out vec4 fragment_color;

uniform vec4 myColor;



void main () {

	fragment_color = vec4(color, 1.0f);
}