layout (triangle) in;
layout (triangle_strip, max_vertices = 4) out;



void main()
{
	
	gl_Position = gl_in[0].gl_Position + vec4( -0.5f,  0.5f, 1.0f, .0f);
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + vec4(  0.5f,  0.5f, 1.0f, .0f);
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + vec4( -0.5f, -0.5f, 1.0f, .0f);
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + vec4(  0.5f, -0.5f, 1.0f, .0f);
	EmitVertex();

	EndPrimitive();

}