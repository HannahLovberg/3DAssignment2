//--------------------------------------------------------------------------------------
// BTH - Stefan Petersson 2014.
//--------------------------------------------------------------------------------------
#include <windows.h>

#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>

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

glm::vec3 lightPos(0.0f, 0.0f, -3.0f);

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

		std::wstring wmsg(strlen(msg), L'#');
		mbstowcs(&wmsg[0], msg, strlen(msg));
		MessageBox(0, wmsg.c_str(), L"Compile Error", MB_OK);
		delete[] msg;
		exit(-1);
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
	// every eight floats, is one vertex.
	struct TriangleVertex
	{
		float x, y, z;
		float r, g, b;
		float texPos1, texPos2;
	};
	// create the actual data in plane Z = 0
	TriangleVertex triangleVertices[4] = 
	{
		// pos, color, texPos for each vertex						pos					texPos
		{ -0.5f,  0.5f, 0.0f,	 1.0f, 0.0f, 0.0f,	0.0f, 0.0f }, //top left		-- bottom left
		{  0.5f,  0.5f, 0.0f,	 1.0f, 1.0f, 0.0f,	1.0f, 0.0f }, //top right		-- bottom right
		{ -0.5f, -0.5f, 0.0f,	 0.0f, 0.0f, 1.0f,  0.0f, 1.0f }, //bottom lef		-- top left
		{  0.5f, -0.5f, 0.0f,	 0.0f, 1.0f, 0.0f,	1.0f, 1.0f }  //bottom right	-- top right
	};

	

	// Vertex Array Object (VAO) 
	//Creates a new vertex array and sets ID to gVertexAttribute
	glGenVertexArrays(1, &gVertexAttribute);
	//Makes gVertexAttribute active 
	glBindVertexArray(gVertexAttribute);

	
	// create a vertex buffer object (VBO) id
	glGenBuffers(1, &gVertexBuffer);
	// Bind the buffer ID as an ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
	
	//Write the data from the array triangleVertices to the graphic card. 
	//GL_STATIC_DRAW says that the data won't change.
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

	
	//Position
	glEnableVertexAttribArray(0); //Enable first attribute in vertexArray
	glVertexAttribPointer(
		0,						//Attribute location in vertex shader
		3,    					//Number of values in buffer
		GL_FLOAT, 				//The value types are floats
		GL_FALSE,     			//Do not normalize the values
		sizeof(TriangleVertex), //Stride. The offset between each set of values
		BUFFER_OFFSET(0)		//Offset from the start of the buffer to the first set of values
	);

	//Color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex), BUFFER_OFFSET(sizeof(float)*3));

	//Texture
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex), BUFFER_OFFSET(sizeof(float)*6));


	//unbind vertex array to avoid accidents.
	glBindVertexArray(0);
}

void SetViewport()
{
	glViewport(0, 0, 640, 480);
}

void Render()
{
	// use the color to clear the color buffer AND reset the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Enable our shaders
	glUseProgram(gShaderProgram);

    //Activate our Vertex Array Object that holds all info about the vertices
	glBindVertexArray(gVertexAttribute);
	
	//Start at position 0 and draw 4 vertices from our VAO as a triangle strip
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    //Unbind
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

        //Allow the program to check if one shape if before the other
		glEnable(GL_DEPTH_TEST);
        // set the color TO BE used when clearing the window
	    glClearColor(0, 0, 0, 1);


		////////View////////
		glm::mat4 viewMatrix = glm::lookAt(
			glm::vec3(0.0f, 0.0f, -2.0f),   //Position of the camera
			glm::vec3(0.0f, 0.0f, 0.0f),    //Where the camera is pointed to
			glm::vec3(0.0f, 1.0f, 0.0f)     //What the UP in the world is
		);

        //Use our shaderprogram
		glUseProgram(gShaderProgram);
        //Find the location of our Uniform "viewMatrix"
		GLint viewLocation = glGetUniformLocation(gShaderProgram, "viewMatrix");
        //Set our Uniform "viewMatrix" in our shader program to the viewMatrix above
		glUniformMatrix4fv(             //set our Uniform data as a 4x4 float matrix
            viewLocation,               //The location of the uniform
            1,                          //Number of 4x4 matrices
            GL_FALSE,                   //Don't transpose matrix
            glm::value_ptr(viewMatrix)  //Location of the data
        );
        

		///////////Projection/////////////
		GLfloat FoV = glm::pi<GLfloat>() * 0.45f;
		glm::mat4 projectionMatrix = glm::perspective(
			FoV,                //Field of View
			(640.0f / 480.0f),  //Aspect ratio (windowWidth/windowHeight)
			0.1f,               //Front-clip plane. Closest distance to camera
			20.0f               //Back-clip plane. The depth of the FOV. Anything beyond this distance won't be rendered.
		);

        //Find the uniform location
		GLint projectionLocation = glGetUniformLocation(gShaderProgram, "projectionMatrix");
        //Set the uniform to the projectionMatrix data
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

        //Find the location of the lightPos Uniform
		GLuint lightPosLocation = glGetUniformLocation(gShaderProgram, "lightPos");
		glUniform3f(            //Set Uniform data as a vec3 with floats
            lightPosLocation,   //Location of the Uniform
            lightPos.x,         //the x-value of the uniform
            lightPos.y,         //the y-value of the uniform
            lightPos.z          //the z-value of the uniform
        );



		//////// Textur /////////////

		GLuint texture;

        //Create a Texture Object on the graphics card
		glGenTextures(1, &texture);
        //Bind our texture object as a 2D texture
		glBindTexture(GL_TEXTURE_2D, texture);
        
		//Texture parameters
        //S and T represents Width and Height
        //Sets how to handle if a texture coordinate is below 0 or above 1
        //Clamp to Border tells the texture to send a color value instead of the texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        
		//Filtering
        //MIN/MAG Filter tells which filter to use depending on if you're minimizing or magnifying the texture
        //The linear filter takes a combination of the colors in the pixel around the specified point
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
		glTexImage2D(           //Load texture data to graphics card
            GL_TEXTURE_2D,      //It's a 2D texture
            0,                  //Mipmap quality level. 0 is the highest quality
            GL_RGBA,            //What format we use to save the texture on the graphics card
            BTH_IMAGE_WIDTH,    //The texture's width
            BTH_IMAGE_HEIGHT,   //The texture's height
            0,                  //Legacy thingy
            GL_RGBA,            //Format of the source image
            GL_UNSIGNED_BYTE,   //Data type of the source
            BTH_IMAGE_DATA      //Location of the image data
        );
        
        //Create a mipmap
		glGenerateMipmap(GL_TEXTURE_2D);
        //Unbind texture object
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
				/////////////Rotation/////////////
				glm::mat4 worldMatrix;
				worldMatrix = glm::rotate(worldMatrix, (GLfloat)timer.seconds(), glm::vec3(0.0f, -0.05f, 0.0f));

				glUseProgram(gShaderProgram);
				GLint transformLocation0 = glGetUniformLocation(gShaderProgram, "worldMatrix");
				glUniformMatrix4fv(transformLocation0, 1, GL_FALSE, glm::value_ptr(worldMatrix));
		
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
		PFD_DOUBLEBUFFER, //  |                // double buffered
		//PFD_DEPTH_DONTCARE,               // disable depth buffer <-- added by Stefan
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
