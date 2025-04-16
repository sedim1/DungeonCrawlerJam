#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "Raycaster.h"
#define SPEED 50.0
#define RSPEED 380.0

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
void playerUpdate();
void rotateView();
void rotateLeft();
void rotateRight();
bool dirPressed();
bool movePressed();
void movePlayer();
bool wallCollision();

//Window
GLFWwindow* window;

//Map
Map2D map;
//Entities in the map
Entity player;
enum DIRECTION playerDir;
enum DIRECTION keyPressed;
bool canTurn = true;
bool canMove = true;
float tPos = 0.0f;
int turn; //1 right - -1 left

//Screen Size
int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 640;
float distToProjPlane;

//For deltaTime
float deltaTime;
float currentTime = 0.0f;
float lastTime = 0.0f;



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
    

    glEnable(GL_DEPTH_TEST);

    //Set Callbacks
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback); 

    //Load map
    map = loadMap("Map/testMap.txt",MAP2D);

    //Set Up entities in the world
    player.position = cellCordToCartesian(3,1,64);
	    player.angle = 0.0f;
	    playerDir = RIGHT;

	distToProjPlane = ((SCREEN_WIDTH / 2.0f)) / tan((degToRad(FOV)/2.0f));

	return true;
}

void end(){
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Update(){
    while(!glfwWindowShouldClose(window)){
	//calculate deltatime
	currentTime =  (float)glfwGetTime();
	deltaTime = currentTime - lastTime;
	lastTime = currentTime;
        ProcessInput();
	playerUpdate();
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
    drawEntityOnMap(&player);
    drawMap2D(&map);
}

void draw3DProjection(){
	int rays = 0;
        float distH,distV,dist;
        Entity r = player;//The ray represented as an entity;
        r.angle = player.angle+(FOV/2); r.angle = angleAdjust(r.angle);
        float angleStep = (float)FOV / (float)RAYS;
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
		float hOffset = PROJECTION_WIDTH/2;
		float projectedSliceHeight = (map.mapSize / dist) * distToProjPlane;
		float topPoint = (int)(vOffset - projectedSliceHeight/2);
		float bottomPoint = (int)(vOffset + projectedSliceHeight/2);
		if(bottomPoint>PROJECTION_HEIGHT)
			bottomPoint = PROJECTION_HEIGHT;
		glColor3f(shadow,0.0f,0.0f);glPointSize(PIXEL_SIZE);
		//Draw points of the wall
		for(int y = topPoint; y <= bottomPoint; y++){
			glBegin(GL_POINTS);
			glVertex2i(rays * PIXEL_SIZE,y);
			glEnd();
		}
                r.angle -= angleStep; r.angle = angleAdjust(r.angle);
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

}

void playerUpdate(){
	if(player.state == ROTATING){
		if(turn == 1)
			rotateLeft();
		else if(turn == -1)
			rotateRight();
	}
	else if(player.state == MOVING){
		movePlayer();
	}
	else if(player.state == IDLE){
		if(!canTurn)
			canTurn = !dirPressed();
		if(!canMove)
			canMove = !movePressed();
    		PlayerInput();
	}
}

bool movePressed(){
    return glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;
}

bool dirPressed(){
    return glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
}

void rotateLeft(){
	bool finished = false;
	switch(playerDir){
		case 0://UP
			player.angle += RSPEED * deltaTime;
			if(player.angle >= 90.0f){
				finished = true; player.angle = 90.0f;}
			break;
		case 1://DOWN
			player.angle += RSPEED * deltaTime;
			if(player.angle >= 270.0f){
				finished = true;player.angle = 270.0f;}
			break;
		case 2://LEFT
			player.angle += RSPEED * deltaTime;
			if(player.angle >= 180.0f){
				finished = true; player.angle = 180.0f;}
			break;
		case 3://RIGHT
			player.angle += RSPEED * deltaTime;
			if(player.angle >= 360.0f){
				finished = true; player.angle = 360.0f;
				player.angle = angleAdjust(player.angle);
			}
			break;
	}
	if(finished)
		player.state = IDLE;

}

void rotateRight(){
	 bool finished = false;
        switch(playerDir){ 
                case 0://UP
                        player.angle -= RSPEED * deltaTime;                               
                        if(player.angle <= 90.0f){
                                finished = true; player.angle = 90.0f;}
                        break;
                case 1://DOWN
                        player.angle -= RSPEED * deltaTime;                               
        		player.angle = angleAdjust(player.angle);
                        if(player.angle <= 270.0f){
                                finished = true;player.angle = 270.0f;}
                        break;
                case 2://LEFT
                        player.angle -= RSPEED * deltaTime;                                
                        if(player.angle <= 180.0f){
                                finished = true; player.angle = 180.0f;}
                        break;
                case 3://RIGHT
                        player.angle -= RSPEED * deltaTime;                               
                        if(player.angle <= 0.0f){
                                finished = true; player.angle = 0.0f;
				player.angle = angleAdjust(player.angle);
			}
                        break;
        }
        if(finished)
                player.state = IDLE;

}

void movePlayer(){
	player.position = lerp(&player.currentPos,&player.nextPos,tPos);
	tPos += 0.1f * SPEED * deltaTime;
	if(tPos>=1.0f){
		player.position = player.nextPos;
		player.state = IDLE;
	}
}
bool wallPlayerCollision(){
	CellCord gridPos = cartesianToCellCords(player.position.x,player.position.y,CELLSIZE);// get the position on x and y according to the position of player
	switch(playerDir){
		case 0://UP
		       if(keyPressed == UP)
				gridPos.y -=1;
		       else
			       gridPos.y += 1;
			break;
		case 1://DOWN
		       if(keyPressed == UP)
				gridPos.y +=1;
		       else
			       gridPos.y -= 1;
			break;
		      
		case 2://LEFT
		       if(keyPressed == UP)
				gridPos.x -=1;
		       else
			       gridPos.x += 1;
			break;
		case 3://RIGHT
		       if(keyPressed == UP)
				gridPos.x +=1;
		       else
			       gridPos.x -= 1;
			break;
	}
	int mPos = gridPos.y * map.mapWidth + gridPos.x;// convert int into position on array
	if(mPos >= 0 && mPos < map.mapWidth*map.mapHeight && map.buffer[mPos] == 1)
		return true;
	return false;
}

void PlayerInput(){
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && canMove){
	    keyPressed = UP;
	    if(wallPlayerCollision())
		    return;
	    player.state = MOVING;
	    canMove = false;
	    tPos = 0.0f;
	    player.currentPos = player.position; player.nextPos = player.position;
	    switch(playerDir){ //check current direction
                    case 0: //UP
                            player.nextPos.y -= CELLSIZE;
                            break;
                    case 1: //DOWN  
                            player.nextPos.y += CELLSIZE;
                            break;
                    case 2: //LEFT
                            player.nextPos.x -= CELLSIZE;
                            break;
                    case 3: //RIGHT
                            player.nextPos.x  += CELLSIZE;
                            break;
            }
    }
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && canMove){
	    keyPressed = DOWN;
	    if(wallPlayerCollision())
		    return;
	player.state = MOVING;
	canMove = false;
	tPos = 0.0f;
	tPos = 0.0f;
            player.currentPos = player.position; player.nextPos = player.position;
            switch(playerDir){ //check current direction
                    case 0: //UP
                            player.nextPos.y += CELLSIZE;
                            break;
                    case 1: //DOWN  
                            player.nextPos.y -= CELLSIZE;
                            break;
                    case 2: //LEFT
                            player.nextPos.x += CELLSIZE;
                            break;
                    case 3: //RIGHT
                            player.nextPos.x  -= CELLSIZE;
                            break;
            }
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && canTurn){ //Turn left
	    switch(playerDir){ //check current direction
		    case 0: //UP
			    playerDir = LEFT;
			    break;
		    case 1: //DOWN
			    playerDir = RIGHT;
			    break;
		    case 2: //LEFT
			    playerDir = DOWN;
			    break;
		    case 3: //RIGHT
			    playerDir = UP;
			    break;
	    }
	    turn = 1;
	    player.state = ROTATING;
	    canTurn = false;
    }
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && canTurn){ //TURN RIGHT
	    switch(playerDir){ //check current direction
                    case 0: //UP
                            playerDir = RIGHT;
                            break;
                    case 1: //DOWN  
                            playerDir = LEFT;
                            break;
                    case 2: //LEFT
                            playerDir = UP;
                            break;
                    case 3: //RIGHT
                            playerDir = DOWN;  
                            break;
            }
	    turn = -1;
	    player.state = ROTATING;
	    canTurn = false;
    }
}
