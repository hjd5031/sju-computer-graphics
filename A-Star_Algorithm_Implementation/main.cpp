
#define _CRT_SECURE_NO_WARNINGS

#include <vgl.h>
#include <InitShader.h>
#include "MyCube.h"
#include "MyUtil.h"

#include <vec.h>
#include <mat.h>
#include <vector>
#include <cmath>


#define MAZE_FILE	"maze.txt"


MyCube cube;
GLuint program;

mat4 g_Mat = mat4(1.0f);
GLuint uMat;
GLuint uColor;

float wWidth = 1000;
float wHeight = 500;

vec3 cameraPos = vec3(0, 0, 0);
vec3 viewDirection = vec3(0, 0, -1);
vec3 goalPos = vec3(0, 0, 0);

int MazeSize;
char maze[255][255] = { 0 };

float cameraSpeed = 0.1;

float g_time = 0;




bool bAutoRun = false;   // 자동 주행 중인지 여부
int currentPathIndex = 0; // 현재 목표로 하고 있는 경로 인덱스

//충돌한 벽의 인덱스 (default -1)
int hitWallX = -1;
int hitWallZ = -1;

inline void getIndexFromPosition(vec3 pos, int& i, int& j) {
	i = (int)(pos.x + MazeSize / 2.0f);
	j = (int)(pos.z + MazeSize / 2.0f);
}

inline vec3 getPositionFromIndex(int i, int j)
{
	float unit = 1;
	vec3 leftTopPosition = vec3(-MazeSize / 2.0 + unit / 2, 0, -MazeSize / 2.0 + unit / 2);
	vec3 xDir = vec3(1, 0, 0);
	vec3 zDir = vec3(0, 0, 1);
	return leftTopPosition + i * xDir + j * zDir;
}

struct Node {		// A* 알고리즘을 위한 노드 구조체
	int x, y;       // 그리드 인덱스
	float G;        // 시작점부터 현재까지의 비용
	float H;        // 현재부터 목표까지의 예상 비용 (Heuristic)
	float F;        // G + H		F(n) = G(n) + H(n)
	Node* parent;   // 경로 역추적을 위한 부모 노드 포인터
	bool visited;   // 방문 여부
	bool isSearch;    // 탐색노드 리스트 포함 여부
};

std::vector<vec3> pathList;// 찾은 경로를 저장할 전역 변수

float GetHeuristic(int x, int y, int targetX, int targetY) {//격자형 구조 -> Manhattan Distance
	return (float)(abs(x - targetX) + abs(y - targetY));
}

void FindPath() {// A* Algorithm
	pathList.clear(); //Initialize Path

	//Calculate Index of Start and Goal 
	int startX, startY, goalX, goalY;
	getIndexFromPosition(cameraPos, startX, startY);
	getIndexFromPosition(goalPos, goalX, goalY);

	//Initialize Node Map 2D Array
	static Node nodes[255][255];
	for (int i = 0; i < 255; i++) {
		for (int j = 0; j < 255; j++) {
			nodes[i][j] = { i, j, 0, 0, 0, nullptr, false, false };
		}
	}

	
	std::vector<Node*> searchNodeList;//탐색할 노드 저장

	//init start node
	Node* startNode = &nodes[startX][startY];
	startNode->G = 0;//현재가 시작점이니까
	startNode->H = GetHeuristic(startX, startY, goalX, goalY);
	startNode->F = startNode->G + startNode->H; //지금까지 온 비용가 예상 비용 합
	startNode->isSearch = true;
	searchNodeList.push_back(startNode);

	while (!searchNodeList.empty()) {
		//리스트에서 F값이 가장 작은 노드 찾기
		Node* current = searchNodeList[0];
		int currentIndex = 0;
		for (int i = 1; i < searchNodeList.size(); i++) {
			if (searchNodeList[i]->F < current->F) {
				current = searchNodeList[i];
				currentIndex = i;
			}
		}

		// 찾은 F값이 가장 작은 노드를 리스트에서 제거하고 방문 처리
		searchNodeList.erase(searchNodeList.begin() + currentIndex);
		current->isSearch = false;
		current->visited = true;

		
		if (current->x == goalX && current->y == goalY) {// 목표 도달 확인
			
			//pathList에 searchNodeList 반대로 저장
			std::vector<vec3> reversedPath;
			Node* trace = current;
			while (trace != nullptr) {
				reversedPath.push_back(getPositionFromIndex(trace->x, trace->y));
				trace = trace->parent;
			}

			pathList.clear();
			for (int i = reversedPath.size() - 1; i >= 0; i--) {
				pathList.push_back(reversedPath[i]);
			}
			
			
			return;
		}

		//상하좌우 탐색 인덱스가 0이면 0,-1로 위를 의미
		int dx[4] = { 0, 0, -1, 1 };
		int dy[4] = { -1, 1, 0, 0 };

		for (int i = 0; i < 4; i++) {
			int nx = current->x + dx[i];
			int ny = current->y + dy[i];

			if (nx < 0 || nx >= MazeSize || ny < 0 || ny >= MazeSize) continue;//맵 범위 체크
			if (maze[nx][ny] == '*') continue;//벽 체크
			if (nodes[nx][ny].visited) continue;//전에 방문한 노드 건너뛰기

			Node* neighbor = &nodes[nx][ny];
			float potentialG = current->G + 1.0f; //그리드 한개만큼이라 이동비용 1추가

			//비용 업데이트
			if (!neighbor->isSearch || potentialG < neighbor->G) {
				neighbor->parent = current;
				neighbor->G = potentialG;
				neighbor->H = GetHeuristic(nx, ny, goalX, goalY);
				neighbor->F = neighbor->G + neighbor->H;

				if (!neighbor->isSearch) {
					neighbor->isSearch = true;
					searchNodeList.push_back(neighbor);
				}
			}
		}
	}
	/*printf("Path Not Found.\n");*/
}

