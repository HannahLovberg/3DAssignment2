layout (triangle) in;
layout (line_strip, max_vertices = 4) out;



void main()
{
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	EndPrimitive();
}