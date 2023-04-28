#pragma once
#ifndef SHAPE_H
#define SHAPE_H

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Camera.hpp"

class Shape{
public:
	Shape(glm::vec3 pos,
          float scale,
          glm::vec3 diffuse,
          glm::vec3 specular,
          glm::vec3 ambient,
          float exponent);
	virtual ~Shape();
    virtual float intersection(glm::vec3 &p, std::shared_ptr<Ray> r, float near, float far){return 0;};
    
    glm::vec3 pos;
    float scale;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 ambient;
    float exponent;
    
};

class Sphere : public Shape{
public:
    //void initSphere(float _r){this->r = _r;}
    using Shape::Shape;
    float intersection(glm::vec3 &p, std::shared_ptr<Ray> r, float near, float far);
private:
    //float r;    //radius
};

#endif
