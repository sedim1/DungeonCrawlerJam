#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "Raycaster.h"


bool init();
void end();
void Update();
void Draw();

void frameBufferSizeCallback(GLFWwindow* window,int w,int h);
void ProcessInput();

//Player functions
void PlayerInput();

//Screen Size
int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 640;


//Window
GLFWwindow* window;

//Map
Map2D map;
//Entities in the map
Entity player;

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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
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
    //Start viewport and matrices
    glViewport(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0,SCREEN_WIDTH,SCREEN_HEIGHT,0,-1,1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    

    //glEnable(GL_DEPTH_TEST);

    //Set Callbacks
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback); 

    //Load map
    map = loadMap("Map/testMap.txt",MAP2D);

    //Set Up entities in the world
    player.xPos = 128 - ((map.mapSize/2));
    player.yPos = 128 - ((map.mapSize/2));
    player.angle = 90.0f;

    return true;
}

void end(){
    glfwDestroyWindow(window);
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
    drawMap2D(&map);
    drawEntityOnMap(&player);
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

    PlayerInput();
}

void PlayerInput(){
    float dx = cos(degToRad(&player.angle));
    float dy = -sin(degToRad(&player.angle));
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        player.xPos += dx * 4;
        player.yPos += dy * 4;
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        player.xPos -= dx * 4;
        player.yPos -= dy * 4;
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        player.angle +=2.0f;
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        player.angle -= 2.0f;
}

