#define _CRT_SECURE_NO_WARNINGS
#include <vgl.h>
#include <InitShader.h>
#include "MyCube.h"

#include "MyOBJRenderer.h"

#include <vec.h>
#include <mat.h>

MyOBJModel myModel;

MyCube cube;


GLuint program;
GLuint prog_phong;

GLuint uMat;
GLuint uColor;
GLuint uShininess;
GLuint uSpecularEffect;

mat4 g_Mat = mat4(1.0f);

int winWidth = 512;
int winHeight = 512;

float g_Shininess = 20.0f;
vec4 g_SpecularEffect = vec4(vec3(0.5f), 1.0f);

float g_Time = 0;
bool g_Rotate = false;

int g_AxisMode = 1; // 1:X, 2:Y, 3:Z
mat4 g_RotationMat = mat4(1.0f);

mat4 myLookAt(vec3 eye, vec3 at, vec3 up)
{
	
	mat4 V(1.0f);
	vec3 n = at-eye;
	n /= length(n);

	float a = dot(up, n);
	vec3 v = up - a*n;
	v /= length(v);

	vec3 w = cross(n, v);

	mat4 Rw(1.0f);

	Rw[0][0] = w.x;	Rw[0][1] = v.x; Rw[0][2] = -n.x;
	Rw[1][0] = w.y;	Rw[1][1] = v.y; Rw[1][2] = -n.y;
	Rw[2][0] = w.z;	Rw[2][1] = v.z; Rw[2][2] = -n.z;

	mat4 Rc(1.0f);
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			Rc[i][j] = Rw[j][i];

	mat4 Tc = Translate(-eye.x, -eye.y, -eye.z);

	V = Rc*Tc;
		
	return V;
}



mat4 myPerspective(float fovy, float aspectRatio, float zNear, float zFar)
{
	mat4 P(1.0f);
	
	float rad = fovy * 3.141592 / 180.0;
	
	float sz = 1 / zFar;
	float h = zFar * tan(rad/2);
	
	float sy = 1/h;
	float w = h*aspectRatio;
	float sx = 1/w;

	mat4 S = Scale(sx, sy, sz);
	mat4 M(1.0f);

	float c = -zNear / zFar;
	M[2][2] = 1/(c+1);
	M[2][3] = -c/(c+1);
	M[3][2] = -1;
	M[3][3] = 0;

	P = M*S;

	return P;
}


void myInit()
{
    cube.Init();
   
    myModel.InitModel();

    program = InitShader("vshader.glsl", "fshader.glsl");
    prog_phong = InitShader("vphong.glsl", "fphong.glsl");
}

void DrawAxis(mat4 RotMat)
{
    glUseProgram(program);
    mat4 x_a = RotMat * Translate(0.75, 0, 0) * Scale(1.5, 0.01, 0.01);
    glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * x_a);
    glUniform4f(uColor, 1, 0, 0, 1);
    cube.Draw(program);

    mat4 y_a = RotMat * Translate(0, 0.75, 0) * Scale(0.01, 1.5, 0.01);
    glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * y_a);
    glUniform4f(uColor, 0, 1, 0, 1);
    cube.Draw(program);

    mat4 z_a = RotMat * Translate(0, 0, 0.75) * Scale(0.01, 0.01, 1.5);
    glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * z_a);
    glUniform4f(uColor, 0, 0, 1, 1);
    cube.Draw(program);
}

void display()
{
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    mat4 ViewMat = myLookAt(vec3(0, 0, 3.5), vec3(0, 0, 0), vec3(0, 1, 0));
    float aspect = (float)winWidth / (float)winHeight;
    mat4 ProjMat = myPerspective(40, aspect, 0.1, 100);

    g_Mat = ProjMat * ViewMat;

    // 1. Axis
    glUseProgram(program);
    uMat = glGetUniformLocation(program, "uMat");
    uColor = glGetUniformLocation(program, "uColor");
    DrawAxis(g_RotationMat);

    // 2. OBJ
    glUseProgram(prog_phong);
    GLuint uProjMat = glGetUniformLocation(prog_phong, "uProjMat");
    GLuint uModelMat = glGetUniformLocation(prog_phong, "uModelMat");

    uShininess = glGetUniformLocation(prog_phong, "uShininess");
    uSpecularEffect = glGetUniformLocation(prog_phong, "uSpecularEffect");

    glUniform1f(uShininess, g_Shininess);
    glUniform4fv(uSpecularEffect, 1, (GLfloat*)&g_SpecularEffect);

    
    mat4 ObjBaseMat = myModel.InitModelScaleNPos();


    glUniformMatrix4fv(uProjMat, 1, GL_TRUE, ProjMat);
    glUniformMatrix4fv(uModelMat, 1, GL_TRUE, ViewMat * g_RotationMat * ObjBaseMat);

    myModel.Draw(prog_phong);

    glutSwapBuffers();
}

