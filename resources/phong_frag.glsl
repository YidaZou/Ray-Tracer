#version 120

uniform vec3 ka;
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
    vec2 tex;
    tex.x = gl_FragCoord.x/windowSize.x;
    tex.y = gl_FragCoord.y/windowSize.y;
    
    //shading data
    vec3 vPos = texture2D(posTexture, tex).rgb;
    vec3 n = texture2D(norTexture, tex).rgb;
    vec3 ke = texture2D(keTexture, tex).rgb;
    vec3 kd = texture2D(kdTexture, tex).rgb;
    
    float A0 = 1.0;
    float A1 = 0.0429;
    float A2 = 0.9857;
    
    vec3 eyeVec = normalize(-1 * vPos);
    vec3 finalColor = ke;
    
    //lights
    for(int i=0; i<10; i++){
        float r = distance(lightPos[i], vPos);
        vec3 l1 = normalize(lightPos[i] - vPos);
        vec3 cd1 = kd * max(0, dot(l1, n));
        vec3 h1 = normalize(l1 + eyeVec);///sqrt(dot(l+eyeVec,l+eyeVec));
        vec3 cs1 = ks * pow(max(0,dot(h1,n)), s);
        vec3 color = lightArray[i] * (cd1 + cs1);
        float attenuation = 1.0 / (A0 + A1*r + A2*r*r);
        finalColor += color * attenuation;
    }
    gl_FragColor = vec4(finalColor,1.0);  //vec4(color.r, color.g, color.b, 1.0);
    
    //UNCOMMENT BELOW LINES FOR THE 4 RESPECTIVE IMAGES
    //gl_FragColor.rgb = vPos;  //position
    //gl_FragColor.rgb = n;     //normal
    //gl_FragColor.rgb = ke;    //emissive
    //gl_FragColor.rgb = kd;    //diffuse
}
