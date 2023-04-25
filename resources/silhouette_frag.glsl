#version 120

varying vec3 vPos;
varying vec3 normal;    //camera normal
varying vec3 fragPos;   //interpolated fragment position

void main()
{
    vec3 n = normalize(normal);
    vec3 eyeVec = normalize(-1 * vPos);
    
    float threshold = dot(n, eyeVec);
    if(threshold < 0.3){
        gl_FragColor = vec4(0,0,0,1.0);  //vec4(color.r, color.g, color.b, 1.0);
    }else{
        gl_FragColor = vec4(255,255,255,1.0);
    }
}
