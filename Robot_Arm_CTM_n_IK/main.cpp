#include <vgl.h>
#include <InitShader.h>
#include <mat.h>
#include <stack>

#include "MyCube.h"
#include "MyPyramid.h"
#include "MyTarget.h"

MyCube cube;
MyPyramid pyramid;
MyTarget target(&cube);

GLuint program;
GLuint uMat;

mat4 CTM;

vec3 redPos;

bool bPlay = false;
bool bChasingTarget= false;
bool bDrawTarget = false;

float ang1 = 0;
float ang2 = 0;
float ang3 = 0;

void myInit()
{
	cube.Init();
	pyramid.Init();	

	program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);
}

float g_time = 0;

void drawRobotArm(float ang1, float ang2, float ang3)
{
	
	mat4 temp = CTM;//CTM default translate -0.4
	mat4 M(1.0);

	// BASE
	M = Translate(0, 0.025, 0) * Scale(0.3, 0.15, 0.15);
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	pyramid.Draw(program);

	// Upper Arm
	CTM *= RotateZ(ang1);

	//Base - Upper Arm Joint
	M = Translate(0, 0, 0) * Scale(0.027, 0.027, 0.27);
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	cube.Draw(program);

		M = Translate(0, 0.2, -0.1) *  Scale(0.1, 0.47, 0.05);
		glUniformMatrix4fv(uMat, 1, true, CTM*M);
		cube.Draw(program);
		//Upper Arm 보강대
		M = Translate(0, 0.2, 0) * Scale(0.1, 0.05, 0.15);
		glUniformMatrix4fv(uMat, 1, true, CTM * M);
		cube.Draw(program);

		M = Translate(0, 0.2, 0.1) * Scale(0.1, 0.47, 0.05);
		glUniformMatrix4fv(uMat, 1, true, CTM * M);
		cube.Draw(program);

	//Upper Arm - Lower Arm Joint
	M = Translate(0, 0.4, 0) * Scale(0.027, 0.027, 0.27);
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	cube.Draw(program);

	// Lower Arm
	CTM *= Translate(0, 0.4, 0) * RotateZ(ang2);

		M = Translate(0, 0.2, 0.05) * Scale(0.1, 0.47, 0.05);
		glUniformMatrix4fv(uMat, 1, true, CTM*M);
		cube.Draw(program);

		//Lower Arm	보강대
		M = Translate(0, 0.2, 0) * Scale(0.1, 0.05, 0.05);
		glUniformMatrix4fv(uMat, 1, true, CTM * M);
		cube.Draw(program);

		M = Translate(0, 0.2, -0.05) * Scale(0.1, 0.47, 0.05);
		glUniformMatrix4fv(uMat, 1, true, CTM * M);
		cube.Draw(program);
		
	//Lower Arm - Hand Joint
	M = Translate(0, 0.4, 0) * Scale(0.027, 0.027, 0.17);
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	cube.Draw(program);


	// Hand
	CTM *= Translate(0, 0.4, 0) * RotateZ(ang3)*RotateZ(90);
	
	std::stack<mat4> mStack; //손 제작용 CTM Stack

	//손
	M = Translate(0, 0, 0) * Scale(0.1, 0.1, 0.05);//손목
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	cube.Draw(program);

	M = Translate(0, 0.1, 0) * Scale(0.15, 0.1, 0.05);//손바닥
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	cube.Draw(program);

	
	//손가락1
	mStack.push(CTM); //손바닥 상태 저장
	{
		// 첫 번째 마디
		CTM *= Translate(0.085, 0.07, 0.0185);
		M = Scale(0.035, 0.035, 0.09);
		glUniformMatrix4fv(uMat, 1, true, CTM * M);
		cube.Draw(program);

		// 두 번째 마디
		CTM *= Translate(-0.02, 0, 0.035);
		M = Scale(0.06, 0.025, 0.025);
		glUniformMatrix4fv(uMat, 1, true, CTM * M);
		cube.Draw(program);
	}CTM = mStack.top(); mStack.pop(); //손바닥 상태로 복원

	//손가락2
	mStack.push(CTM); //손바닥 상태 저장
	{
		// 첫 번째 마디
		CTM *= Translate(-0.058, 0.16, 0.0185);
		M = Scale(0.035, 0.035, 0.09);
		glUniformMatrix4fv(uMat, 1, true, CTM * M);
		cube.Draw(program);
		// 두 번째 마디
		CTM *= Translate(0, -0.02, 0.035);
		M = Scale(0.025, 0.06, 0.025);
		glUniformMatrix4fv(uMat, 1, true, CTM * M);
		cube.Draw(program);
	}CTM = mStack.top(); mStack.pop(); //손바닥 상태로 복원

	//손가락3
	mStack.push(CTM); //손바닥 상태 저장
	{
		// 첫 번째 마디
		CTM *= Translate(-0.017, 0.16, 0.0185);
		M = Scale(0.035, 0.035, 0.09);
		glUniformMatrix4fv(uMat, 1, true, CTM * M);
		cube.Draw(program);
		// 두 번째 마디
		CTM *= Translate(0, -0.02, 0.035);
		M = Scale(0.025, 0.06, 0.025);
		glUniformMatrix4fv(uMat, 1, true, CTM * M);
		cube.Draw(program);
	}CTM = mStack.top(); mStack.pop(); //손바닥 상태로 복원

	//손가락4
	mStack.push(CTM); //손바닥 상태 저장
	{
		// 첫 번째 마디
		CTM *= Translate(0.022, 0.16, 0.0185);
		M = Scale(0.035, 0.035, 0.09);
		glUniformMatrix4fv(uMat, 1, true, CTM * M);
		cube.Draw(program);
		// 두 번째 마디
		CTM *= Translate(0, -0.02, 0.035);
		M = Scale(0.025, 0.06, 0.025);
		glUniformMatrix4fv(uMat, 1, true, CTM * M);
		cube.Draw(program);
	}CTM = mStack.top(); mStack.pop(); //손바닥 상태로 복원

	//손가락 5(검지) 목표를 가리키는 손가락
	mStack.push(CTM); //손바닥 상태 저장
	{
		// 첫 번째 마디
		CTM *= Translate(0.055, 0.185, 0);
		M = Scale(0.035, 0.09, 0.035);
		glUniformMatrix4fv(uMat, 1, true, CTM * M);
		cube.Draw(program);
		// 두 번째 마디
		CTM *= Translate(0, 0.075, 0);
		M = Scale(0.025, 0.06, 0.025);
		glUniformMatrix4fv(uMat, 1, true, CTM * M);
		cube.Draw(program);
	}CTM = mStack.top(); mStack.pop(); //손바닥 상태로 복원


	CTM = temp;
}





