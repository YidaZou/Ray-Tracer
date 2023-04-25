//
//  Objects.cpp
//  A3
//
//  Created by Yida Zou on 3/27/23.
//

#include "Objects.hpp"
#include "Program.h"
Objects::Objects(float _xTrans, float _yTrans, float _zTrans,
        float _scale, float _baseY, float _shear,
        float _xRot, float _yRot, float _zRot, std::shared_ptr<Shape> _s)
:xTrans(_xTrans),yTrans(_yTrans),zTrans(_zTrans),
scale(_scale),baseY(_baseY),shear(_shear),
xRot(_xRot),yRot(_yRot),zRot(_zRot), s(_s){}

Objects::~Objects(){}

void Objects::draw(std::shared_ptr<MatrixStack> MV, std::shared_ptr<Program> prog, unsigned int n, std::vector<float> positions,
                   unsigned int start){
    /*
    unsigned int posI = 0;
    for(int i=0; i<n; i++){
        MV->pushMatrix();
        MV->translate(0.0f, baseY, 0.0f);  //align with ground
        float x = positions[start + posI];
        float z = positions[start + posI+1];
        posI += 2;
        MV->translate(x, 0, z);
        MV->rotate(positions[i], 0,1,0);
        auto MV_it = glm::inverse(glm::transpose((MV->topMatrix())));
        glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
        glUniformMatrix4fv(prog->getUniform("MV_it"), 1, GL_FALSE, glm::value_ptr(MV_it));
        glUniform3f(prog->getUniform("ka"), RANDOM_COLORS[i%7][0]*255, RANDOM_COLORS[i%7][1]*255, RANDOM_COLORS[i%7][2]*255);  //ambient color
        glUniform3f(prog->getUniform("kd"), RANDOM_COLORS[i%7][0]*255, RANDOM_COLORS[i%7][1]*255, RANDOM_COLORS[i%7][2]*255);  //ambient color
        glUniform3f(prog->getUniform("ks"), RANDOM_COLORS[i%7][0]*255, RANDOM_COLORS[i%7][1]*255, RANDOM_COLORS[i%7][2]*255);  //ambient color
        
        s->draw(prog);
        MV->popMatrix();
    }
     */
}