void idle()
{
    if (g_Rotate) {
        float speed = 1.5f; // 회전 속도
        mat4 deltaRot = mat4(1.0f);

        
        if (g_AxisMode == 1) deltaRot = RotateX(speed);
        else if (g_AxisMode == 2) deltaRot = RotateY(speed);
        else if (g_AxisMode == 3) deltaRot = RotateZ(speed);

        g_RotationMat = deltaRot * g_RotationMat;
    }

    Sleep(16);
    glutPostRedisplay();
}
void reshape(int w, int h) {
    winWidth = w; 
    winHeight = h;
    glViewport(0, 0, w, h);
    glutPostRedisplay(); }

void MyKeyboard(unsigned char key, int x, int y)//ks와sh 조작 부분
{
    switch (key) {
    case '1': printf("Using Vertex Normal!\n"); myModel.SetShadingMode(1); break;
    case '2': printf("Using Surface Normal!\n"); myModel.SetShadingMode(2); break;
    case '3':
        g_SpecularEffect.x += 0.1f; g_SpecularEffect.y += 0.1f; g_SpecularEffect.z += 0.1f;
        if (g_SpecularEffect.x > 5.0f) g_SpecularEffect = vec4(5.0f);
        printf("Increasing Specular Effect!\n");
        break;
    case '4':
        g_SpecularEffect.x -= 0.1f; g_SpecularEffect.y -= 0.1f; g_SpecularEffect.z -= 0.1f;
        if (g_SpecularEffect.x < 0.0f) g_SpecularEffect = vec4(0.0f);
        printf("Decreasing Specular Effect!\n");
        break;
    case '5': g_Shininess += 0.5f; if (g_Shininess > 40.0f)g_Shininess = 40.0f; printf("Increasing Shininess!\n"); break;
    case '6': g_Shininess -= 0.5f; if (g_Shininess < 4.f) g_Shininess = 4.f; printf("Decreasing Shininess!\n"); break;
    case ' ':if (g_Rotate)printf("Stop!\n"); else printf("Play!\n"); g_Rotate = !g_Rotate; break;

    case 'q': exit(0); break;
    }
    glutPostRedisplay();
}


void MyMouse(int m, int s, int x, int y)
{
    if (s == GLUT_DOWN) {
        if (m == GLUT_LEFT_BUTTON) {
            g_AxisMode = 1; //X축
        }
        else if (m == GLUT_MIDDLE_BUTTON) {
            g_AxisMode = 2; // Y축
        }
        else if (m == GLUT_RIGHT_BUTTON) {
            g_AxisMode = 3; // Z축
        }
    }
}

int main(int argc, char** argv)
{
    printf("MY SIMPLE OBJ MODEL VIEWER\n");
    printf("Programming Assignment #4 for Computer Graphics. Department of Software, Sejong University\n\n");
    printf("--------------------------------------------------------------------------------------------------\n");
    printf("Spacebar: Starting/stoping rotation\n\n");

    printf("Left Mouse Button: rotating around X-axis\n");
    printf("Middle Mouse Button: rotating around Y-axis\n");
    printf("Right Mouse Button: rotating around Z-axis\n\n");

    printf("'1' key: Using Vertex Normal for shading\n");
    printf("'2' key: Using Surface Normal for shading\n");
    printf("'3' key: Increasing Specular effect (ks)\n");
    printf("'4' key: Decreasing Specular effect (ks)\n");
    printf("'5' key: Increasing Shininess\n");
    printf("'6' key: Decreasing Shininess\n\n");

    printf("'Q' key: Exit the program\n");
    printf("--------------------------------------------------------------------------------------------------\n");


    myModel.ReadOBJ();//콘솔로 파일 읽어들이기

    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(winWidth, winHeight);
    glutCreateWindow("OpenGL Homework 4");

    glewExperimental = true;
    glewInit();

    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

    myInit();

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(MyKeyboard);
    glutMouseFunc(MyMouse); //마우스 입력
    glutMainLoop();

    return 0;
}