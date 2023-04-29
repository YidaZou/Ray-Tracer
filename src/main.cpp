#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include "Shape.h"
#include "Image.h"
#include "Camera.hpp"
#include "Light.hpp"
#include <algorithm>

using namespace std;

shared_ptr<Sphere> sphere1;
shared_ptr<Sphere> sphere2;
shared_ptr<Sphere> sphere3;
shared_ptr<Sphere> sphere4;
shared_ptr<Plane> plane1;
shared_ptr<Plane> plane2;
shared_ptr<Ellipsoid> ellipsoid1;
vector<shared_ptr<Shape>> shapes;
shared_ptr<Light> light1;
shared_ptr<Light> light2;
vector<shared_ptr<Light>> lights;

shared_ptr<Camera> camera;

glm::vec3 blinnPhong(shared_ptr<Ray> R, int &sI, glm::vec3 &origin, vector<shared_ptr<Shape>> &shapes, vector<shared_ptr<Light>> &lights, int recursion){
    if(recursion > 10){ //# of bounces
        return glm::vec3(0,0,0);
    }
    
    glm::vec3 V = glm::normalize(origin - R->intersect);
    glm::vec3 finalColor = shapes[sI]->ambient;
    
    for(int i=0; i<lights.size(); i++){
        //check shadow
        shared_ptr<Ray> shadowRay = make_shared<Ray>(-1,-1, glm::normalize(R->intersect - lights[i]->pos));
        float lightDist = glm::distance(lights[i]->pos, R->intersect);
        float closestDistance = MAXFLOAT;
        float tempDist;
        for(int j=0; j<shapes.size(); j++){
            tempDist = shapes[j]->intersection(lights[i]->pos, shadowRay, 0, lightDist-0.001);
            if(tempDist > 0 && tempDist < closestDistance){
                closestDistance = tempDist;
            }
        }
        if(lightDist < closestDistance){  //not in shadow
            glm::vec3 l1 = normalize(lights[i]->pos - R->intersect);
            glm::vec3 cd1 = shapes[sI]->diffuse * max(0.0f, glm::dot(l1, R->normal));
            glm::vec3 h1 = glm::normalize(l1 + V);//normalize(l1 + camera->pos);
            glm::vec3 cs1 = shapes[sI]->specular * pow(max(0.0f,dot(h1,R->normal)), shapes[sI]->exponent);
            glm::vec3 color = lights[i]->intensity * (cd1 + cs1);
            finalColor += color;
        }
    }
    
    if(shapes[sI]->reflective){
        //cout<< "test" << endl;
        glm::vec3 newRay = R->ray + (-2 * glm::dot(R->normal, R->ray) * R->normal);
        shared_ptr<Ray> reflectedRay = make_shared<Ray>(-1, -1, newRay);
        float closestDistance = MAXFLOAT;
        int closestShapeI = -1;
        for(int i=0; i<shapes.size(); i++){
            float tempDist = shapes[i]->intersection(R->intersect, reflectedRay, 0, MAXFLOAT);
            if(tempDist > 0 && tempDist < closestDistance){
                closestDistance = tempDist;
                closestShapeI = i;
            }
        }
        if(closestShapeI != -1){
            finalColor += blinnPhong(reflectedRay, closestShapeI, R->intersect, shapes, lights, recursion+1);
        }
    }
    return finalColor;
}

void render(shared_ptr<Camera> camera, vector<shared_ptr<Shape>> &shapes, vector<shared_ptr<Light>> &lights, shared_ptr<Image> image){
    for(int j=0; j<camera->rays.size(); j++){
        int closestShapeI = -1;
        float closestDistance = MAXFLOAT;
        for(int i=0; i<shapes.size(); i++){
            float tempDist = shapes[i]->intersection(camera->pos, camera->rays[j], 0, MAXFLOAT);
            if(tempDist > 0 && tempDist < closestDistance){
                closestDistance = tempDist;
                closestShapeI = i;
            }
        }
        if(closestShapeI != -1){   //checked last shape
            glm::vec3 finalColor = blinnPhong(camera->rays[j], closestShapeI, camera->pos, shapes, lights, 0);
            finalColor = 255.0f*finalColor;
            if (finalColor.r < 0){finalColor.r = 0;}
            if (finalColor.r > 255){finalColor.r = 255;}
            if (finalColor.g < 0){finalColor.g = 0;}
            if (finalColor.g > 255){finalColor.g = 255;}
            if (finalColor.b < 0){finalColor.b = 0;}
            if (finalColor.b > 255){finalColor.b = 255;}
            image->setPixel(camera->rays[j]->x, camera->rays[j]->y, finalColor.r, finalColor.g, finalColor.b);
        }
    }
}

