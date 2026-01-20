#ifndef __MY_PLANE_H__
#define __MY_PLANE_H__

#include <vgl.h>
#include <vec.h>
#include <InitShader.h>
struct MyPlaneVertex {//My Plane's pos and color
	vec4 position;
	vec4 color;
};


class MyPlane {
public:
	int m_div;//num of division(min = 2)
	int m_triangles;//m_div^2*2
	int m_numVertices;//m_triangles*3

	GLuint vao;
	GLuint vbo;
	GLuint prog;

	bool bInitialized;

	MyPlane() {
		m_div = 30;//default 30
		m_triangles = m_div * m_div * 2;
		m_numVertices = m_triangles * 3;
		bInitialized = false;
	}
	void init(int div) {
		m_div = div;
		m_triangles = m_div * m_div * 2;
		m_numVertices = m_triangles * 3;
		printf("Division: %d, Num.of Triangles : %d, Num.of Vertices : %d\n", m_div, m_triangles, m_numVertices);
		MyPlaneVertex* data = new MyPlaneVertex[m_numVertices];

		float width = 1.5f;
		int curIdx = 0;
		vec4 cur_color;
		for (int i = 0; i < m_div; i++) {
			for (int j = 0; j < m_div; j++) {
				if ((i + j) % 2 == 0)
					cur_color = vec4(0.6, 0.6, 0.6, 1);   //dark grey
				else
					cur_color = vec4(0.7, 0.7, 0.7, 1); //light grey

				float x1 = -width / 2.0f + width / m_div * i;
				float y1 = -width / 2.0f + width / m_div * j;
				float x2 = -width / 2.0f + width / m_div * (i + 1);
				float y2 = -width / 2.0f + width / m_div * (j + 1);

				vec4 a = vec4(x1, y1, 0, 1);
				vec4 b = vec4(x1, y2, 0, 1);
				vec4 c = vec4(x2, y2, 0, 1);
				vec4 d = vec4(x2, y1, 0, 1);

				data[curIdx].position = a;		data[curIdx].color = cur_color;		curIdx++;
				data[curIdx].position = b;		data[curIdx].color = cur_color;		curIdx++;
				data[curIdx].position = c;		data[curIdx].color = cur_color;		curIdx++;

				data[curIdx].position = c;		data[curIdx].color = cur_color;		curIdx++;
				data[curIdx].position = d;		data[curIdx].color = cur_color;		curIdx++;
				data[curIdx].position = a;		data[curIdx].color = cur_color;		curIdx++;
			}
		}


		// 1. send data to GPU
		sendDataToGPU(data);

		// 2. load shaders
		if(!bInitialized) prog = InitShader("vshader.glsl", "fshader.glsl");
		glUseProgram(prog);

		connectDataToShader();

		delete[]data;
		bInitialized = true;
	}
	void sendDataToGPU(MyPlaneVertex *data) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glBufferData(GL_ARRAY_BUFFER,
			sizeof(MyPlaneVertex) * m_numVertices, data, GL_STATIC_DRAW);
	}
	void draw(float g_Time, bool g_bWave) {

		glBindVertexArray(vao);
		glUseProgram(prog);
		connectDataToShader();

		GLuint uTime = glGetUniformLocation(prog, "uTime");
		glUniform1f(uTime, g_Time);
	
		GLuint uWavePlay = glGetUniformLocation(prog, "uWavePlay");
		glUniform1f(uWavePlay, g_bWave);

		glDrawArrays(GL_TRIANGLES, 0, m_numVertices);

	}
	void connectDataToShader() {
		// 3. connect data to shader
		GLuint vPosition = glGetAttribLocation(prog, "vPosition");
		glEnableVertexAttribArray(vPosition);
		glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, (sizeof(MyPlaneVertex)), BUFFER_OFFSET(0));

		GLuint vColor = glGetAttribLocation(prog, "vColor");
		glEnableVertexAttribArray(vColor);
		glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, (sizeof(MyPlaneVertex)), BUFFER_OFFSET(sizeof(vec4)));
		
	}
};


#endif
