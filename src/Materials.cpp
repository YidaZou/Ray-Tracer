//
//  Materials.cpp
//  A3
//
//  Created by Yida Zou on 3/1/23.
//

#include "Materials.hpp"

Materials::Materials(float _kaR, float _kaG, float _kaB,
                     float _kdR, float _kdG, float _kdB,
                     float _ksR, float _ksG, float _ksB):
kaR(_kaR),kaG(_kaG),kaB(_kaB),
kdR(_kdR),kdG(_kdG),kdB(_kdB),
ksR(_ksR),ksG(_ksG),ksB(_ksB)
{};

Materials::~Materials(){};
