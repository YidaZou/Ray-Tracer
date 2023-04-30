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

std::vector<float> findBounds(std::vector<double*>& vertices){
    float xMin = vertices[0][0];
    std::cout<<xMin<<std::endl;
    float xMax = vertices[0][0];
    float yMin = vertices[0][1];
    float yMax = vertices[0][1];
    float zMin = vertices[0][2];
    float zMax = vertices[0][2];
    for(auto v : vertices){
        //std::cout<<v[0]<<std::endl;
        if(v[0] < xMin){
            xMin = v[0];
        }
        if(v[0] > xMax){
            xMax = v[0];
        }
        if(v[1] < yMin){
            yMin = v[1];
        }
        if(v[1] > yMax){
            yMax = v[1];
        }
        if(v[2] < zMin){
            zMin = v[2];
        }
        if(v[2] > zMax){
            zMax = v[2];
        }
    }
    return {xMin,xMax,yMin,yMax,zMin,zMax};
}

#endif /* boundingBox_h */
