#version 120

uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 lightColor1;
uniform vec3 lightColor2;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

varying vec3 vPos;
varying vec3 normal;    //camera normal

void main()
{
    vec3 n = normalize(normal);
    vec3 eyeVec = normalize(-1 * vPos);
    
    //light 1
    vec3 l1 = normalize(lightPos1 - vPos);
    vec3 cd1 = kd * max(0, dot(l1, n));
    vec3 h1 = normalize(l1 + eyeVec);///sqrt(dot(l+eyeVec,l+eyeVec));
    vec3 cs1 = ks * pow(max(0, dot(h1,n)), s);
    
    //light 2
    vec3 l2 = normalize(lightPos2 - vPos);
    vec3 cd2 = kd * max(0, dot(l2, n));
    vec3 h2 = normalize(l2 + eyeVec);///sqrt(dot(l+eyeVec,l+eyeVec));
    vec3 cs2 = ks * pow(max(0, dot(h2,n)), s);
    
    //final RGB
    //vec3 final = lightColor1*(ka+cd1+cs1) + lightColor2*(ka+cd2+cs2);
    
    float R = lightColor1.x*(ka.x+cd1.x+cs1.x) + lightColor2.x*(ka.x+cd2.x+cs2.x);
    float G = lightColor1.y*(ka.y+cd1.y+cs1.y) + lightColor2.y*(ka.y+cd2.y+cs2.y);
    float B = lightColor1.z*(ka.z+cd1.z+cs1.z) + lightColor2.z*(ka.z+cd2.z+cs2.z);
    
    //red
    if(R < 0.25){
        R = 0.0;
    }else if(R < 0.5){
        R = 0.25;
    }else if(R < 0.75){
        R = 0.5;
    }else if(R < 1.0){
        R = 0.75;
    }else{
        R = 1.0;
    }
    //green
    if(G < 0.25){
        G = 0.0;
    }else if(G < 0.5){
        G = 0.25;
    }else if(G < 0.75){
        G = 0.5;
    }else if(G < 1.0){
        G = 0.75;
    }else{
        G = 1.0;
    }
    //blue
    if(B < 0.25){
        B = 0.0;
    }else if(B < 0.5){
        B = 0.25;
    }else if(B < 0.75){
        B = 0.5;
    }else if(B < 1.0){
        B = 0.75;
    }else{
        B = 1.0;
    }
    
    gl_FragColor = vec4(R,G,B,1.0);  //vec4(color.r, color.g, color.b, 1.0);
}
