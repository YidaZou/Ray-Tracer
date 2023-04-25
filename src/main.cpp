#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"

#include "Materials.hpp"
#include "Light.hpp"
#include "Objects.hpp"
#include "Texture.h"

using namespace std;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "../../resources"; // Where the resources are loaded from
bool OFFLINE = false;

shared_ptr<Camera> camera;
shared_ptr<Camera> cameraH; //hud camera
shared_ptr<Camera> cameraT; //top down camera
shared_ptr<Program> prog;   //normal shader
shared_ptr<Program> prog1;  //phong shader
shared_ptr<Program> prog1H; //hud shader
shared_ptr<Program> prog2;  //revo shader
shared_ptr<Program> prog3;  //cel shader
shared_ptr<Shape> ground;   //cube
shared_ptr<Shape> bunny;    //bunny
shared_ptr<Shape> teapot;   //teapot
shared_ptr<Shape> sphere;   //sphere
shared_ptr<Shape> revo;     //surface of revolution
shared_ptr<Shape> frustrum; //frustrum
shared_ptr<Texture> texture0;   //ground texture

int textureWidth = 640;
int textureHeight = 480;
GLuint framebufferID;
GLuint posTexture;
GLuint norTexture;
GLuint keTexture;
GLuint kdTexture;

double RANDOM_COLORS[7][3] = {
    {0.0000,    0.4470,    0.7410},
    {0.8500,    0.3250,    0.0980},
    {0.9290,    0.6940,    0.1250},
    {0.4940,    0.1840,    0.5560},
    {0.4660,    0.6740,    0.1880},
    {0.3010,    0.7450,    0.9330},
    {0.6350,    0.0780,    0.1840},
};

bool topDown = false;
bool keyToggles[256] = {false}; // only for English keyboards!

vector<float> randomN;
vector<float> randomScale;

//materials
vector<Materials> mats;
int i=0;    //index for materials
//shader index
vector<shared_ptr<Program>> shaders;
int s=1;    //0 = normal, 1 = phong, 2 = silhouette
//lights
glm::vec3 lights[10];
glm::vec3 lightsPos[10];
int l=0;    //index for lights
//objects
vector<Objects> objects;
//camera variables
glm::vec3 eye = glm::vec3(0,0,0);
float theta = M_PI;
glm::vec3 forw = glm::vec3(sin(theta),0,cos(theta));
float pitch = 0;
float yaw = 0;
glm::vec3 up = glm::vec3(0,1,0);

float getBaseHeight(shared_ptr<Shape> s){
    float m = MAXFLOAT;
    for(int i=1; i<s->posBuf.size(); i+=2){
        if(s->posBuf[i] < m){
            m = s->posBuf[i];
        }
    }
    
    //float m = *min_element(s->posBuf.begin(), s->posBuf.begin());
    return m;
}

// This function is called when a GLFW error occurs
static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

// This function is called when a key is pressed
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

// This function is called when the mouse is clicked
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if(action == GLFW_PRESS) {
		bool shift = (mods & GLFW_MOD_SHIFT) != 0;
		bool ctrl  = (mods & GLFW_MOD_CONTROL) != 0;
		bool alt   = (mods & GLFW_MOD_ALT) != 0;
		camera->mouseClicked((float)xmouse, (float)ymouse, shift, ctrl, alt);
	}
}

// This function is called when the mouse moves
static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if(state == GLFW_PRESS)
        camera->mouseMoved((float)xmouse, (float)ymouse);
    theta = -(camera->rotations.x);
    pitch = camera->rotations.y;
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
    keyToggles[key] = !keyToggles[key];
    float speed = 0.1;
    glm::vec3 f = glm::vec3(-sin(theta), 0, -cos(theta));
    switch(key)
    {
        case 'w':
            eye+= speed * f;
            break;
        case 'a':
            eye-= speed * glm::cross(f, up);
            break;
        case 's':
            eye-= speed * f;
            break;
        case 'd':
            eye+= speed * glm::cross(f, up);
            break;
        case 'z':
            camera->fovy -= speed * camera->fovy;
            if(camera->fovy < 4*M_PI/180){
                camera->fovy = 4*M_PI/180;
            }
            break;
        case 'Z':
            camera->fovy += speed * camera->fovy;
            if(camera->fovy > 114*M_PI/180){
                camera->fovy = 114*M_PI/180;
            }
            break;
        case 't':
            if(!topDown){
                topDown = true;
            }else{
                topDown = false;
            }
            break;
        case ',':
            
        default:
            break;
    }
}

