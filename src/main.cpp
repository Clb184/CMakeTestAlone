
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include <wmmintrin.h>
#include <immintrin.h>
#include "MemManager.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "call.hpp"
#include "another_call.hpp"

#define LogInfo(x) PrintInfo(x)
#define LogError(x) PrintError(__FILE__, __LINE__, x)

#ifdef WIN32
#define LogVaStart __crt_va_start
#define LogVaEnd __crt_va_end
#elif defined __linux__
#define LogVaStart __builtin_va_start
#define LogVaEnd __builtin_va_end
#endif

inline void PrintInfo(const char* str) {
	printf("[GL INFO] - %s\n", str);
}

inline void PrintError(const char* src_file, int line, const char* str) {
	char* f = (char*)src_file + strlen(src_file);
	while (f >= src_file && !(*f == '\\' || *f == '/')) f--;
	f++;
	while (GLenum err = glGetError()) {
		printf("%s : %5d [GL ERROR (%d)] - %s\n", f, line, err, str);
	}
}

typedef struct tagFloat2 {
	float x = 0.0f;
	float y = 0.0f;
} float2, vec2;

typedef struct tagFloat3 {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
} float3, vec3;

typedef struct tagFloat4 {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 0.0f;
} float4, vec4;

// Transform and lit, textured vertex
struct TLVertex {
	float2 pos;
	float2 uv;
	GLuint color = 0xffffffff;
};

struct Matrix {
	__m128 m[4];
};

void MatrixIdentity(Matrix* pMatrix) {
	pMatrix->m[0] = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
	pMatrix->m[1] = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
	pMatrix->m[2] = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
	pMatrix->m[3] = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
}

__m128 MatrixVectorMultiply(Matrix* pMatrix, __m128 vec) {
	//__m128 stuff = DirectX::XMVector4Transform(vec, DirectX::XMMatrixIdentity());

	__m128
		vv0 = _mm_set_ps1(_mm_cvtss_f32(_mm_shuffle_ps(vec, vec, 0))),
		vv1 = _mm_set_ps1(_mm_cvtss_f32(_mm_shuffle_ps(vec, vec, 1))),
		vv2 = _mm_set_ps1(_mm_cvtss_f32(_mm_shuffle_ps(vec, vec, 2))),
		vv3 = _mm_set_ps1(_mm_cvtss_f32(_mm_shuffle_ps(vec, vec, 3)));

	__m128
		f0 = _mm_mul_ps(pMatrix->m[0], vv0),
		f1 = _mm_mul_ps(pMatrix->m[1], vv1),
		f2 = _mm_mul_ps(pMatrix->m[2], vv2),
		f3 = _mm_mul_ps(pMatrix->m[3], vv3);

	return _mm_add_ps(f0, _mm_add_ps(f1, _mm_add_ps(f2, f3)));
}

void MatrixMultiply(Matrix* pMat0, Matrix* pMat1) {
	 

}

const GLchar* g_VSSource =
"#version 460 core\n"
"layout(location = 0) in vec2 POSITION;\n"
"layout(location = 1) in vec2 TEXCOORDS;\n"
"layout(location = 2) in uint COLOR;\n"
"uniform mat4 g_CameraMatrix;\n"
"out vec4 uColor;\n"
"out vec2 tex_coords;\n"
"const float div = 1.0f / 255.0f;\n"
"void main() {\n"
"	uvec4 rgbai = (uvec4(COLOR) >> uvec4(24, 16, 8, 0)) & 0xff;\n"
//"	uColor = vec4(rgbai) * div;\n"
//"	uColor = texture(g_Sampler, TEXCOORDS);"
"	tex_coords = TEXCOORDS;\n"
"	gl_Position = g_CameraMatrix * vec4(POSITION, 0.0f, 1.0f);\n"
"}\n"
;

