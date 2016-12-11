#version 430

in vec3 fPosition;
in vec3 fColor;
in vec2 fuv;

uniform sampler2D sampler;


void main()
{
  gl_FragColor = vec4(fColor, 1.0) * texture(sampler, fuv);
}