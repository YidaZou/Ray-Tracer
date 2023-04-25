//
//  Materials.hpp
//  A3
//
//  Created by Yida Zou on 3/1/23.
//

#ifndef Materials_hpp
#define Materials_hpp

#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


class Materials{
public:
    Materials(float _kaR, float _kaG, float _kaB,
              float _kdR, float _kdG, float _kdB,
              float _ksR, float _ksG, float _ksB);
    ~Materials();
    
    float kaR,kaG,kaB;  //ambient
    float kdR,kdG,kdB;  //diffuse
    float ksR,ksG,ksB;  //specular
private:

};

#endif /* Materials_hpp */
