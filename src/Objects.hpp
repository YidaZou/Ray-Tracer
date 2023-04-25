//
//  Objects.hpp
//  A3
//
//  Created by Yida Zou on 3/27/23.
//

#ifndef Objects_hpp
#define Objects_hpp

#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shape.h"
#include <vector>
#include <algorithm>
#include "MatrixStack.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Objects{
public:
    float xTrans, yTrans, zTrans;
    float scale;
    float baseY;
    float shear;
    float xRot, yRot, zRot;
    std::shared_ptr<Shape> s;

    Objects(float xTrans, float yTrans, float zTrans,
            float scale,
            float baseY,
            float shear,
            float xRot, float yRot, float zRot, std::shared_ptr<Shape> s);
    ~Objects();
    void draw(std::shared_ptr<MatrixStack> MV, std::shared_ptr<Program> prog, unsigned int n, std::vector<float> positions, unsigned int start);
    
};
#endif /* Objects_hpp */
