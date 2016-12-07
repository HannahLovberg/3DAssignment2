#version 400
in vec3 color;
in vec2 TexCoord;

out vec4 fragment_color;

uniform sampler2D ourTexture; 

void main () {
	
	fragment_color = texture(ourTexture, TexCoord) * vec4(color, 1.0f);
}