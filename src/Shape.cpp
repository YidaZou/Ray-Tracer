#include "Shape.h"
#include <algorithm>
#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

using namespace std;

Shape::Shape(glm::vec3 _pos,
             float _scale,
             glm::vec3 _diffuse,
             glm::vec3 _specular,
             glm::vec3 _ambient,
             float _exponent) :
pos(_pos),
scale(_scale),
diffuse(_diffuse),
specular(_specular),
ambient(_ambient),
exponent(_exponent)
{}

Shape::~Shape()
{}


//SPHERE
float Sphere::intersection(glm::vec3 &p, shared_ptr<Ray> r, float near, float far){
    float a = glm::dot(r->ray,r->ray);
    float b = 2*glm::dot(r->ray,p-pos);
    float c = glm::dot(p - pos,p - pos) - pow(scale,2); //scale == radius
    float d = pow(b,2) - 4*a*c;

    float t;
    if(d > 0){
        float tP = (-b + sqrt(d))/(2*a);
        float tM = (-b - sqrt(d))/(2*a);
        t = min(tP,tM); //take closest
        if(t < r->intDist && t > near && t < far){
            r->intersect = p + t*r->ray;
            r->normal = glm::normalize(r->intersect - pos);
            r->intDist = t;
            return t;
        }
    }else if(abs(d) <= 0.0001){
        t = (-b + sqrt(d))/(2*a);
        if(t < r->intDist && t > near && t < far){
            r->intersect = p + t*r->ray;
            r->normal = glm::normalize(r->intersect - pos);
            r->intDist = t;
            return t;
        }
    }
    return 0;
}

//PLANE
float Plane::intersection(glm::vec3 &p, shared_ptr<Ray> r, float near, float far){
    //rotation is normal
    float t = glm::dot(rotation,pos - p) / (glm::dot(rotation, r->ray));
    if(t < r->intDist && t > near && t < far){
        r->intersect = p + t*r->ray;
        r->normal = rotation;
        r->intDist = t;
        return t;
    }
    return 0;
}

//ELLIPSOID
void Ellipsoid::ellipseTransform(){
    auto M = make_shared<MatrixStack>();
    M->translate(pos);
    //glm::vec3 axis = glm::normalize(glm::vec3(1.0,1.0,1.0));
    //M->rotate(1.0,axis);
    M->scale(ellipseScale);
    E = M->topMatrix();
}
float Ellipsoid::intersection(glm::vec3 &p, shared_ptr<Ray> r, float near, float far){
    glm::mat4 inverseE = glm::inverse(E);
    glm::mat4 inverseTransposeE = glm::inverse(glm::transpose(E));
    glm::vec3 pP = inverseE*glm::vec4(p,1.0);
    glm::vec3 vP = normalize(glm::vec3(inverseE*glm::vec4(r->ray,0.0)));
    float a = glm::dot(vP,vP);
    float b = 2*glm::dot(vP,pP);
    float c = glm::dot(pP,pP) - 1;
    float d = pow(b,2) - 4*a*c;

    float t;
    if(d > 0){
        float tP = (-b + sqrt(d))/(2*a);
        float tM = (-b - sqrt(d))/(2*a);
        t = min(tP,tM); //take closest
        glm::vec3 xP = pP+t*vP;
        glm::vec3 x = E * glm::vec4(xP, 1.0);
        glm::vec3 n = glm::normalize(glm::vec3(inverseTransposeE * glm::vec4(xP, 0.0)));
        t = distance(x, p);
        if(glm::dot(r->ray, (x - p)) < 0){
            t = -t;
        }
        if(t < r->intDist && t > near && t < far){
            r->intersect = x;
            r->normal = n;
            r->intDist = t;
            return t;
        }
    }else if(abs(d) <= 0.0001){
        t = (-b + sqrt(d))/(2*a);
        glm::vec3 xP = pP+t*vP;
        glm::vec3 x = E * glm::vec4(xP, 1.0);
        glm::vec3 n = glm::normalize(glm::vec3(inverseTransposeE * glm::vec4(xP, 0.0)));
        t = distance(x, p);
        if(glm::dot(r->ray, (x - p)) < 0){
            t = -t;
        }
        if(t < r->intDist && t > near && t < far){
            r->intersect = x;
            r->normal = n;
            r->intDist = t;
            return t;
        }
    }
    return 0;
}
