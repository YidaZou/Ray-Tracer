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
    Light(float x, float y, float z,
          float r, float g, float b);
    ~Light();
    
    float x,y,z;    //position
    float r,g,b;    //color
private:
    
};

#endif /* Light_hpp */