const GLchar* g_PSSource =
"#version 460 core\n"
"out vec4 COLOR;\n"
"in vec4 uColor;\n"
"in vec2 tex_coords;\n"
"uniform sampler2D g_Sampler;\n"
"void main() {\n"
"	COLOR = texture(g_Sampler, tex_coords);\n"
"}\n"
;

struct WindowData {
	GLFWwindow* pWindow = nullptr;
	int width = 0;
	int height = 0;
	const char* title = "";
	int num_samples = 0;
	bool fullscreen = false;
};

void ThisKeyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_TAB: {
			WindowData* pWindata;
			if (pWindata = (WindowData*)glfwGetWindowUserPointer(pWindow)) {
				GLFWmonitor* pMonitor = nullptr;
				if (!pWindata->fullscreen) {
					pMonitor = glfwGetPrimaryMonitor();
				}
				glfwSetWindowMonitor(pWindow, pMonitor, 0, 0 + 32, pWindata->width, pWindata->height, 60);
				pWindata->fullscreen = !pWindata->fullscreen;
			}
		} break;
		case GLFW_KEY_S: glfwWindowHint(GLFW_SAMPLES, 4); break;
		case GLFW_KEY_W: glfwWindowHint(GLFW_SAMPLES, 0); break;
		}
	}
}

GLint CreateTexture(void* data, GLsizei w, GLsizei h) {
	GLuint ret;
	glGenTextures(1, &ret);
	glBindTexture(GL_TEXTURE_2D, ret);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else {
		void* null_data = MemAlloc(w * h * 4);
		memset(null_data, 0xffffffff, w * h * 4);
		for (int i = 0; i < w * h; i++) {
			char dt = rand() % 255;
			int col = (0xff000000) | (dt << 16) | (dt << 8) | dt;
			((int*)null_data)[i] = col;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, null_data);
		MemFree(null_data);
	}
	return ret;
}

