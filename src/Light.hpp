//
//  Light.hpp
//  A3
//
//  Created by Yida Zou on 3/2/23.
//

#ifndef Light_hpp
#define Light_hpp

#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Light{
public:
    Light(glm::vec3 _pos,
          float _intensity) :
    pos(_pos), intensity(_intensity){}
    
    ~Light(){}
    
    glm::vec3 pos;    //position
    float intensity;    //intensity
private:
    
};

#endif /* Light_hpp */