// If the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
    textureWidth = width;
    textureHeight = height;
    
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
    //resize textures
    glBindTexture(GL_TEXTURE_2D, posTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, norTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, keTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, kdTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
    
    GLuint depthrenderbuffer;
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, textureWidth, textureHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// https://lencerf.github.io/post/2019-09-21-save-the-opengl-rendering-to-image-file/
static void saveImage(const char *filepath, GLFWwindow *w)
{
	int width, height;
	glfwGetFramebufferSize(w, &width, &height);
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	int rc = stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
	if(rc) {
		cout << "Wrote to " << filepath << endl;
	} else {
		cout << "Couldn't write to " << filepath << endl;
	}
}

// This function is called once to initialize the scene and OpenGL
static void init()
{
	// Initialize time.
	glfwSetTime(0.0);
	
	// Set background color.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

    //initialize materials
    Materials original = Materials(0.2,0.2,0.2,
                                   0.8,0.7,0.7,
                                   1.0,0.9,0.8);
    mats.push_back(original);
    Materials blue = Materials(0.1,0.1,0.5,
                               0.0,0.0,0.5,
                               0.5,1,1);
    mats.push_back(blue);
    Materials grey = Materials(0.01,0.01,0.01,
                               0.7,0.7,0.8,
                               0.0,0,0);
    mats.push_back(grey);
    
    //ground shader (OLD)
    prog = make_shared<Program>();
    prog->setShaderNames(RESOURCE_DIR + "ground_vert.glsl", RESOURCE_DIR + "ground_frag.glsl");
    prog->setVerbose(true);
    prog->init();
    prog->addAttribute("aPos");
    prog->addAttribute("aTex");
    prog->addUniform("MV");
    prog->addUniform("P");
    prog->addUniform("texture0");
    prog->setVerbose(false);
    
    //phong shader
    prog1 = make_shared<Program>();
    prog1->setShaderNames(RESOURCE_DIR + "phong_vert.glsl", RESOURCE_DIR + "pass1_frag.glsl");
    prog1->setVerbose(true);
    prog1->init();
    prog1->addAttribute("aPos");
    prog1->addAttribute("aNor");
    prog1->addUniform("MV");
    prog1->addUniform("MV_it");
    prog1->addUniform("P");
    prog1->addUniform("lightArray");
    prog1->addUniform("lightPos");
    prog1->addUniform("ke");//ambient
    prog1->addUniform("kd");//diffuse
    prog1->addUniform("ks");//specular
    prog1->addUniform("s"); //shininess
    prog1->setVerbose(false);
    
    //revo phong shader
    prog2 = make_shared<Program>();
    prog2->setShaderNames(RESOURCE_DIR + "revo_vert.glsl", RESOURCE_DIR + "pass1_frag.glsl");
    prog2->setVerbose(true);
    prog2->init();
    prog2->addAttribute("aPos");
    prog2->addAttribute("aNor");
    prog2->addUniform("MV");
    prog2->addUniform("MV_it");
    prog2->addUniform("P");
    prog2->addUniform("t");
    prog2->addUniform("lightArray");
    prog2->addUniform("lightPos");
    prog2->addUniform("ke");//ambient
    prog2->addUniform("kd");//diffuse
    prog2->addUniform("ks");//specular
    prog2->addUniform("s"); //shininess
    prog2->setVerbose(false);
    
    //pass2 shader
    prog3 = make_shared<Program>();
    prog3->setShaderNames(RESOURCE_DIR + "pass2_vert.glsl", RESOURCE_DIR + "phong_frag.glsl");
    prog3->setVerbose(true);
    prog3->init();
    prog3->addAttribute("aPos");
    prog3->addAttribute("aNor");
    prog3->addUniform("MV");
    prog3->addUniform("MV_it");
    prog3->addUniform("P");
    prog3->addUniform("lightArray");
    prog3->addUniform("lightPos");
    prog3->addUniform("ke");//ambient
    prog3->addUniform("kd");//diffuse
    prog3->addUniform("ks");//specular
    prog3->addUniform("s"); //shininess
    prog3->setVerbose(false);
    prog3->addUniform("posTexture");
    prog3->addUniform("norTexture");
    prog3->addUniform("keTexture");
    prog3->addUniform("kdTexture");
    prog3->addUniform("windowSize");
    prog3->bind();
    glUniform1i(prog3->getUniform("posTexture"), 0);
    glUniform1i(prog3->getUniform("norTexture"), 1);
    glUniform1i(prog3->getUniform("keTexture"), 2);
    glUniform1i(prog3->getUniform("kdTexture"), 3);
    prog3->unbind();

    shaders.push_back(prog);
    shaders.push_back(prog1);
    shaders.push_back(prog2);
	
	camera = make_shared<Camera>();
	camera->setInitDistance(15.0f); // Camera's initial Z translation
    
    cameraH = make_shared<Camera>();
    cameraH->setInitDistance(0.0f); // Camera's initial Z translation
    
    cameraT = make_shared<Camera>();
    cameraT->setInitDistance(10.0f); // Camera's initial Z translation
	
    //ground (square)
    ground = make_shared<Shape>();
    ground->loadMesh(RESOURCE_DIR + "square.obj");
    ground->init();
    
    //bunny
	bunny = make_shared<Shape>();
    bunny->loadMesh(RESOURCE_DIR + "bunny.obj");
    bunny->init();
    
    //teapot
    teapot = make_shared<Shape>();
    teapot->loadMesh(RESOURCE_DIR + "teapot.obj");
    teapot->init();
    
    //sphere
    sphere = make_shared<Shape>();
    //sphere->loadMesh(RESOURCE_DIR + "sphere.obj");
    sphere->createSphere(0.5);
    sphere->init();
    
    //surface of revolution
    revo = make_shared<Shape>();
    //sphere->loadMesh(RESOURCE_DIR + "sphere.obj");
    revo->createRevo();
    revo->init();

    //initialize lights
    for(int i=0; i<10; i++){
        glm::vec3 lght = glm::vec3(0.8+(i%5)/3,0.8+(i%6)/4,0.8+(i%4)/2);
        lights[i] = (lght);
        glm::vec3 lghtPos = glm::vec3(-5+(3*i)%10 + 0.5, -0.8, -5+i + 0.5);
        lightsPos[i] = (lghtPos);
    }
    
	//positions
    for(int i=0; i<10; i++){
        for(int j=0; j<10; j++){
            randomN.push_back(-5+i + (rand()%4)/5);
            randomN.push_back(-5+j + (rand()%4)/5);
        }
    }
    
    randomScale.push_back(0.3);
    randomScale.push_back(0.4);
    randomScale.push_back(0.5);
    randomScale.push_back(0.35);
    
    //Objects
    auto bunnyObjects = Objects(0,0,0,
                                1,0,0,
                                0,0,0,bunny);
    auto teapotObjects = Objects(0,0,0,
                                1,0,0,
                                0,0,0,teapot);
    auto sphereObjects = Objects(0,0,0,
                                1,0,0,
                                0,0,0,sphere);
    auto revoObjects = Objects(0,0,0,
                                1,0,0,
                                0,0,0,revo);
    objects.push_back(bunnyObjects);
    objects.push_back(teapotObjects);
    objects.push_back(sphereObjects);
    objects.push_back(revoObjects);
    
    //set textures to window size on open
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    textureWidth = width;
    textureHeight = height;
    
    glGenFramebuffers(1, &framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
    GLSL::checkError(GET_FILE_LINE);
    //position texture
    glGenTextures(1, &posTexture);
    glBindTexture(GL_TEXTURE_2D, posTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, posTexture, 0);
    GLSL::checkError(GET_FILE_LINE);
    //normal texture
    glGenTextures(1, &norTexture);
    glBindTexture(GL_TEXTURE_2D, norTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, norTexture, 0);
    //ke texture
    glGenTextures(1, &keTexture);
    glBindTexture(GL_TEXTURE_2D, keTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, keTexture, 0);
    //kd texture
    glGenTextures(1, &kdTexture);
    glBindTexture(GL_TEXTURE_2D, kdTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, kdTexture, 0);
    GLSL::checkError(GET_FILE_LINE);
    
    GLuint depthrenderbuffer;
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, textureWidth, textureHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
    
    GLenum attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, attachments);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        cerr << "Framebuffer is not ok" << endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
	GLSL::checkError(GET_FILE_LINE);
}

// This function is called every frame to draw the scene.
static void render()
{
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles[(unsigned)'q']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
    
	//RENDER TO FRAME BUFFER
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
    glViewport(0, 0, textureWidth, textureHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	double t = glfwGetTime();
    
	if(keyToggles[(unsigned)' ']) {
		// Spacebar turns animation on/off
		t = 0.0f;
	}
	
	// Matrix stacks
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();

    auto MV_it = glm::inverse(glm::transpose((MV->topMatrix())));
    
    P->pushMatrix();
    camera->applyProjectionMatrix(P);
    MV->pushMatrix();
    camera->applyViewMatrix(MV, eye, forw, pitch);
     
    //lights
    glm::vec3 lightWorld[10];   //light pos in world
    for(int i=0; i<10; i++){
        lightWorld[i] = (MV->topMatrix() * glm::vec4(lightsPos[i],1));
    }
    prog1->bind();
    for(int i=0; i<10; i++){
        glUniformMatrix4fv(prog1->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
        MV->pushMatrix();
        MV->translate(lightsPos[i]);
        MV->scale(0.1);

        MV_it = glm::inverse(glm::transpose((MV->topMatrix())));
        glUniformMatrix4fv(prog1->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
        glUniformMatrix4fv(prog1->getUniform("MV_it"), 1, GL_FALSE, glm::value_ptr(MV_it));
        glUniform3fv(prog1->getUniform("ke"), 1, glm::value_ptr(lights[i]));  //ambient color
        glUniform3f(prog1->getUniform("kd"), 0,0,0);  //diffuse color
        glUniform3f(prog1->getUniform("ks"), 0,0,0);  //specular color
        glUniform3fv(prog1->getUniform("lightArray"), 10, glm::value_ptr(lights[0]));
        glUniform3fv(prog1->getUniform("lightPos"), 10, glm::value_ptr(lightWorld[0])); //lightPos in world
        sphere->draw(prog1);
        MV->popMatrix();
    }
    prog1->unbind();
    
    //GROUND
    prog1->bind();
    MV->pushMatrix();
    glUniformMatrix4fv(prog1->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
    //texture0->bind(prog->getUniform("texture0"));
    MV->translate(0.0f, -1.0f, 0.0f);
    MV->scale(100);
    MV->rotate(-M_PI/2, 1,0,0);
    MV_it = glm::inverse(glm::transpose((MV->topMatrix())));
    glUniformMatrix4fv(prog1->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    glUniformMatrix4fv(prog1->getUniform("MV_it"), 1, GL_FALSE, glm::value_ptr(MV_it));
    glUniform3f(prog1->getUniform("ke"), 0,0,0);  //ambient color
    glUniform3f(prog1->getUniform("kd"), 1,1,1);  //diffuse color
    glUniform3f(prog1->getUniform("ks"), 1,1,1);  //specular color
    glUniform3fv(prog1->getUniform("lightArray"), 10, glm::value_ptr(lights[0]));
    glUniform3fv(prog1->getUniform("lightPos"), 10, glm::value_ptr(lightWorld[0])); //lightPos in world
    glUniform1f(prog1->getUniform("s"), 10.0f);   //shininess factor
    ground->draw(prog1);
    //texture0->unbind();
    MV->popMatrix();
    prog1->unbind();
    
    unsigned int posI = 0;
    unsigned int n = 25;
    
    for(int i=0; i<n; i++){
        for(int o=0; o<4; o++){
            MV->pushMatrix();
            float x = randomN[posI];
            float z = randomN[posI+1];
            posI += 2;
            MV->translate(x, 0, z);
            MV->translate(0.0f, -1.0 - (objects[o].s->baseY)*randomScale[i%4], 0.0f);  //align with ground
            if(o==2){   //bouncing sphere
                MV->translate(0, 0.3*(0.5*sin((2*M_PI/1.7)*(t+0.9))+0.5), 0);
                MV->scale(-0.5*(0.5*cos((4*M_PI/1.7)*(t+0.9))+0.5)+1, 1, -0.5*(0.5*cos((4*M_PI/1.7)*(t+0.9))+0.5)+1);
            }
            if(o==1){   //teapot shear
                glm::mat4 S(1.0f);
                S[1][2] = 0.5f*cos(t);
                MV->multMatrix(S);
            }
            if(o==0){   //bunny rotate
                MV->rotate(t, 0,1,0);
            }
            MV->scale(randomScale[i%4]);
            if(o==3){   //surface of revolution
                prog2->bind();
                MV->pushMatrix();
                MV->rotate(M_PI/2, 0,0,1);
                MV->scale(0.2);
                MV_it = glm::inverse(glm::transpose((MV->topMatrix())));
                glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
                glUniformMatrix4fv(prog2->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
                glUniformMatrix4fv(prog2->getUniform("MV_it"), 1, GL_FALSE, glm::value_ptr(MV_it));
                glUniform1f(prog2->getUniform("t"), t);  //time
                glUniform3f(prog2->getUniform("ke"), 0,0,0);  //ambient color
                glUniform3f(prog2->getUniform("kd"), RANDOM_COLORS[(i+1)%7][0], RANDOM_COLORS[(i+1)%7][1], RANDOM_COLORS[(i+1)%7][2]);  //diffuse color
                glUniform3f(prog2->getUniform("ks"), 1,1,1);  //specular color
                glUniform3fv(prog2->getUniform("lightArray"), 10, glm::value_ptr(lights[0]));
                glUniform3fv(prog2->getUniform("lightPos"), 10, glm::value_ptr(lightWorld[0])); //lightPos in world
                glUniform1f(prog2->getUniform("s"), 10.0f);   //shininess factor
                objects[o].s->draw(prog2);
                MV->popMatrix();
                prog2->unbind();
            }else{  //other objects
                prog1->bind();
                MV_it = glm::inverse(glm::transpose((MV->topMatrix())));
                glUniformMatrix4fv(prog1->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
                glUniformMatrix4fv(prog1->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
                glUniformMatrix4fv(prog1->getUniform("MV_it"), 1, GL_FALSE, glm::value_ptr(MV_it));
                glUniform3f(prog1->getUniform("ke"), 0,0,0);  //ambient color
                glUniform3f(prog1->getUniform("kd"), RANDOM_COLORS[(i+1)%7][0], RANDOM_COLORS[(i+1)%7][1], RANDOM_COLORS[(i+1)%7][2]);  //diffuse color
                glUniform3f(prog1->getUniform("ks"), 1, 1, 1);  //specular color
                glUniform3fv(prog1->getUniform("lightArray"), 10, glm::value_ptr(lights[0]));
                glUniform3fv(prog1->getUniform("lightPos"), 10, glm::value_ptr(lightWorld[0])); //lightPos in world
                glUniform1f(prog1->getUniform("s"), 10.0f);   //shininess factor
                objects[o].s->draw(prog1);
                prog1->unbind();
            }
            MV->popMatrix();
        }
    }
	MV->popMatrix();
    P->popMatrix();
    
    //RENDER TO SCREEN
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    camera->setAspect((float)width/(float)height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    prog3->bind();
    P->pushMatrix();
    MV->pushMatrix();
    //MV->rotate(M_PI, 0,1,0);
    MV->scale(2);
    glUniformMatrix4fv(prog3->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
    glUniformMatrix4fv(prog3->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
    glUniform3f(prog3->getUniform("ks"), 1,1,1);  //specular color
    glUniform3fv(prog3->getUniform("lightArray"), 10, glm::value_ptr(lights[0]));
    glUniform3fv(prog3->getUniform("lightPos"), 10, glm::value_ptr(lightWorld[0])); //lightPos in world
    glUniform1f(prog3->getUniform("s"), 10.0f);   //shininess factor
    glm::vec2 windowSize(textureWidth, textureHeight);
    glUniform2fv(prog3->getUniform("windowSize"), 1, glm::value_ptr(windowSize));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, posTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, norTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, keTexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, kdTexture);
    glActiveTexture(GL_TEXTURE0);
    ground->draw(prog3);
    prog3->unbind();
    MV->popMatrix();
    P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
	
	if(OFFLINE) {
		saveImage("output.png", window);
		GLSL::checkError(GET_FILE_LINE);
		glfwSetWindowShouldClose(window, true);
	}
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Usage: A3 RESOURCE_DIR" << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	
	// Optional argument
	if(argc >= 3) {
		OFFLINE = atoi(argv[2]) != 0;
	}

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "YIDA ZOU", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	GLSL::checkVersion();
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Set the window resize call back.
	glfwSetFramebufferSizeCallback(window, resize_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
