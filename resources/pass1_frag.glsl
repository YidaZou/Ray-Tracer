#version 120

uniform vec3 ke;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

uniform vec3 lightArray[10];
uniform vec3 lightPos[10];

varying vec3 vPos;
varying vec3 normal;    //camera normal
varying vec3 fragPos;   //interpolated fragment position

uniform sampler2D posTexture;
uniform sampler2D norTexture;
uniform sampler2D keTexture;
uniform sampler2D kdTexture;
uniform vec2 windowSize;

void main()
{
    vec3 n = normalize(normal);
    
    gl_FragData[0].xyz = vPos;
    gl_FragData[1].xyz = n;
    gl_FragData[2].xyz = ke;
    gl_FragData[3].xyz = kd;

}
