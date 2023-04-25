#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 MV_it;
uniform float t;

attribute vec4 aPos; // in object space //aPos.x = x; aPos.y = theta
attribute vec3 aNor; // in object space

varying vec3 vPos;
varying vec3 normal;    //camera normal
varying vec3 fragPos;   //interpolated fragment position

void main()
{
    //revolution calculations
    vec3 p = vec3(aPos.x, (cos(aPos.x+t)+2)*cos(aPos.y), (cos(aPos.x+t)+2)*sin(aPos.y));
    vec3 dpdx = vec3(1, -sin(aPos.x+t)*cos(aPos.y), -sin(aPos.x+t)*sin(aPos.y));
    vec3 dpdtheta = vec3(0, -(cos(aPos.x+t)+2)*sin(aPos.y), (cos(aPos.x+t)+2)*cos(aPos.y));
    
	gl_Position = P * MV * vec4(p, 1.0);
    vec4 tmp = MV * vec4(p, 1.0);
    vPos = tmp.xyz;
    tmp = MV_it * vec4(cross(dpdtheta, dpdx),0.0);
    normal = normalize(tmp.xyz);
}