int main(int argc, char **argv)
{
    if(argc < 3) {
        cout << "Usage: ./A6 <SCENE> <IMAGE SIZE> <IMAGE FILENAME>" << endl;
        return 0;
    }
    int scene = atoi(argv[1]);
    int imgSize = atoi(argv[2]);
    string imgName(argv[3]);
    float fov = (M_PI/180)*45;
    glm::vec3 origin = glm::vec3(0,0,5);
    //glm::vec3 plane = glm::vec3(0,0,4);
    camera = make_shared<Camera>(origin, fov, imgSize, imgSize);
    auto image = make_shared<Image>(imgSize, imgSize);
    switch (scene) {
        case 1:
        case 2:
            sphere1 = make_shared<Sphere>(glm::vec3(-0.5, -1.0, 1.0), 1.0,
                                         glm::vec3(1.0, 0.0, 0.0),
                                         glm::vec3(1.0, 1.0, 0.5),
                                         glm::vec3(0.1, 0.1, 0.1), 100);
            shapes.push_back(sphere1);
            sphere2 = make_shared<Sphere>(glm::vec3(0.5, -1.0, -1.0), 1.0,
                                         glm::vec3(0.0, 1.0, 0.0),
                                         glm::vec3(1.0, 1.0, 0.5),
                                         glm::vec3(0.1, 0.1, 0.1), 100);
            shapes.push_back(sphere2);
            sphere3 = make_shared<Sphere>(glm::vec3(0.0, 1.0, 0.0), 1.0,
                                         glm::vec3(0.0, 0.0, 1.0),
                                         glm::vec3(1.0, 1.0, 0.5),
                                         glm::vec3(0.1, 0.1, 0.1), 100);
            shapes.push_back(sphere3);
            
            light1 = make_shared<Light>(glm::vec3(1,2,2), 0.5);
            lights.push_back(light1);
            light2 = make_shared<Light>(glm::vec3(-1,2,-1), 0.5);
            lights.push_back(light2);
            break;
            
        case 3:
            ellipsoid1 = make_shared<Ellipsoid>(glm::vec3(0.5, 0.0, 0.5), 1.0,
                                             glm::vec3(1.0, 0.0, 0.0),
                                             glm::vec3(1.0, 1.0, 0.5),
                                             glm::vec3(0.1, 0.1, 0.1), 100);
            ellipsoid1->ellipseScale = glm::vec3(0.5,0.6,0.2);
            ellipsoid1->ellipseTransform();
            shapes.push_back(ellipsoid1);
            sphere1 = make_shared<Sphere>(glm::vec3(-0.5, 0.0, -0.5), 1.0,
                                         glm::vec3(0.0, 1.0, 0.0),
                                         glm::vec3(1.0, 1.0, 0.5),
                                         glm::vec3(0.1, 0.1, 0.1), 100);
            shapes.push_back(sphere1);
            plane1 = make_shared<Plane>(glm::vec3(0.0, -1.0, 0.0), 1.0,
                                         glm::vec3(1.0, 1.0, 1.0),
                                         glm::vec3(0.0, 0.0, 0.0),
                                         glm::vec3(0.1, 0.1, 0.1), 0);
            plane1->rotation = glm::vec3(0,1,0);
            shapes.push_back(plane1);
            
            light1 = make_shared<Light>(glm::vec3(1,2,2), 0.5);
            lights.push_back(light1);
            light2 = make_shared<Light>(glm::vec3(-1,2,-1), 0.5);
            lights.push_back(light2);
            break;
        case 4:
        case 5:
            sphere1 = make_shared<Sphere>(glm::vec3(0.5, -0.7, 0.5), 0.3,
                                         glm::vec3(1.0, 0.0, 0.0),
                                         glm::vec3(1.0, 1.0, 0.5),
                                         glm::vec3(0.1, 0.1, 0.1), 100);
            shapes.push_back(sphere1);
            sphere2 = make_shared<Sphere>(glm::vec3(1.0, -0.7, 0.0), 0.3,
                                         glm::vec3(0.0, 0.0, 1.0),
                                         glm::vec3(1.0, 1.0, 0.5),
                                         glm::vec3(0.1, 0.1, 0.1), 100);
            shapes.push_back(sphere2);
            
            sphere3 = make_shared<Sphere>(glm::vec3(-0.5, 0.0, -0.5), 1.0,
                                         glm::vec3(0.0, 0.0, 0.0),
                                         glm::vec3(0.0, 0.0, 0.0),
                                         glm::vec3(0.1, 0.1, 0.1), 0);
            sphere3->reflective = true;
            shapes.push_back(sphere3);
            sphere4 = make_shared<Sphere>(glm::vec3(1.5, 0.0, -1.5), 1.0,
                                         glm::vec3(0.0, 0.0, 0.0),
                                         glm::vec3(0.0, 0.0, 0.0),
                                         glm::vec3(0.0, 0.0, 0.0), 0);
            sphere4->reflective = true;
            shapes.push_back(sphere4);
            
            plane1 = make_shared<Plane>(glm::vec3(0.0, -1.0, 0.0), 1.0,
                                         glm::vec3(1.0, 1.0, 1.0),
                                         glm::vec3(0.0, 0.0, 0.0),
                                         glm::vec3(0.1, 0.1, 0.1), 0);
            plane1->rotation = glm::vec3(0,1,0);
            shapes.push_back(plane1);
            plane2 = make_shared<Plane>(glm::vec3(0.0, 0.0, -3.0), 1.0,
                                         glm::vec3(1.0, 1.0, 1.0),
                                         glm::vec3(0.0, 0.0, 0.0),
                                         glm::vec3(0.1, 0.1, 0.1), 0);
            plane2->rotation = glm::vec3(0,0,1);
            shapes.push_back(plane2);
            
            light1 = make_shared<Light>(glm::vec3(-1,2,1), 0.5);
            lights.push_back(light1);
            light2 = make_shared<Light>(glm::vec3(0.5,-0.5,0.0), 0.5);
            lights.push_back(light2);
            break;
        default:
            break;
    }
    
    render(camera, shapes, lights, image);
    image->writeToFile(imgName);
    return 0;
}
