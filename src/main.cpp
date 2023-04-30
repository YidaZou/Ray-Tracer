#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include "Shape.h"
#include "Image.h"
#include "Camera.hpp"
#include "Light.hpp"
#include <algorithm>
#include "raytri.c"
#include "boundingBox.h"
#include "triangle.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace std;

shared_ptr<Sphere> sphere1;
shared_ptr<Sphere> sphere2;
shared_ptr<Sphere> sphere3;
shared_ptr<Sphere> sphere4;
shared_ptr<Cylinder> cylinder1;
shared_ptr<Plane> plane1;
shared_ptr<Plane> plane2;
shared_ptr<Ellipsoid> ellipsoid1;
vector<shared_ptr<Shape>> shapes;
shared_ptr<Light> light1;
shared_ptr<Light> light2;
vector<shared_ptr<Light>> lights;
shared_ptr<Camera> camera;

bool motionBlur = false;

glm::vec3 blinnPhongOBJ(shared_ptr<Ray> R, shared_ptr<Sphere> sphere1, glm::vec3 &origin, vector<shared_ptr<Light>> &lights, vector<double*> &vertices, glm::vec3 &normal){
    
    glm::vec3 V = glm::normalize(origin - R->intersect);
    glm::vec3 finalColor = sphere1->ambient;
    
    for(int i=0; i<lights.size(); i++){
        //check shadow
        float lightDist = glm::distance(lights[i]->pos, R->intersect);
        double *t = new double(MAXFLOAT);
        double *u = new double;
        double *v = new double;
        float tF = MAXFLOAT;
        double origin[3] = {lights[i]->pos.x,lights[i]->pos.y,lights[i]->pos.z};
        double dir[3] = {R->ray.x,R->ray.y,R->ray.z};
        for(int j=0; j<vertices.size()-2; j+=3){
            if(intersect_triangle(origin, dir, vertices[j], vertices[j+1], vertices[j+2], t, u, v)){
                if(*t>0 && *t<tF){
                    tF = *t;
                }
            }
        }
        if(lightDist < tF){  //not in shadow
            glm::vec3 l1 = normalize(lights[i]->pos - R->intersect);
            glm::vec3 cd1 = sphere1->diffuse * max(0.0f, glm::dot(l1, normal));
            glm::vec3 h1 = glm::normalize(l1 + V);//normalize(l1 + camera->pos);
            glm::vec3 cs1 = sphere1->specular * pow(max(0.0f,dot(h1,normal)), sphere1->exponent);
            glm::vec3 color = lights[i]->intensity * (cd1 + cs1);
            finalColor += color;
        }
    }
    
    return finalColor;
}

