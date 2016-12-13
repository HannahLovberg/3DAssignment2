#version 430
layout(triangles) in;
layout(triangle_strip, max_vertices = 6) out;

in vec4 geo_position[];
in vec3 geo_color[];
in vec2 geo_uv[];

out vec3 fPosition;
out vec3 fColor;
out vec2 fuv;
out vec3 Normal;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 worldMatrix;

mat4 pvMatrix = projectionMatrix * viewMatrix;

vec3 getTriangleNormal(vec4 p0, vec4 p1, vec4 p2)
{
  vec3 normal;

  vec3 U = p1.xyz - p0.xyz;
  vec3 V = p2.xyz - p0.xyz;

  normal = cross(U,V);

  return normalize(normal);
  

}

void main()
{	
	
  Normal = getTriangleNormal(geo_position[0], geo_position[1], geo_position[2]);
  Normal = vec4(worldMatrix * vec4(Normal, 0.0f)).xyz;  

  for(int i=0; i<3; i++)
  {
    fPosition = (worldMatrix * geo_position[i]).xyz;
	fColor = geo_color[i];
	fuv = geo_uv[i];

	gl_Position = pvMatrix * vec4(fPosition, 1.0f);
	
    EmitVertex();
  }
  EndPrimitive();

  vec4 gPos;


  for(int i = 0; i < 3; i++)
  {
	gPos = geo_position[i];
	gPos.z -= 1.0f;

	fPosition = (worldMatrix * gPos).xyz;
	fColor = geo_color[i];
	fuv = geo_uv[i];

	gl_Position = pvMatrix * vec4(fPosition, 1.0f);
	
    EmitVertex();
  }
  EndPrimitive();

  
}  