//
//  Hit.hpp
//  A6
//
//  Created by Yida Zou on 4/28/23.
//

#ifndef Hit_hpp
#define Hit_hpp

#include <stdio.h>
#include <glm/glm.hpp>

class Hit{
private:
    glm::vec3 x;    //pos
    glm::vec3 n;    //nor
    float t;        //dist
public:
    Hit() : x(0), n(0), t(0) {}
    Hit(const glm::vec3 &x, const glm::vec3 &n, float t)
        {this->x = x; this->n = n; this->t = t;}
};
#endif /* Hit_hpp */
