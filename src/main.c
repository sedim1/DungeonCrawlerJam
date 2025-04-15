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
void draw2DProjection();
void draw3DProjection();

void frameBufferSizeCallback(GLFWwindow* window,int w,int h);
void ProcessInput();

//Player functions
void PlayerInput();


//Window
GLFWwindow* window;

//Map
Map2D map;
//Entities in the map
Entity player;

//Screen Size
int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 640;
float distToProjPlane;
float angleStep;

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
    player.position.x = 128 - 32;
    player.position.y = 128 - 32;
	    player.angle = 0.02f;

	distToProjPlane = ((SCREEN_WIDTH / 2.0f)) / tan((degToRad(FOV)/2.0f));
	angleStep = FOV / SCREEN_WIDTH;

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
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if( map.projection == MAP2D)
	    draw2DProjection();
    if( map.projection == PROJECTION3D)
	    draw3DProjection();
    glfwPollEvents();
    glfwSwapBuffers(window);
}

void draw2DProjection(){
    drawMap2D(&map);
    drawRays3D(&map,&player);
    drawEntityOnMap(&player);
}

void draw3DProjection(){
	int rays = 0;
        float distH,distV,dist;
        Entity r = player;//The ray represented as an entity;
        r.angle = player.angle+(RAYS/2); r.angle = angleAdjust(r.angle);
        //Get the position in cell cord where the ray starts to get caste
        for(rays = 0; rays < RAYS ; rays++){
                VECTOR2D rayH = castRayH(&map,&r);
                VECTOR2D rayV = castRayV(&map,&r);
                VECTOR2D ray;
                distH = length(&player.position,&rayH);                                                                                                                     
                distV = length(&player.position,&rayV);                                                                                                                     
                ray = (distH < distV) ? rayH : rayV;                                                                                                                         
                dist = (distH < distV) ? distH : distV; //Save distance of the selected wall
                float shadow = (distH < distV) ? 1.0f : 0.5f; //Save distance of the selected wall
		float ca = player.angle - r.angle; ca = angleAdjust(ca);
		dist = dist * cos(degToRad(ca));
		//Draw 3D WALSS
		float vOffset = PROJECTION_HEIGHT/2;
		float projectedSliceHeight = (map.mapSize / dist) * distToProjPlane;
		float topPoint = (int)(vOffset - projectedSliceHeight/2);
		float bottomPoint = (int)(vOffset + projectedSliceHeight/2);
		if(bottomPoint>PROJECTION_HEIGHT)
			bottomPoint = PROJECTION_HEIGHT;
		glColor3f(shadow,0.0f,0.0f);glPointSize(PIXEL_SIZE);
		//Draw points of the wall
		for(int y = topPoint; y <= bottomPoint; y++){
			glBegin(GL_POINTS);
			glVertex2i(80 + rays * PIXEL_SIZE,y);
			glEnd();
		}
                r.angle -= 1.0f; r.angle = angleAdjust(r.angle);
        }
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
    if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	    map.projection = MAP2D;
    if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	    map.projection = PROJECTION3D;

    PlayerInput();
}

void PlayerInput(){
    float dx = cos(degToRad(player.angle));
    float dy = -sin(degToRad(player.angle));
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        player.position.x += dx * 4;
        player.position.y += dy * 4;
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        player.position.x -= dx * 4;
        player.position.y -= dy * 4;
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
        player.angle +=1.0f;
    	player.angle = angleAdjust(player.angle);
    }
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
        player.angle -= 1.0f;
    	player.angle = angleAdjust(player.angle);
    }
}
