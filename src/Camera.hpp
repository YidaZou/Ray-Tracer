//
//  Camera.hpp
//  A6
//
//  Created by Yida Zou on 4/25/23.
//

#ifndef Camera_hpp
#define Camera_hpp

#include <stdio.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

struct Ray{
    int x,y;    //pixel coordinates
    glm::vec3 ray;
    glm::vec3 intersect;
    glm::vec3 normal;
    float intDist;
    
    Ray(int _x, int _y, glm::vec3 _ray) :
    x(_x), y(_y), ray(_ray)
    {intDist = MAXFLOAT;}
};

class Camera{
public:
    Camera(glm::vec3 _pos, float _fov, int _width, int _height);
    ~Camera();
    
    glm::vec3 pos;
    float fov;
    std::vector<std::shared_ptr<Ray>> rays;
    
    //resolution
    int width;
    int height;
};

#endif /* Camera_hpp */