// 주어진 3개의 각도(angle)에 Hand 끝 위치 전달
vec3 calculateHandPos(const vec3& angle) {
	mat4 HandCTM = Translate(0, -0.4, 0)
		* RotateZ(angle.x)
		* Translate(0, 0.4, 0)
		* RotateZ(angle.y)
		* Translate(0, 0.4, 0)
		* RotateZ(angle.z)
		* Translate(-0.33, 0.08, 0);
	
	
	vec4 calculatedHandPos = HandCTM * vec4(0, 0, 0, 1);

	return vec3(calculatedHandPos.x,calculatedHandPos.y,calculatedHandPos.z);
}

float degrees(float radians) { return radians * 180.0f / 3.1415926535f; }
float radians(float degrees) { return degrees * 3.141592 / 180.0f; }

void computeAngle()
{
	vec3 curHandTargetVec = redPos; //update red box position
	vec3 currentAngles(ang1, ang2, ang3);
	vec3 angDiff;

	const int maxIterations = 20;			//Repeat Time
	const float learningRate = 0.5f;		//학습률을 낮춤 (떨림 방지)
	const float deltaDeg = 0.01f;			// 미분을 위한 아주 작은 변화량 (도 단위)
	const float deltaRad = radians(deltaDeg); // 라디안으로 변환

	
	for (int iter = 0; iter < maxIterations; ++iter)
	{
		vec3 curHandPosVec = calculateHandPos(currentAngles);//현재 ang1,ang2,ang3 기준으로 hand position 계산
		vec3 error = curHandPosVec - curHandTargetVec;//오차(hand와 redpos의 거리)

		if (length(error) < 0.01f) break;//오차 원하는 값 이하면 반복문 강제 종료

		//ang1, ang2, ang3에 각각 작은 크기의 각을 더해 순간 변화율(기울기)의 효과를 적용
	
		//ang1 순간 변화율
		angDiff = currentAngles; angDiff.x += deltaDeg;
		vec3 ang1Diff = calculateHandPos(angDiff);//ang1에 deltaDeg 추가한 후 Robot Arm과 Redpos의 거리
		vec3 ang1Slope = (ang1Diff - curHandPosVec) / deltaRad;//ang1을 추가하기 전과 후의 순간 변화율 계산

		//ang2 순간 변화율 
		angDiff = currentAngles; angDiff.y += deltaDeg;
		vec3 ang2Diff = calculateHandPos(angDiff);
		vec3 ang2Slope = (ang2Diff - curHandPosVec) / deltaRad;

		//ang3 순간 변화율
		angDiff = currentAngles; angDiff.z += deltaDeg;
		vec3 ang3Diff = calculateHandPos(angDiff);
		vec3 ang3Slope = (ang3Diff - curHandPosVec) / deltaRad;

		//내적하여 최적의 크기와 방향 판단 -> 실질적 기울기
		float deltaAng1Rad = dot(ang1Slope, error) * 4 *learningRate;
		float deltaAng2Rad = dot(ang2Slope, error) *learningRate;
		float deltaAng3Rad = dot(ang3Slope, error) * learningRate;

		//Gradient Descent 수행
		currentAngles.x -= degrees(deltaAng1Rad);
		currentAngles.y -= degrees(deltaAng2Rad);
		currentAngles.z -= degrees(deltaAng3Rad);
	}

	//계산된 각도 ang에 적용
	ang1 = currentAngles.x;
	ang2 = currentAngles.y;
	ang3 = currentAngles.z;
}
void myDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	GLuint uColor = glGetUniformLocation(program, "uColor");
	glUniform4f(uColor, -1, -1, -1, -1);


	uMat = glGetUniformLocation(program, "uMat");
	CTM = Translate(0, -0.4, 0) * RotateY(g_time*30);
	drawRobotArm(ang1, ang2, ang3);

	
	redPos = target.GetPosition(g_time);//빨간 박스의 현재값 불러오기
	redPos.y -= 0.4f;//빨간 박스 위치 보정

	glUniform4f(uColor, 1, 0, 0, 1);
	if (bDrawTarget == true) {
		target.Draw(program, CTM, g_time);
	}
	
	glutSwapBuffers();
}