void renderOBJ(shared_ptr<Camera> camera, shared_ptr<Sphere> sphere1, vector<shared_ptr<Light>> &lights, shared_ptr<Image> image, vector<double*> &vertices, vector<float*> &normals){
    for(int i=0; i<camera->rays.size(); i++){
        if(sphere1->intersection(camera->pos, camera->rays[i], 0, MAXFLOAT)){
            double *t = new double(MAXFLOAT);
            double *u = new double(-1);
            double *v = new double(-1);
            float tF = MAXFLOAT;
            float uF, vF;
            double origin[3] = {camera->pos.x,camera->pos.y,camera->pos.z};
            double dir[3] = {camera->rays[i]->ray.x,camera->rays[i]->ray.y,camera->rays[i]->ray.z};
            int bufPos = -1;
            for(int j=0; j<vertices.size()-2; j+=3){
                if(intersect_triangle(origin, dir, vertices[j], vertices[j+1], vertices[j+2], t, u, v)){
                    if(*t>0 && *t<tF){
                        tF = *t;
                        uF = *u;
                        vF = *v;
                        bufPos = j;
                        glm::vec3 baryCoord =   (1-uF-vF)*glm::vec3(vertices[j][0],vertices[j][1],vertices[j][2]) +
                                                uF*glm::vec3(vertices[j+1][0],vertices[j+1][1],vertices[j+1][2]) +
                                                vF*glm::vec3(vertices[j+2][0],vertices[j+2][1],vertices[j+2][2]);
                        camera->rays[i]->intersect = baryCoord;
                    }
                }
            }
            //cout<< tF << endl;
            if(bufPos != -1){   //checked last shape
                glm::vec3 normal =  (1-uF-vF)*glm::vec3(normals[bufPos][0],normals[bufPos][1],normals[bufPos][2]) +
                                    uF*glm::vec3(normals[bufPos+1][0],normals[bufPos+1][1],normals[bufPos+1][2]) +
                                    vF*glm::vec3(normals[bufPos+2][0],normals[bufPos+2][1],normals[bufPos+2][2]);
                glm::vec3 color = blinnPhongOBJ(camera->rays[i], sphere1, camera->pos, lights, vertices, normal);
                glm::vec3 finalColor;
                finalColor = 255.0f*color;
                
                if (finalColor.r < 0){finalColor.r = 0;}
                if (finalColor.r > 255){finalColor.r = 255;}
                if (finalColor.g < 0){finalColor.g = 0;}
                if (finalColor.g > 255){finalColor.g = 255;}
                if (finalColor.b < 0){finalColor.b = 0;}
                if (finalColor.b > 255){finalColor.b = 255;}
                image->setPixel(camera->rays[i]->x, camera->rays[i]->y, finalColor.r, finalColor.g, finalColor.b);
            
            }
        }
    }
}

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
            finalColor += shapes[sI]->reflectiveComponent * blinnPhong(reflectedRay, closestShapeI, R->intersect, shapes, lights, recursion+1);
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
            glm::vec3 color = blinnPhong(camera->rays[j], closestShapeI, camera->pos, shapes, lights, 0);
            glm::vec3 finalColor;
            if(motionBlur){
                camera->rays[j]->color += color*0.1f;
                finalColor = 255.0f*camera->rays[j]->color;
            }else{
                finalColor = 255.0f*color;
            }
            
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
    if(scene == 8){
        camera = make_shared<Camera>(glm::vec3(-3,0,0), (M_PI/180)*60, glm::vec3(1,0,0), imgSize, imgSize);
    }else{
        camera = make_shared<Camera>(origin, fov, glm::vec3(0,0,-1), imgSize, imgSize);
    }
    auto image = make_shared<Image>(imgSize, imgSize);
    switch (scene) {
        case 0:
            sphere1 = make_shared<Sphere>(glm::vec3(0.5, -0.7, 0.5), 0.3,
                                         glm::vec3(1.0, 0.0, 0.0),
                                         glm::vec3(1.0, 1.0, 0.5),
                                         glm::vec3(0.1, 0.1, 0.1), 100);
            shapes.push_back(sphere1);
            cylinder1 = make_shared<Cylinder>(glm::vec3(-1.5, 0, 0.5), 0.1,
                                         glm::vec3(1.0, 0.0, 0.0),
                                         glm::vec3(1.0, 1.0, 0.5),
                                         glm::vec3(0.1, 0.1, 0.1), 100);
            shapes.push_back(cylinder1);
            sphere2 = make_shared<Sphere>(glm::vec3(1.0, -0.7, 0.0), 0.3,
                                         glm::vec3(0.0, 0.0, 1.0),
                                         glm::vec3(1.0, 1.0, 0.5),
                                         glm::vec3(0.1, 0.1, 0.1), 100);
            shapes.push_back(sphere2);
            
            sphere3 = make_shared<Sphere>(glm::vec3(-0.5, 0.0, -0.5), 1.0,
                                         glm::vec3(0.0, 0.5, 0.5),
                                         glm::vec3(0.0, 0.0, 0.0),
                                         glm::vec3(0.1, 0.1, 0.1), 0);
            sphere3->reflective = true;
            sphere3->reflectiveComponent = glm::vec3(0.3,0.3,0.3);
            shapes.push_back(sphere3);
            sphere4 = make_shared<Sphere>(glm::vec3(1.5, 0.0, -1.5), 1.0,
                                         glm::vec3(0.0, 0.5, 0.0),
                                         glm::vec3(0.0, 0.0, 0.0),
                                         glm::vec3(0.0, 0.0, 0.0), 0);
            sphere4->reflective = true;
            sphere4->reflectiveComponent = glm::vec3(0.3,0.3,0.3);
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
        case 1:
        case 2:
        case 8:
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
            
            light1 = make_shared<Light>(glm::vec3(-2,1,1), 1);
            lights.push_back(light1);
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
            sphere3->reflectiveComponent = glm::vec3(1,1,1);
            shapes.push_back(sphere3);
            sphere4 = make_shared<Sphere>(glm::vec3(1.5, 0.0, -1.5), 1.0,
                                         glm::vec3(0.0, 0.0, 0.0),
                                         glm::vec3(0.0, 0.0, 0.0),
                                         glm::vec3(0.0, 0.0, 0.0), 0);
            sphere4->reflective = true;
            sphere4->reflectiveComponent = glm::vec3(1,1,1);
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
        case 6:{
            // Load geometry
            vector<float> posBuf; // list of vertex positions
            vector<float> norBuf; // list of vertex normals
            vector<float> texBuf; // list of vertex texture coords
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shap;
            std::vector<tinyobj::material_t> materials;
            string errStr;
            string meshName = "../../resources/bunny.obj";
            bool rc = tinyobj::LoadObj(&attrib, &shap, &materials, &errStr, meshName.c_str());
            if(!rc) {
                cerr << errStr << endl;
            } else {
                // Some OBJ files have different indices for vertex positions, normals,
                // and texture coordinates. For example, a cube corner vertex may have
                // three different normals. Here, we are going to duplicate all such
                // vertices.
                // Loop over shapes
                for(size_t s = 0; s < shap.size(); s++) {
                    // Loop over faces (polygons)
                    size_t index_offset = 0;
                    for(size_t f = 0; f < shap[s].mesh.num_face_vertices.size(); f++) {
                        size_t fv = shap[s].mesh.num_face_vertices[f];
                        // Loop over vertices in the face.
                        for(size_t v = 0; v < fv; v++) {
                            // access to vertex
                            tinyobj::index_t idx = shap[s].mesh.indices[index_offset + v];
                            posBuf.push_back(attrib.vertices[3*idx.vertex_index+0]);
                            posBuf.push_back(attrib.vertices[3*idx.vertex_index+1]);
                            posBuf.push_back(attrib.vertices[3*idx.vertex_index+2]);
                            if(!attrib.normals.empty()) {
                                norBuf.push_back(attrib.normals[3*idx.normal_index+0]);
                                norBuf.push_back(attrib.normals[3*idx.normal_index+1]);
                                norBuf.push_back(attrib.normals[3*idx.normal_index+2]);
                            }
                            if(!attrib.texcoords.empty()) {
                                texBuf.push_back(attrib.texcoords[2*idx.texcoord_index+0]);
                                texBuf.push_back(attrib.texcoords[2*idx.texcoord_index+1]);
                            }
                        }
                        index_offset += fv;
                        // per-face material (IGNORE)
                        shap[s].mesh.material_ids[f];
                    }
                }
            }
            cout << "Number of vertices: " << posBuf.size()/3 << endl;
            vector<double*> vertices;
            vector<float*> normals;
            for(int i=0; i<posBuf.size()-2; i+=3){
                double* vertex = new double[3];
                vertex[0] = posBuf[i];
                vertex[1] = posBuf[i+1];
                vertex[2] = posBuf[i+2];
                vertices.push_back(vertex);
                float* normal = new float[3];
                normal[0] = norBuf[i];
                normal[1] = norBuf[i+1];
                normal[2] = norBuf[i+2];
                normals.push_back(normal);
            }
            /*
            for(int i=0; i<vertices.size(); i++){
                cout<<vertices[i][0]<<endl;
            }
             */
            vector<float> bounds = findBounds(vertices);
            float xDist = bounds[1]-bounds[0];
            float yDist = bounds[3]-bounds[2];
            float zDist = bounds[5]-bounds[4];
            float scale = max(xDist,yDist);
            scale = max(scale, zDist);
            cout << scale <<endl;
            sphere1 = make_shared<Sphere>(glm::vec3(0,1,0), scale,
                                         glm::vec3(0.0, 0.0, 1.0),
                                         glm::vec3(1.0, 1.0, 0.5),
                                         glm::vec3(0.1, 0.1, 0.1), 100);
            light1 = make_shared<Light>(glm::vec3(-1,1,1), 1.0);
            lights.push_back(light1);
            vector<double*> dist;
            vector<double*> bary;
            renderOBJ(camera, sphere1, lights, image, vertices, normals);
        }
        default:
            break;
    }
    if(scene != 0 && scene != 6){
        render(camera, shapes, lights, image);
    }else if(scene == 0){
        motionBlur = true;
        for(int i=0; i<10; i++){
            shapes[0]->pos = glm::vec3(0.5-(float)i/10, -0.7, 0.5+(float)i/10);
            camera->resetRays();
            render(camera, shapes, lights, image);
        }
    }
    
    image->writeToFile(imgName);
    return 0;
}
