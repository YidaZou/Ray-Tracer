//
//  boundingBox.h
//  A1
//
//  Created by Yida Zou on 1/26/23.
//

#ifndef boundingBox_h
#define boundingBox_h
#include <vector>
#include <algorithm>
#include "Image.h"

vector<float> findBounds(vector<Vertex>& vertices){
    float xMin = vertices[0].x;
    float xMax = vertices[0].x;
    float yMin = vertices[0].y;
    float yMax = vertices[0].y;
    float zMin = vertices[0].z;
    float zMax = vertices[0].z;
    for(auto v : vertices){
        if(v.x < xMin){
            xMin = v.x;
        }
        if(v.x > xMax){
            xMax = v.x;
        }
        if(v.y < yMin){
            yMin = v.y;
        }
        if(v.y > yMax){
            yMax = v.y;
        }
        if(v.z < zMin){
            zMin = v.z;
        }
        if(v.z > zMax){
            zMax = v.z;
        }
    }
    return {xMin,xMax,yMin,yMax,zMin,zMax};
}



void scaleTranslate(int& width, int& height, vector<float>& bounds, vector<Triangle>& triangles){
    //SCALE
    float dXo = bounds[1]-bounds[0];  //xMax - xMin : of obj
    float dYo = bounds[3]-bounds[2];  //yMax - yMin : of obj
    float xScale = width/dXo;
    float yScale = height/dYo;
    float scale = std::min(xScale, yScale); //find if height or length is limiting
    
    //TRANSLATION
    //middle of image
    int midX = width/2;
    int midY = height/2;
    int midObjX = scale * ((bounds[1]+bounds[0])/2);  //s * (1/2)(Xmax + Xmin)
    int midObjY = scale * ((bounds[3]+bounds[2])/2);  //s * (1/2)(Ymax + Ymin)
    int transX = midX - midObjX;
    int transY = midY - midObjY;
    
    //scale and translate triangles
    for(auto& tri : triangles){
        tri.a.x = scale * tri.a.x + transX;
        tri.a.y = scale * tri.a.y + transY;
        tri.b.x = scale * tri.b.x + transX;
        tri.b.y = scale * tri.b.y + transY;
        tri.c.x = scale * tri.c.x + transX;
        tri.c.y = scale * tri.c.y + transY;
        //std::cout << "new: " << tri.b.x << std::endl;
    }
    //scale and translate bounds of object for later
    bounds[0] = scale * bounds[0] + transX; //xMin
    bounds[1] = scale * bounds[1] + transX; //xMax
    bounds[2] = scale * bounds[2] + transY; //yMin
    bounds[3] = scale * bounds[3] + transY; //yMax
}


#endif /* boundingBox_h */