bool isWall(vec3 pos) {//충돌판단
	float unit = 1.0f;
	
	int i = (int)((pos.x  + MazeSize / 2.0f) / unit);
	int j = (int)((pos.z  + MazeSize / 2.0f) / unit);

	// 배열 범위 체크 및 벽 확인
	if (i < 0 || i >= MazeSize || j < 0 || j >= MazeSize) return true;
	if (maze[i][j] == '*') return true;
	return false;
}

void LoadMaze()
{
	FILE* file = fopen(MAZE_FILE, "r");
	char buf[255];
	fgets(buf, 255, file);
	sscanf(buf, "%d", &MazeSize);
	for (int j = 0; j < MazeSize; j++)
	{
		fgets(buf, 255, file);
		for (int i = 0; i < MazeSize; i++)
		{
			maze[i][j] = buf[i];
			if (maze[i][j] == 'C')				// Setup Camera Position
				cameraPos = getPositionFromIndex(i, j);
			if (maze[i][j] == 'G')				// Setup Goal Position
				goalPos = getPositionFromIndex(i, j);
		}
	}
	fclose(file);
}

void DrawMaze()
{
	for (int j = 0; j < MazeSize; j++){
		for (int i = 0; i < MazeSize; i++) {
			if (maze[i][j] == '*'){
				mat4 ModelMat = Translate(getPositionFromIndex(i, j));
				glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);

				if (i == hitWallX && j == hitWallZ) {
					glUniform4f(uColor, 1, 0, 0, 1); //충돌시 빨강
				}
				else {
					vec3 color = vec3(i / (float)MazeSize, j / (float)MazeSize, 1);
					glUniform4f(uColor, color.x, color.y, color.z, 1);
				}
				
			}
				cube.Draw(program);
		}
	}
}

void myInit()
{
	LoadMaze();
	cube.Init();
	program = InitShader("vshader.glsl", "fshader.glsl");

}


void DrawPath()
{
	if (pathList.size() < 2) return; // 점이 2개 이상이어야 선을 그림

	glUseProgram(program);

	
	float speed = 0.2f;
	float waveGap = 0.5f;

	// 0, pathList.size()-1까지
	for (size_t i = 0; i < pathList.size() - 1; i++)
	{
		vec3 p1 = pathList[i];     // 현재 점
		vec3 p2 = pathList[i + 1]; // 다음 점

		vec3 center = (p1 + p2) / 2.0f;
		center.y = -0.45f; //바닥보다 살짝 위로

		vec3 diff = p2 - p1;//두 점의 차이 및 방향

		float thickness = 0.2f;
		vec3 scale = vec3(thickness, 0.1f, thickness);

		if (abs(diff.x) > 0.001f) scale.x = 1.0f; //크기 늘리기
		if (abs(diff.z) > 0.001f) scale.z = 1.0f;

		mat4 ModelMat = Translate(center) * Scale(scale);
		glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);

		//시간에 따라 밝기가 달라짐
		float flowPhase = (g_time * speed) - (i * waveGap);
		float brightness = 0.4f + 0.6f * abs(sin(flowPhase));

		glUniform4f(uColor, brightness, 0, 0, 1);
		cube.Draw(program);
	}

	//모서리
	for (size_t i = 0; i < pathList.size(); i++)
	{
		vec3 pos = pathList[i];
		pos.y = -0.45f;
		// 선 두께보다 아주 살짝 크게 해서 덮어줌
		mat4 ModelMat = Translate(pos) * Scale(0.2f, 0.1f, 0.2f);
		glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
		cube.Draw(program);
	}
}