void myIdle()
{
	if(bPlay)
	{
		g_time += 1/60.0f;
		Sleep(1/60.0f*1000);

		if(bChasingTarget == false)
		{
			ang1 = 45 * sin(g_time*3.141592);
			ang2 = 60 * sin(g_time*2*3.141592);
			ang3 = 30 * sin(g_time*3.141592);
		}
		else
			computeAngle();

		glutPostRedisplay();
	}
}

void myKeyboard(unsigned char c, int x, int y)
{
	switch(c)
	{
	case '1':
		bChasingTarget = !bChasingTarget;
		break;
	case '2':
		bDrawTarget = !bDrawTarget;
		break;
	case '3':
		target.toggleRandom();
		break;
	case ' ':
		bPlay = !bPlay;
		break;
	default:
		break;
	}
}


int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
	glutInitWindowSize(500,500);
	glutCreateWindow("Target Pointing Robot Arm");

	glewExperimental = true;
	glewInit();

	myInit();
	glutDisplayFunc(myDisplay);
	glutKeyboardFunc(myKeyboard);
	glutIdleFunc(myIdle);
	printf("SIMPLE TARGET POINTING ROBOT ARM\n");
	printf("JaeDong Hwang, Department of Software, Sejong University\n");
	printf("===============================================================\n");
	printf("'1' key: Toggle Between Chasing the target or not\n");
	printf("'2' key: Toggle Between Showing the target position or not\n");
	printf("'3' key: Toggle Between Randomly Positioning the target or not\n\n");

	printf("Spacebar: start/stop Playing\n");
	printf("===============================================================\n");
	glutMainLoop();

	return 0;
}