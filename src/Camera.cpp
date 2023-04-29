//
//  Camera.cpp
//  A6
//
//  Created by Yida Zou on 4/25/23.
//

#include "Camera.hpp"

//looks down the negative z axis by default
Camera::Camera(glm::vec3 _pos, float _fov, glm::vec3 view, int _width, int _height)
:pos(_pos),fov(_fov),width(_width),height(_height)
{
    //maps rays to pixels
    //topLeft -> bottomRight
    //aspect ratio is 1
    float z = -1;
    for(float i=0; i<width; i++){
        for(float j=0; j<height; j++){
            float x = (2*(i+0.5)/width - 1) * tan(fov * 0.5);
            float y = (1 - 2*(j+0.5)/height) * tan(fov * 0.5);
            std::shared_ptr<Ray> newR;
            if(view.x!=0){
                newR = std::make_shared<Ray>(i,height-1-j,glm::normalize(glm::vec3(view.x,y,x)));
            }else if(view.y!=0){
                newR = std::make_shared<Ray>(i,height-1-j,glm::normalize(glm::vec3(x,view.y,y)));
            }else if(view.z!=0){
                newR = std::make_shared<Ray>(i,height-1-j,glm::normalize(glm::vec3(x,y,view.z)));
            }
            rays.push_back(newR);
            //std::cout << newP->x << std::endl;
        }
    }
};

Camera::~Camera(){}


