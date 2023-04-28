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
    //origin (p), direction (v)
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
