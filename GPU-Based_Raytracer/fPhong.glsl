#version 330

in vec3 N3; 
in vec3 L3; 
in vec3 V3; 
in vec3 wV;
in vec3 wP;
in vec3 wN;

out vec4 fColor;

struct Material {
    vec4  k_d;  // diffuse coefficient
    vec4  k_s;  // specular coefficient
    float n;    // specular exponent
}; 

struct Sphere {
    vec4     center;
    float    radius;
    Material mtl;
};

struct Ray {
    vec3 pos;
    vec3 dir;
};

struct HitInfo {
    float    t;
    vec4     position;
    vec3     normal;
    Material mtl;
};

uniform mat4 uModelMat; 
uniform mat4 uViewMat; 
uniform mat4 uProjMat;
uniform vec4 uLPos; 
uniform vec4 uLIntensity;
uniform vec4 uAmb; 
uniform vec4 uDif; 
uniform vec4 uSpc; 
uniform float uShininess;
uniform samplerCube uCube;
uniform vec4 uEPos;
uniform int uNumSphere;
uniform Sphere uSpheres[20];
uniform int uBounceLimit;
uniform int uDrawingMode;


bool IntersectRay( inout HitInfo hit, Ray ray )
{
    hit.t = 1e30;
    bool foundHit = false;

    for ( int i=0; i<uNumSphere; ++i ) {
        //Ray-Sphere Intersection
        vec3 oc = ray.pos - uSpheres[i].center.xyz;
        float b = dot(oc, ray.dir);
        float c = dot(oc, oc) - uSpheres[i].radius * uSpheres[i].radius;
        float h = b * b - c;//판별식

        if (h >= 0.0) {//광선이 구체에 접하거나 뚫고 지나감
            float t = -b - sqrt(h); //closest intersection
            //update hit info
            if (t > 0.001 && t < hit.t) {
                hit.t = t;
                hit.position = vec4(ray.pos + t * ray.dir, 1.0);
                hit.normal = normalize(hit.position.xyz - uSpheres[i].center.xyz);
                hit.mtl = uSpheres[i].mtl;
                foundHit = true;
            }
        }
    }
    return foundHit;
}


vec4 Shade( Material mtl, vec4 position, vec3 normal, vec3 view )
{
    vec4 color = vec4(0,0,0,1);
    vec3 lightDir = normalize(uLPos.xyz - position.xyz);

    // Shadow Check 충돌 지점에서 광원 방향으로 다시 광선을 쏨 
    Ray shadowRay;
    shadowRay.pos = position.xyz + normal * 0.001; // Offset
    shadowRay.dir = lightDir;
    
    HitInfo shadowHit;
    if (!IntersectRay(shadowHit, shadowRay)) {
        // 그림자에 가려지지 않았을 때만 Diffuse 적용
        float diff = max(dot(normal, lightDir), 0.0);
        color += mtl.k_d * uLIntensity * diff;
    }

    return color;
}

vec4 RayTracer( Ray ray )
{
    HitInfo hit;
    if ( IntersectRay( hit, ray ) ) {
        vec3 view = normalize( -ray.dir );
        vec4 clr = Shade( hit.mtl, hit.position, hit.normal, view );

        // Multiple Reflection 구현
        vec4 k_s_accum = hit.mtl.k_s; 
        for ( int bounce=0; bounce<uBounceLimit; bounce++ ) {
            // 반사 계수가 거의 없으면 중단
            if ( k_s_accum.r + k_s_accum.g + k_s_accum.b <= 0.01 ) break;
            
            Ray r; // 반사 광선 생성
            r.dir = reflect(ray.dir, hit.normal);
            r.pos = hit.position.xyz + r.dir * 0.001;
            
            HitInfo h;
            if ( IntersectRay( h, r ) ) {
                // 반사된 지점 쉐이딩 후 누적
                clr += k_s_accum * Shade(h.mtl, h.position, h.normal, -r.dir);
                k_s_accum *= h.mtl.k_s; // 반사 계수 업데이트
                hit = h;
                ray = r;
            } else {
                // 큐브맵 반사
                clr += k_s_accum * texture(uCube, vec3(1,-1,1)*r.dir);
                break;
            }
        }
        return clr;
    } else {
        // 아무것도 맞지 않으면 배경 반환
        return texture(uCube, vec3(1,-1,1)*ray.dir);
    }
}

void main()
{
    if(uDrawingMode == 0) // Rasterize Mode
    {
        vec3 N = normalize(N3); 
        vec3 L = normalize(L3); 
        vec3 V = normalize(V3);
        vec3 H = normalize(V+L); 

        float NL = max(dot(N, L), 0); 
        float VR = pow(max(dot(H, N), 0), uShininess);
        fColor = uAmb + uLIntensity*uDif*NL + uLIntensity*uSpc*VR; 
        fColor.w = 1; 

        vec3 viewDir = wP - wV;
        vec3 dir = reflect(viewDir, wN);
        fColor += uSpc*texture(uCube, vec3(1,-1,1)*dir); 
    }
    else if(uDrawingMode == 1) // Raytracing Mode
    {
        Ray r;
        r.pos = wV; 
        r.dir = normalize(wP - wV); 
        fColor = RayTracer (r); 
    }
}