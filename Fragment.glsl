#version 430

in vec3 fPosition;
in vec3 fColor;
in vec2 fuv;
in vec3 Normal;

uniform sampler2D sampler;

uniform vec3 lightPos;
uniform vec3 lightColor;


void main()
{
  float ambientStrength = 0.1f;
  vec3 ambient = ambientStrength * vec3(1.0f, 1.0f, 1.0f);

  vec3 norm = Normal;
  vec3 lightDir = normalize(lightPos - fPosition);
  float diff = max(dot(norm, lightDir), 0.0f);
  vec3 diffuse = diff * vec3(1.0f, 1.0f, 1.0f);

  vec3 result = (ambient + diffuse) *fColor;

  gl_FragColor = vec4(result, 1.0f) * texture(sampler, fuv);
}