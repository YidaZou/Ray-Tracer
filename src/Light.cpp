//
//  Light.cpp
//  A3
//
//  Created by Yida Zou on 3/2/23.
//

#include "Light.hpp"

Light::Light(float _x, float _y, float _z,
             float _r, float _g, float _b)
:x(_x),y(_y),z(_z),
r(_r),g(_g),b(_b)
{};

Light::~Light(){};
