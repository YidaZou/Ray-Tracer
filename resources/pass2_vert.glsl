#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 MV_it;

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space

varying vec3 vPos;
varying vec3 normal;    //camera normal
varying vec3 fragPos;   //interpolated fragment position

void main()
{
	gl_Position = P * MV * aPos;
}