void DrawGrid()
{
	float n = 40;
	float w = MazeSize;
	float h = MazeSize;

	for (int i = 0; i < n; i++)
	{
		mat4 m = Translate(0, -0.5, -h / 2 + h / n * i) * Scale(w, 0.02, 0.02);
		glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * m);
		glUniform4f(uColor, 1, 1, 1, 1);
		cube.Draw(program);
	}
	for (int i = 0; i < n; i++)
	{
		mat4 m = Translate(-w / 2 + w / n * i, -0.5, 0) * Scale(0.02, 0.02, h);
		glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * m);
		glUniform4f(uColor, 1, 1, 1, 1);
		cube.Draw(program);
	}
}


void drawCamera()
{
	float cameraSize = 0.5;
	float PI = 3.141592f;

	float angle = -atan2(viewDirection.x, -viewDirection.z) * 180.0f / PI;

	//카메라 몸
	mat4 ModelMat = Translate(cameraPos) * RotateY(angle) * Scale(vec3(cameraSize));

	glUseProgram(program);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
	glUniform4f(uColor, 0, 1, 0, 1);
	cube.Draw(program);

	//카메라의 렌즈
	ModelMat = Translate(cameraPos + viewDirection * cameraSize / 2) * RotateY(angle) * Scale(vec3(cameraSize / 2));

	glUseProgram(program);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
	glUniform4f(uColor, 0, 1, 0, 1);
	cube.Draw(program);
}

void drawGoal()
{
	glUseProgram(program);
	float GoalSize = 0.7;

	mat4 ModelMat = Translate(goalPos) * RotateY(g_time * 3) * Scale(vec3(GoalSize));
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
	glUniform4f(uColor, 0, 0, 0, 0);
	cube.Draw(program);

	ModelMat = Translate(goalPos) * RotateY(g_time * 3 + 45) * Scale(vec3(GoalSize));
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
	glUniform4f(uColor, 0, 0, 0, 0);
	cube.Draw(program);
}


void drawScene(bool bDrawCamera = true)
{
	glUseProgram(program);
	uMat = glGetUniformLocation(program, "uMat");
	uColor = glGetUniformLocation(program, "uColor");

	DrawGrid();
	DrawMaze();
	drawGoal();

	DrawPath();


	if (bDrawCamera)
		drawCamera();



}

void display()
{
	glEnable(GL_DEPTH_TEST);

	float vWidth = wWidth / 2;
	float vHeight = wHeight;

	// LEFT SCREEN : View From Camera (Perspective Projection)
	glViewport(0, 0, vWidth, vHeight);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	float h = 4;
	float aspectRatio = vWidth / vHeight;
	float w = aspectRatio * h;
	mat4 ViewMat = myLookAt(cameraPos, cameraPos + viewDirection, vec3(0, 1, 0));
	mat4 ProjMat = myPerspective(45, aspectRatio, 0.01, 20);

	g_Mat = ProjMat * ViewMat;
	drawScene(false);							// drawing scene except the camera


	// RIGHT SCREEN : View from above (Orthographic parallel projection)
	glViewport(vWidth, 0, vWidth, vHeight);
	h = MazeSize;
	w = aspectRatio * h;
	ViewMat = myLookAt(vec3(0, 5, 0), vec3(0, 0, 0), vec3(0, 0, -1));
	ProjMat = myOrtho(-w / 2, w / 2, -h / 2, h / 2, 0, 20);

	g_Mat = ProjMat * ViewMat;
	drawScene(true);


	glutSwapBuffers();
}

