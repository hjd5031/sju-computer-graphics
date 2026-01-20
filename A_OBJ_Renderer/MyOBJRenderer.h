#define _CRT_SECURE_NO_WARNINGS
#ifndef MY_OBJ_RENDERER
#define MY_OBJ_RENDERER

#include <vector>
#include <vgl.h>
#include <vec.h>
#include <mat.h>

using namespace std;

struct MyOBJVertex {
    vec3 position;
    vec3 normal;
    vec3 color;
};

struct MyOBJModel {//model이 들어오면 꼭짓점, phong shading, flat shading normal 처리
    std::vector<MyOBJVertex> vertices;
    std::vector<vec3> phongNormals;
    std::vector<vec3> flatNormals;

    GLuint VAO;
    GLuint VBO;
    bool bInitialized;

    vec3 pivot;
    float scaleFactor;

    MyOBJModel() {  //model 초기화
        bInitialized = false;
        VAO = 0; 
        VBO = 0;
        scaleFactor = 1.0f; 
        pivot = vec3(0, 0, 0);
    }

    void MakeModelData(const vector<vec3>& tmpVertex, const vector<int>& tmpFrag) {
        vertices.clear();//벡터 초기화
        phongNormals.clear(); 
        flatNormals.clear();

        int numVerticies = (int)tmpVertex.size();
        vector<vec3> accumulatedNormals(numVerticies, vec3(0, 0, 0));//벡터를 축적할 벡터 초기화

        for (int i = 0; i < tmpFrag.size(); i += 3) {//꼭짓점에 꼭짓점으로 구성된 모든 면의 normal을 축적(phong shading을 위해)
            int idx1 = tmpFrag[i]; int idx2 = tmpFrag[i + 1]; int idx3 = tmpFrag[i + 2];
            vec3 p1 = tmpVertex[idx1]; vec3 p2 = tmpVertex[idx2]; vec3 p3 = tmpVertex[idx3];
            vec3 faceNormal = normalize(cross(p2 - p1, p3 - p1));
            accumulatedNormals[idx1] += faceNormal; accumulatedNormals[idx2] += faceNormal; accumulatedNormals[idx3] += faceNormal; // 면의 벡터 정점에 누적
        }

        for (int i = 0; i < numVerticies; i++) {//축적된 normal 정규화
            if (length(accumulatedNormals[i]) > 0) accumulatedNormals[i] = normalize(accumulatedNormals[i]);
        }

        for (int i = 0; i < tmpFrag.size(); i += 3) {
            int idx[3] = { tmpFrag[i], tmpFrag[i + 1], tmpFrag[i + 2] };

            vec3 p1 = tmpVertex[idx[0]]; 
            vec3 p2 = tmpVertex[idx[1]]; 
            vec3 p3 = tmpVertex[idx[2]];

            vec3 flatN = normalize(cross(p2 - p1, p3 - p1));

            for (int k = 0; k < 3; k++) {
                MyOBJVertex v;
                v.position = tmpVertex[idx[k]];
                v.normal = accumulatedNormals[idx[k]];
                v.color = vec3(0.8f, 0.7f, 1.0f);

                vertices.push_back(v);
                phongNormals.push_back(accumulatedNormals[idx[k]]);
                flatNormals.push_back(flatN);
            }
        }
    }

    void FindPivot(const std::vector<vec3>& tmpVertex, const std::vector<int>& tmpFrag) {
        if (tmpFrag.empty()) return;
        vec3 minPos = tmpVertex[tmpFrag[0]];
        vec3 maxPos = tmpVertex[tmpFrag[0]];

        for (size_t i = 0; i < tmpFrag.size(); i++) {
            vec3 v = tmpVertex[tmpFrag[i]];
            if (v.x < minPos.x) minPos.x = v.x; if (v.x > maxPos.x) maxPos.x = v.x;
            if (v.y < minPos.y) minPos.y = v.y; if (v.y > maxPos.y) maxPos.y = v.y;
            if (v.z < minPos.z) minPos.z = v.z; if (v.z > maxPos.z) maxPos.z = v.z;
        }
        pivot = (minPos + maxPos) / 2.0f;
        float maxLen = max(maxPos.x - minPos.x, max(maxPos.y - minPos.y, maxPos.z - minPos.z));
        scaleFactor = (maxLen > 0) ? (1.5f / maxLen) : 1.0f;
    }

    void ReadOBJ(const char* filename = NULL) {//파일읽기
        char nameBuf[256];
        if (filename == NULL) {//file 이름이 들어올때까지 계속 호출
            while (true) {
                printf("Input OBJ File Name: ");
                scanf("%s", nameBuf);
                FILE* fp = fopen(nameBuf, "r");
                if (fp) { fclose(fp); break; }
                printf("Error: File not found.\n");
            }
        }
        else {
            strcpy(nameBuf, filename);
        }

        FILE* fp = fopen(nameBuf, "r");
        std::vector<vec3> tmpVertex;
        std::vector<int> tmpFrag;
        char buf[256];

        while (fgets(buf, 255, fp)) {
            if (buf[0] == 'v' && buf[1] == ' ') {//obj file에서 vertex 정보 입력
                float x, y, z; sscanf(buf + 2, "%f %f %f", &x, &y, &z);
                tmpVertex.push_back(vec3(x, y, z));
            }
            else if (buf[0] == 'f' && buf[1] == ' ') {//obj file에서 fragment 정보 입력
                int v1, v2, v3; sscanf(buf + 2, "%d %d %d", &v1, &v2, &v3);
                tmpFrag.push_back(v1 - 1); tmpFrag.push_back(v2 - 1); tmpFrag.push_back(v3 - 1);
            }
        }
        fclose(fp);

        FindPivot(tmpVertex, tmpFrag);//들어온 모델의 중심을 피봇으로 설정
        MakeModelData(tmpVertex, tmpFrag);

        printf("OBJ Load Complete: %d vertices\n", (int)vertices.size());
    }

    void InitModel() {
        if (bInitialized || vertices.empty()) return;

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MyOBJVertex), &vertices[0], GL_STATIC_DRAW);

        bInitialized = true;
    }

    void SetPositionAndColorAttribute(GLuint program) {
        GLuint vPosition = glGetAttribLocation(program, "vPosition");
        if (vPosition != -1) {
            glEnableVertexAttribArray(vPosition);
            glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, sizeof(MyOBJVertex), (void*)0);
        }

        GLuint vNormal = glGetAttribLocation(program, "vNormal");
        if (vNormal != -1) {
            glEnableVertexAttribArray(vNormal);
            glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(MyOBJVertex), (void*)sizeof(vec3));
        }

        GLuint vColor = glGetAttribLocation(program, "vColor");
        if (vColor != -1) {
            glEnableVertexAttribArray(vColor);
            glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, sizeof(MyOBJVertex), (void*)(sizeof(vec3) * 2));
        }
    }

    void Draw(GLuint program) {
        if (!bInitialized) return;
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        SetPositionAndColorAttribute(program);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());
    }

    void SetShadingMode(int mode) {
        if (!bInitialized) return;
        for (size_t i = 0; i < vertices.size(); i++) {
            if (mode == 1) vertices[i].normal = phongNormals[i];
            else vertices[i].normal = flatNormals[i];
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(MyOBJVertex), &vertices[0], GL_STATIC_DRAW);
    }

    mat4 InitModelScaleNPos() {
        if (!bInitialized) return mat4(1.0f);
        return Scale(scaleFactor, scaleFactor, scaleFactor) * Translate(-pivot.x, -pivot.y, -pivot.z);
    }
};

#endif