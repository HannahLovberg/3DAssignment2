//--------------------------------------------------------------------------------------
// BTH - Stefan Petersson 2014.
//--------------------------------------------------------------------------------------
#include <windows.h>

#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>

#define GLFW_DLL
#include <gl/glew.h>
#include <gl/GL.h>

#include "glm-0.9.8.3\glm\glm\glm.hpp"
#include "glm-0.9.8.3\glm\glm\gtc\matrix_transform.hpp"
#include "glm-0.9.8.3\glm\glm\gtc\type_ptr.hpp"

#include"bth_image.h"

#include "Timer.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")

using namespace std;

HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HGLRC CreateOpenGLContext(HWND wndHandle);

GLuint gVertexBuffer = 0;
GLuint gVertexAttribute = 0;
GLuint gShaderProgram = 0;

Timer timer;

#define BUFFER_OFFSET(i) ((char *)nullptr + (i))


void getShaderError(GLuint shader)
{
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		GLint msgSize = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &msgSize);
		char* msg = new char[msgSize];
		glGetShaderInfoLog(shader, msgSize, nullptr, msg);
		glDeleteShader(shader);

		std::cout << msg << endl;
	}
}

void CreateShaders()
{
	//create vertex shader
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	// open glsl file and put it in a string
	ifstream shaderFile("VertexShader.glsl");
	std::string shaderText((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
	shaderFile.close();
	// make a double pointer (only valid here)
	const char* shaderTextPtr = shaderText.c_str();
	// ask GL to load this
	glShaderSource(vs, 1, &shaderTextPtr, nullptr);
	// ask GL to compile it
	glCompileShader(vs);
	getShaderError(vs);

	//create fragment shader | same process.
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	shaderFile.open("Fragment.glsl");
	shaderText.assign((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
	shaderFile.close();
	shaderTextPtr = shaderText.c_str();
	glShaderSource(fs, 1, &shaderTextPtr, nullptr);
	glCompileShader(fs);
	getShaderError(fs);

	//Geometry shader
	GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
	shaderFile.open("GeometryShader.glsl");
	shaderText.assign((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
	shaderFile.close();
	shaderTextPtr = shaderText.c_str();
	glShaderSource(gs, 1, &shaderTextPtr, nullptr);
	glCompileShader(gs);
	getShaderError(gs);

	//link shader program (connect vs and ps)
	gShaderProgram = glCreateProgram();
	glAttachShader(gShaderProgram, fs);
	glAttachShader(gShaderProgram, vs);
	glAttachShader(gShaderProgram, gs);
	glLinkProgram(gShaderProgram);



}



void CreateTriangleData()
{
	// this is how we will structure the input data for the vertex shader
	// every six floats, is one vertex.
	struct TriangleVertex
	{
		float x, y, z;
		float r, g, b;
		float texPos1, texPos2;
	};
	// create the actual data in plane Z = 0
	TriangleVertex triangleVertices[4] = 
	{
		// pos, color, texPos for each vertex
		{ -0.5f, 0.5f, 0.0f,	 1.0f, 0.0f, 0.0f,   0.0f, 1.0f }, //Top left
		{  0.5f, 0.5f, 0.0f,	 1.0f, 1.0f, 0.0f,	 1.0f, 1.0f }, //top right
		{ -0.5f, -0.5f, 0.0f,	 0.0f, 0.0f, 1.0f,   0.0f, 0.0f }, //bottom lef
		{  0.5f, -0.5f, 0.0f,	 0.0f, 1.0f, .0f,	 1.0f, 0.0f }  //bottom right
	};

	

	// Vertex Array Object (VAO) 
	glGenVertexArrays(1, &gVertexAttribute);
	glGenBuffers(1, &gVertexBuffer);

	glBindVertexArray(gVertexAttribute);

	// this activates the first and second attributes of this VAO
	glEnableVertexAttribArray(0); 
	glEnableVertexAttribArray(1);

	// create a vertex buffer object (VBO) id

	// Bind the buffer ID as an ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
	// This "could" imply copying to the GPU, depending on what the driver wants to do...
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

	// query where which slot corresponds to the input vertex_position in the Vertex Shader 
	GLuint vertexPos = glGetAttribLocation(gShaderProgram, "vertex_position");

	// specify that: the vertex attribute "vertexPos", of 3 elements of type FLOAT, not normalized, with STRIDE != 0,
	//               starts at offset 0 of the gVertexBuffer (it is implicitly bound!)
	glVertexAttribPointer(vertexPos, 3,    GL_FLOAT, GL_FALSE,     sizeof(TriangleVertex), BUFFER_OFFSET(0));

	// query where which slot corresponds to the input vertex_color in the Vertex Shader 
	GLuint vertexColor = glGetAttribLocation(gShaderProgram, "vertex_color");
	// specify that: the vertex attribute "vertex_color", of 3 elements of type FLOAT, not normalized, with STRIDE != 0,
	//               starts at offset (12 bytes) of the gVertexBuffer 
	glVertexAttribPointer(vertexColor, 3,    GL_FLOAT, GL_FALSE,     sizeof(TriangleVertex), BUFFER_OFFSET(sizeof(float)*3));

	//Texture
	glVertexAttribPointer(2, 2, GL_FLOAT,GL_FALSE, sizeof(TriangleVertex), BUFFER_OFFSET(sizeof(float)*6));
	glEnableVertexAttribArray(2);



	glBindVertexArray(0);


	


}

void SetViewport()
{
	glViewport(0, 0, 640, 480);
}

void Render()
{
	// set the color TO BE used
	glClearColor(0, 0, 0, 1);
	// use the color to clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(gShaderProgram);


	glBindVertexArray(gVertexAttribute);
	
	
	glDrawArrays(GL_POINTS, 0, 4);
	glBindVertexArray(0);
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa fönster
	
	if (wndHandle)
	{
		HDC hDC = GetDC(wndHandle);
		HGLRC hRC = CreateOpenGLContext(wndHandle); //2. Skapa och koppla OpenGL context

		glewInit(); //3. Initiera The OpenGL Extension Wrangler Library (GLEW)

		SetViewport(); //4. Sätt viewport

		CreateShaders(); //5. Skapa vertex- och fragment-shaders

		CreateTriangleData(); //6. Definiera triangelvertiser, 7. Skapa vertex buffer object (VBO), 8.Skapa vertex array object (VAO)

		ShowWindow(wndHandle, nCmdShow);

		///////View//////////
		glm::mat4 view = glm::lookAt(
			glm::vec3(0.0f, 0.0f, -2.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);



		///////////Projection/////////////
		GLfloat FoV = glm::pi<GLfloat>() * 0.45f;
		glm::mat4 projection = glm::perspective(
			FoV,
			(640.0f / 480.0f),
			0.1f,
			20.0f
		);

		glUseProgram(gShaderProgram);
		GLint projectionLocation0 = glGetUniformLocation(gShaderProgram, "projection");
		glUniformMatrix4fv(projectionLocation0, 1, GL_FALSE, glm::value_ptr(projection));



		//////// Textur /////////////

		GLuint texture;

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		//texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		//filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//load, create mipmap

		int width = 64;
		int height = 64;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, BTH_IMAGE_DATA);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);



		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{

				///////// view ////////
				glUseProgram(gShaderProgram);
				GLint viewLocation0 = glGetUniformLocation(gShaderProgram, "view");
				glUniformMatrix4fv(viewLocation0, 1, GL_FALSE, glm::value_ptr(view));

				/////////////Rotate/////////////////
				glm::mat4 world;
				world = glm::rotate(world, (GLfloat)timer.seconds(), glm::vec3(0.0f, -0.05f, 0.0f));

				glUseProgram(gShaderProgram);
				GLint transformLocation0 = glGetUniformLocation(gShaderProgram, "world");
				glUniformMatrix4fv(transformLocation0, 1, GL_FALSE, glm::value_ptr(world));
		
				glBindTexture(GL_TEXTURE_2D, texture);
				
				

				Render(); //9. Rendera

				SwapBuffers(hDC); //10. Växla front- och back-buffer
			}
		}

		wglMakeCurrent(NULL, NULL);
		ReleaseDC(wndHandle, hDC);
		wglDeleteContext(hRC);
		DestroyWindow(wndHandle);
	}

	return (int) msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.hInstance      = hInstance;
	wcex.lpszClassName = L"BTH_GL_DEMO";
	if( !RegisterClassEx(&wcex) )
		return false;

	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	
	HWND handle = CreateWindow(
		L"BTH_GL_DEMO",
		L"BTH OpenGL Demo",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	return handle;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message) 
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;		
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HGLRC CreateOpenGLContext(HWND wndHandle)
{
	//get handle to a device context (DC) for the client area
	//of a specified window or for the entire screen
	HDC hDC = GetDC(wndHandle);

	//details: http://msdn.microsoft.com/en-us/library/windows/desktop/dd318286(v=vs.85).aspx
	static  PIXELFORMATDESCRIPTOR pixelFormatDesc =
	{
		sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd  
		1,                                // version number  
		PFD_DRAW_TO_WINDOW |              // support window  
		PFD_SUPPORT_OPENGL |              // support OpenGL  
		PFD_DOUBLEBUFFER |                // double buffered
		PFD_DEPTH_DONTCARE,               // disable depth buffer <-- added by Stefan
		PFD_TYPE_RGBA,                    // RGBA type  
		32,                               // 32-bit color depth  
		0, 0, 0, 0, 0, 0,                 // color bits ignored  
		0,                                // no alpha buffer  
		0,                                // shift bit ignored  
		0,                                // no accumulation buffer  
		0, 0, 0, 0,                       // accum bits ignored  
		0,                                // 0-bits for depth buffer <-- modified by Stefan      
		0,                                // no stencil buffer  
		0,                                // no auxiliary buffer  
		PFD_MAIN_PLANE,                   // main layer  
		0,                                // reserved  
		0, 0, 0                           // layer masks ignored  
	};

	//attempt to match an appropriate pixel format supported by a
	//device context to a given pixel format specification.
	int pixelFormat = ChoosePixelFormat(hDC, &pixelFormatDesc);

	//set the pixel format of the specified device context
	//to the format specified by the iPixelFormat index.
	SetPixelFormat(hDC, pixelFormat, &pixelFormatDesc);

	//create a new OpenGL rendering context, which is suitable for drawing
	//on the device referenced by hdc. The rendering context has the same
	//pixel format as the device context.
	HGLRC hRC = wglCreateContext(hDC);
	
	//makes a specified OpenGL rendering context the calling thread's current
	//rendering context. All subsequent OpenGL calls made by the thread are
	//drawn on the device identified by hdc. 
	wglMakeCurrent(hDC, hRC);

	return hRC;
}