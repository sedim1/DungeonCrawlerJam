#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

bool init();
void end();
void Update();
void Draw();

void frameBufferSizeCallback(GLFWwindow* window,int w,int h);
void ProcessInput();

//Screen Size
int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 640;
//Window
GLFWwindow* window;

int main(int argc,char* argv[]){
    if(!init())
        return -1;
    Update();
    end();
    return 0;
}

bool init(){
    //Init glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //Start window
    window = glfwCreateWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"DungeonCrawlerJam2025",NULL,NULL);
    if(!window){
        printf("ERROR::FAILED TO START GLFW::\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    //StartGlad
    if(!gladLoadGLLoader ((GLADloadproc)glfwGetProcAddress)){
        printf("ERROR::FAILED TO LOAD GLAD::\n");
        return false;
    }
    //Start viewport
    glViewport(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0,SCREEN_WIDTH,SCREEN_HEIGHT,0,-1,1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    

    glEnable(GL_DEPTH_TEST);

    //Set Callbacks
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback); 

    return true;
}

void end(){
    glfwTerminate();
}

void Update(){
    while(!glfwWindowShouldClose(window)){
        ProcessInput();
        //Draw on Screen
        Draw();
    }
}

void Draw(){
    glClearColor(0.2f,0.3f,0.3f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(SCREEN_WIDTH/2, 0);
    glVertex2f(0, SCREEN_HEIGHT);
    glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnd();
    glfwPollEvents();
    glfwSwapBuffers(window);
}

void frameBufferSizeCallback(GLFWwindow* window,int w,int h){
    SCREEN_WIDTH = w;
    SCREEN_HEIGHT = h;
    glViewport(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0,SCREEN_WIDTH,SCREEN_HEIGHT,0,-1,1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void ProcessInput(){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