void idle()
{
	g_time += 1;
	hitWallX = -1;
	hitWallZ = -1;

	
	
	if ((GetAsyncKeyState(VK_SPACE) & 0x8000) == 0x8000){
		if (!pathList.empty()){
			if (bAutoRun){//toggle
				bAutoRun = false;
			}
			else{
				bAutoRun = true;
				currentPathIndex = 0;

				//init camera pos and rot
				cameraPos = pathList[0];
				if (pathList.size() > 1) {
					viewDirection = normalize(pathList[1] - pathList[0]);
				}
			}
		}
		Sleep(200);
	}

	if (bAutoRun)//자동주행 중
	{
		
		if (currentPathIndex < pathList.size())// 목표 지점에 도달했는지 확인
		{
			vec3 targetPos = pathList[currentPathIndex];

			// Y축 높이 무시하고 X, Z 평면에서의 거리만 계산
			vec3 diff = targetPos - cameraPos;
			diff.y = 0;

			float distance = length(diff);

			
			if (distance < 0.1f)// 목표점에 거의 도착하면 다음 목표로 변경
			{
				currentPathIndex++;
			}
			else
			{
				//진행방향으로 이동 및 회전
				vec3 moveDir = normalize(diff);
				cameraPos += moveDir * cameraSpeed;

				float rotSpeed = 0.3f; //회전 부드러움 정도
				viewDirection = normalize(viewDirection * (1.0f - rotSpeed) + moveDir * rotSpeed);
			}
		}
		else
		{
			bAutoRun = false;// 경로의 끝에 도착했으면 멈춤
		}
	}
	else {
		if ((GetAsyncKeyState('A') & 0x8000) == 0x8000) {	// if "A" key is pressed	: turn left
			mat4 rot = RotateY(cameraSpeed * 20.0f);
			vec4 temp = rot * vec4(viewDirection, 0.0f);
			viewDirection = vec3(temp.x, temp.y, temp.z);
		}
		
		if ((GetAsyncKeyState('D') & 0x8000) == 0x8000) {	// if "A" key is pressed	: turn right
			mat4 rot = RotateY(-cameraSpeed * 20.0f);
			vec4 temp = rot * vec4(viewDirection, 0.0f);
			viewDirection = vec3(temp.x, temp.y, temp.z);
		}
		if ((GetAsyncKeyState('Q') & 0x8000) == 0x8000) {	// if "Q" key is pressed	: show shortest route to goal
			FindPath();
		}
		
		vec3 moveDir = vec3(0, 0, 0);

		if ((GetAsyncKeyState('W') & 0x8000) == 0x8000)		// if "W" key is pressed	: Go Forward
			moveDir = cameraSpeed * viewDirection;
		else if ((GetAsyncKeyState('S') & 0x8000) == 0x8000)// if "A" key is pressed	: Go Backward
			moveDir = -cameraSpeed * viewDirection;

		float padding = 0.3f; // 충돌 여유값

		if (length(moveDir) > 0)
		{
			//x축 이동
			float xOffset = (moveDir.x > 0) ? padding : -padding;
			vec3 nextPosX = cameraPos;
			nextPosX.x += moveDir.x + xOffset;

			if (isWall(nextPosX) == false) {
				cameraPos.x += moveDir.x;
			}
			else {
				getIndexFromPosition(nextPosX, hitWallX, hitWallZ);//카메라 충돌판단
			}

			//z축 이동
			float zOffset = (moveDir.z > 0) ? padding : -padding;
			vec3 nextPosZ = cameraPos;
			nextPosZ.z += moveDir.z + zOffset;

			if (isWall(nextPosZ) == false) {
				cameraPos.z += moveDir.z;
			}
			else {
				getIndexFromPosition(nextPosZ, hitWallX, hitWallZ);//카메라 충돌판단
			}
		}
	}

	Sleep(16);											// for vSync
	glutPostRedisplay();
}

void reshape(int wx, int wy)
{
	printf("%d %d \n", wx, wy);
	wWidth = wx;
	wHeight = wy;
	glutPostRedisplay();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(wWidth, wHeight);

	glutCreateWindow("Homework3 (Maze Navigator)");

	glewExperimental = true;
	glewInit();

	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
		glGetString(GL_SHADING_LANGUAGE_VERSION));

	myInit();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutMainLoop();

	return 0;
}