int main(void){
	const float scr_width = 640.0f, scr_height = 480.0f;
	
	WindowData windata;

	Matrix mat;
	MatrixIdentity(&mat);
	__m128 v0 = _mm_set_ps(1.0f, 0.0f, 10.0f, 20.0f);
	__m128 v1 = MatrixVectorMultiply(&mat, v0);

	// Start GLFW somehow
	if(!glfwInit()){
		printf("Failed initializing GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	windata.width = scr_width;
	windata.height = scr_height;
	windata.title = "Test GLFW3";
	windata.pWindow = glfwCreateWindow(windata.width, windata.height, windata.title, nullptr, nullptr);
	glfwSetKeyCallback(windata.pWindow, ThisKeyCallback);
	glfwSetWindowUserPointer(windata.pWindow, &windata);

	glfwMakeContextCurrent(windata.pWindow);

	// Start GLEW
	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit()) {
		printf("Failed initializing GLEW\n");
		return -1;
	}

	GLint ublockv, ublockp;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &ublockv);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &ublockp);

	// Configure viewport
	glEnable(GL_BLEND);
	glViewport(0, 0, scr_width, scr_height);

	// Create shaders (vertex and fragment)
	GLuint vshader = -1, fshader = -1, program = -1;
	GLint err = 0;

	vshader = glCreateShader(GL_VERTEX_SHADER);
	LogError("Error while creating Vertex Shader");
	glShaderSource(vshader, 1, &g_VSSource, NULL);
	glCompileShader(vshader);
	LogError("Error while compiling Vertex Shader");
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &err);
	if (!err) {
		GLchar buf[1024 + 1] = "";
		glGetShaderInfoLog(vshader, 1024, NULL, buf);
		printf("Vertex Shader compiling failed:\n%s\n", buf);
		return -1;
	}

	fshader = glCreateShader(GL_FRAGMENT_SHADER);
	LogError("Error while creating Fragment Shader");
	glShaderSource(fshader, 1, &g_PSSource, NULL);
	glCompileShader(fshader);
	LogError("Error while compiling Fragment Shader");
	glGetShaderiv(fshader, GL_COMPILE_STATUS, &err);
	if (!err) {
		GLchar buf[1024 + 1] = "";
		glGetShaderInfoLog(fshader, 1024, NULL, buf);
		printf("Fragment Shader compiling failed:\n%s\n", buf);
		return -1;
	}

	program = glCreateProgram();
	LogError("Error while creating Shader Program");
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);
	LogError("Error while linking Shader Program");
	glGetProgramiv(program, GL_LINK_STATUS, &err);
	if (!err) {
		GLchar buf[1024 + 1] = "";
		glGetProgramInfoLog(program, 1024, NULL, buf);
		printf("Shader Program linking failed:\n%s\n", buf);
		return -1;
	}

	glUseProgram(program);

	GLuint vertex = -1, array = -1;
	glGenVertexArrays(1, &array);
	glBindVertexArray(array);

	LogInfo("Creating Vertex Buffer");
	glCreateBuffers(1, &vertex);
	glBindBuffer(GL_ARRAY_BUFFER, vertex);
	//glBufferStorage(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, nullptr, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TLVertex) * 4, nullptr, GL_DYNAMIC_DRAW);
	LogError("Error while creating Vertex Buffer");

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TLVertex), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TLVertex), (void*)sizeof(float2));
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(TLVertex), (void*)(sizeof(float2) * 2));
	LogError("Error while setting Vertex Buffer attributes");

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	LogError("Error while enabling Vertex Buffer attributes");
	const float right = 0.0, left = scr_width, top = scr_height, bottom = 0.0, far = 100.0, near = 1.0;
	float matrix[4 * 4] = {
		1.0f / (left - right), 0.0, 0.0, 0.0f,//,
		0.0, 1.0f / (top - bottom), 0.0, 0.0f,//,
		0.0, 0.0, 1.0f / (far - near),   0.0,//,
		-((right + left) * 0.5f) / (left - right), -((top + bottom) * 0.5f) / (top - bottom), -((far + near) * 0.5f) / (far - near), 1.0f,
	};

	GLuint tex;
	tex = CreateTexture(nullptr, 16, 16);

	GLuint sampler;
	glGenSamplers(1, &sampler);
	glBindSampler(0, sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GLuint identity = glGetUniformLocation(program, "g_CameraMatrix");
	//glUniformMatrix4fv(identity, 1, GL_FALSE, (const GLfloat*)&ortho);
	glUniformMatrix4fv(identity, 1, GL_FALSE, (const GLfloat*)matrix);

	// cos -sin        x
	// sin  cos    *   y     x * cos + y * -sin  sin * x + cos * y
	float angle = 0.0f;
	const float tx  = scr_width * 0.5f, ty = scr_height * 0.5f;

	while(!glfwWindowShouldClose(windata.pWindow)){
		glfwPollEvents();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(program);
		glBindBuffer(GL_ARRAY_BUFFER, vertex);
		TLVertex* pVert = (TLVertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (pVert) {
			float c = cosf(angle), s = sinf(angle);
			//s = _mm_sincos_ps(&c, _mm_load_ps1(&angle));
			float cosine = (c), sine = (s);
			pVert[0] = { {tx + 120.0f * -sine, ty + cosine * 120.0f}, {0.0f, 0.0f},	0xff0000ff };
			pVert[1] = { {tx + -160.0f * cosine - sine * -120.0f, ty + sine * -160.0f + cosine * -120.0f}, {0.0f, 1.0f},	0xffff00ff };
			pVert[2] = { {tx + 160.0f * cosine - sine * -120.0f, ty + sine * 160.0f + cosine * -120.0f}, {1.0f, 1.0f},	0x0000ffff };

			glUnmapBuffer(GL_ARRAY_BUFFER);
			LogError("Error mapping buffer data");
		}
		LogError("Error while mapping Vertex Buffer");
		glDrawArrays(GL_TRIANGLES, 0, 3);
		LogError("Error while drawing triangle for test");
		glfwSwapBuffers(windata.pWindow);
		angle += 0.01f;
	}
	
	return 0;
}
