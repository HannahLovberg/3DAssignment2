layout (square) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT 
{
	vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.4f;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

void GenerateLine(int index)
{
	gl_Position = gl_in[index].gl_Position;
	EmitVertex();
	gl_Position = gl[index].gl_Position + vec4(gs_in[index].normal, 0.0f) * MAGNITUDE;
	EmitVertex();
	EndPrimitive();
}

void main()
{
	GenerateLine(0);
	GenerateLine(1);
	GenerateLine(2);
	GenerateLine(3);

	gl_Position = projection * view * world * vec4(vertex_position, 1.0f);
}