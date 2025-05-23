#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "Raycaster.h"
#include "BFS.h"
#include "Textures/MapChecker.h"
#include "Textures/walls.h"
#include "Textures/floors.h"
#include "Textures/ceiling.ppm"
#define SPEED 30.0
#define RSPEED 320.0
#define ENEMY_SPEED 50

bool init();
void end();
void Update();
void Draw();
void draw2DProjection();
void draw3DProjection();
void initScene();

void frameBufferSizeCallback(GLFWwindow* window,int w,int h);
void ProcessInput();

void draw2DSprite(Sprite* s);

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

//MainEnemyFunctions
void mainEnemyUpdate();
void moveEnemy();
void passiveState(); //Will just be traveling to points of the map
void agressiveState();// Will be following the player

//Window
GLFWwindow* window;

//Map
Map2D map;
Map2D floors;
Map2D ceiling;
//Entities in the map
Entity player;
enum DIRECTION playerDir;
enum DIRECTION keyPressed;
bool canTurn = true;
bool canMove = true;
float tPos = 0.0f;
int turn; //1 right - -1 left
//Enemy wolf
Entity mainEnemy;
int enemyState = 0; //Normal passive state, agressive state
Sprite enemy;
float tePos = 0.0f;
int point = 0;
float waitTime = 0.5f;
float eTmove = 0.0f;
float etCurrent = 0.0f;
CellCord path[5] = {{1,1},{2,5},{6,7},{15,5},{5,1}}; //POints where the enemy will be traversing in normal state


//Screen Size
int SCREEN_WIDTH = 768;
int SCREEN_HEIGHT = 640;
float distToProjPlane;

//For deltaTime
float deltaTime;
float currentTime = 0.0f;
float lastTime = 0.0f;




int main(int argc,char* argv[]){
    if(!init())
        return -1;
    initScene();
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
    map = loadMap("Map/walls.txt",MAP2D);
    floors = loadMap("Map/floors.txt",MAP2D);
    ceiling = loadMap("Map/ceiling.txt",MAP2D);

	distToProjPlane = ((SCREEN_WIDTH / 2.0f)) / tan((degToRad(FOV)/2.0f));
	return true;
}

void initScene(){
	    //Set Up enemy
	    player.position = cellCordToCartesian(3,1,map.mapSize);
	    player.angle = 0.0f;
	    playerDir = RIGHT;
	    player.state = IDLE;
	    //SetUp Enemy
	    mainEnemy.position = cellCordToCartesian(6,7,map.mapSize);
	    mainEnemy.currentPos = mainEnemy.position;
	    CellCord start  = {6,7}; CellCord end  = {3,1};
	    CellCord nextPos = getNextPosition(&map,&start,&end);
	    mainEnemy.nextPos = cellCordToCartesian(nextPos.x,nextPos.y,map.mapSize);
	    mainEnemy.state = IDLE;
	    eTmove = (float)glfwGetTime();
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
	mainEnemyUpdate();
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
    if( map.projection == PROJECTION3D){
	    draw2DSprite(&enemy);
	    draw3DProjection();
    }
    glfwPollEvents();
    glfwSwapBuffers(window);
}

void draw2DProjection(){
    	drawEntityOnMap(&player,1.0f,1.0f,0.0f);
    	drawEntityOnMap(&mainEnemy,0.4f,0.0f,1.0f);
    	drawMap2D(&map);
}

void draw2DSprite(Sprite* s){
	float sx = s->x - player.position.x;
	float sy = s->y - player.position.y;
        float sz = s->z;

	float CS = cos(degToRad(player.angle));
	float SN = sin(degToRad(player.angle));
	float a = sx * CS - sy * SN;
	float b = sy * SN + sy * CS;
	sx = a; sy = b;

	sx = (sx * 108.0/sy)+(120/2);
	sy = (sz * 108.0/sy)+(80/2);

	//Convert to screen space (x,y)
	printf("sx %d sy%d\n",sx,sy);
	glPointSize(PIXEL_SIZE); glColor3f(1,1,0);glBegin(GL_POINTS);glVertex2i(sx * PIXEL_SIZE,sy * PIXEL_SIZE);glEnd();
}

void draw3DProjection(){
	float cr = 0.5f, cg = 0.6, cb =  0.2;
	int rays = 0;
        float distH,distV,dist;
        Entity r = player;//The ray represented as an entity;
        r.angle = player.angle+(FOV/2); r.angle = angleAdjust(r.angle);
        float angleStep = (float)FOV / (float)RAYS;
        //Get the position in cell cord where the ray starts to get caste
        for(rays = 0; rays <= RAYS ; rays++){
		int vtm =0; int vhm = 0;
		float red = 0.0f, green = 0.0f, blue = 0.0f; int pixel = 0;
                VECTOR2D rayH = castRayH(&map,&r,&vhm);
                VECTOR2D rayV = castRayV(&map,&r,&vtm);
                VECTOR2D ray;
                distH = length(&player.position,&rayH);                                                                                                                     
                distV = length(&player.position,&rayV);                                                                                                                     
                ray = (distV < distH) ? rayV : rayH;                                                                                                                         
                int mapVal = (distV < distH) ? vtm : vhm;                                                                                                                         
                dist = (distH < distV) ? distH : distV; //Save distance of the selected wall
                float shadow = (distH < distV) ? 1.0f : 0.5f; //Save distance of the selected wall
		float ca = player.angle - r.angle; ca = angleAdjust(ca);
		float correctedDist = dist * cos(degToRad(ca));
		//Draw 3D WALSS
		float vOffset = PROJECTION_HEIGHT/2;
		float hOffset = PROJECTION_WIDTH/2;
		float projectedSliceHeight = (map.mapSize / correctedDist) * distToProjPlane;
		float topPoint = (int)(vOffset - projectedSliceHeight/2);
		float bottomPoint = (int)(vOffset + projectedSliceHeight/2);
		if(bottomPoint>PROJECTION_HEIGHT){
			bottomPoint = PROJECTION_HEIGHT;
		}
		float ambient = fogFactor(correctedDist);

		//Get Texture map
		float ty = mapVal * TEXTURE_RESOLUTION; ; float tyOffset = 0;
		float tyStep = (float)TEXTURE_RESOLUTION / projectedSliceHeight;
		float tx;
		if(shadow == 1.0f){ tx = (int)(ray.x/MAP_SCALE)%TEXTURE_RESOLUTION;if(r.angle > 180.0f){tx=TEXTURE_RESOLUTION - 1 - tx;}}
		else if(shadow == 0.5f){ tx = (int)(ray.y/MAP_SCALE)%TEXTURE_RESOLUTION;if(r.angle > 90.0f && r.angle < 270.0f){tx=TEXTURE_RESOLUTION - 1 - tx;}}
		glPointSize(PIXEL_SIZE);
		//Draw the walls
		for(int y = topPoint; y < bottomPoint; y++){
			pixel = ((int)ty*TEXTURE_RESOLUTION+(int)tx) * 3;
			red = (wallTextures[pixel+0]);
			green = (wallTextures[pixel+1]);
			blue = (wallTextures[pixel+2]);
			glColor3f((red / 255.0f) * ambient * cr,(green / 255.0f) * ambient * cg,(blue / 255.0f) * ambient * cb);
			glBegin(GL_POINTS);
			glVertex2i(rays * PIXEL_SIZE,y);
			glEnd();
			ty+=tyStep;
		}
		int i = 0;
		//Draw the floor
		for(int y = bottomPoint;y < PROJECTION_HEIGHT;y++){
			//Draw floor
			float dy = y-(PROJECTION_HEIGHT/2.0f); float raFixed = cos(degToRad(angleAdjust(player.angle-r.angle)));
			float deg = degToRad(r.angle);
			float worldX = player.position.x / MAP_SCALE + cos(deg) * FLOOR_SCALE * TEXTURE_RESOLUTION / dy / raFixed;
			float worldY = player.position.y / MAP_SCALE - sin(deg) * FLOOR_SCALE * TEXTURE_RESOLUTION / dy / raFixed;
			// Fog: distancia aproximada desde el jugador
			float dx = worldX - player.position.x / MAP_SCALE;
			float dyFog = worldY - player.position.y / MAP_SCALE;
			float distToFloorPoint = sqrtf(dx * dx + dyFog * dyFog) * MAP_SCALE;
			//Getting the floor tile to get the textyre
			int fx = (int)(worldX / TEXTURE_RESOLUTION);
			int fy = (int)(worldY / TEXTURE_RESOLUTION);
			int floorTile = 0;
			if(fx >= 0 && fx < map.mapWidth && fy >= 0 && fy < map.mapHeight)
				floorTile = floors.buffer[fy * map.mapWidth + fx] - 1;
			// Fog attenuation
			ambient = fogFactor(distToFloorPoint);
			tx = player.position.x/MAP_SCALE + (cos(deg)*FLOOR_SCALE*TEXTURE_RESOLUTION/dy/raFixed);
			ty = player.position.y/MAP_SCALE - (sin(deg)*FLOOR_SCALE*TEXTURE_RESOLUTION/dy/raFixed);

			int mp = floorTile * TEXTURE_RESOLUTION * TEXTURE_RESOLUTION;
			pixel = (((int)(ty) & (TEXTURE_RESOLUTION - 1)) * TEXTURE_RESOLUTION + ((int)(tx) & (TEXTURE_RESOLUTION - 1))) * 3 + mp * 3;
			red = (texturedFloors[pixel+0]/255.0f);
			green = (texturedFloors[pixel+1]/255.0f);
			blue = (texturedFloors[pixel+2]/255.0f);
			glColor3f((red) * ambient * cr ,(green) * ambient * cg ,(blue) * ambient * cb);
			glBegin(GL_POINTS);
			glVertex2i(rays * PIXEL_SIZE,y);
			glEnd();

			//Draw Ceiling
			if(fx >= 0 && fx < map.mapWidth && fy >= 0 && fy < map.mapHeight)
				floorTile = ceiling.buffer[fy * map.mapWidth + fx] - 1;
			mp = floorTile * TEXTURE_RESOLUTION * TEXTURE_RESOLUTION;
			if(floorTile>= 0){
				pixel = (((int)(ty) & (TEXTURE_RESOLUTION - 1)) * TEXTURE_RESOLUTION + ((int)(tx) & (TEXTURE_RESOLUTION - 1))) * 3 + mp * 3;
				red = (texturedCeiling[pixel+0]/255.0f);
				green = (texturedCeiling[pixel+1]/255.0f);
				blue = (texturedCeiling[pixel+2]/255.0f);
			}
			else{ red = 0.0f; blue = 0.0f; green = 0.0f;}
			glColor3f((red) * ambient * cr ,(green) * ambient * cg ,(blue) * ambient * cb);
			glBegin(GL_POINTS);
			glVertex2i(rays * PIXEL_SIZE,topPoint - i);
			glEnd();

			i++;
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
	if(mPos >= 0 && mPos < map.mapWidth*map.mapHeight && map.buffer[mPos] > 0)
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

//Functions of the enemy
void moveEnemy(){ //Moving in passive
	mainEnemy.position = lerp(&mainEnemy.currentPos,&mainEnemy.nextPos,tePos);
	tePos += 0.1f * ENEMY_SPEED * deltaTime;
	if(tePos>=1.0f){
		mainEnemy.position = mainEnemy.nextPos;
		mainEnemy.state = IDLE;
		tePos = 0.0f;
		eTmove = (float)glfwGetTime();
	}
}

void mainEnemyUpdate(){
	//States for moving
	if(mainEnemy.state == MOVING){ //move to that target
		moveEnemy();
	}
	else if(mainEnemy.state == IDLE){ //On idle state keep track of the current target go
		etCurrent = (float)glfwGetTime();
		if(etCurrent - eTmove >= waitTime){
			CellCord start = cartesianToCellCords(mainEnemy.position.x,mainEnemy.position.y,map.mapSize);
			CellCord target = cartesianToCellCords(player.position.x,player.position.y,map.mapSize);
			CellCord nextPos = getNextPosition(&map,&start,&target);
			//Check if its in the same cell as player to not get undefined behavior
			if(!equalCellCords(&start,&target)&&nextPos.x!=-1&&nextPos.y!=-1){
					//Update angle based on the direction of the next cell
					if(nextPos.y < start.y && nextPos.x == start.x)//Going up
						mainEnemy.angle = 90.0f;
					else if(nextPos.y > start.y && nextPos.x == start.x)//Going down
						mainEnemy.angle = 270.0f;
					else if(nextPos.y == start.y && nextPos.x < start.x)//Going left
						mainEnemy.angle = 180.0f;
					else if(nextPos.y == start.y && nextPos.x > start.x)//Going right
						mainEnemy.angle = 0.0f;
					mainEnemy.currentPos = mainEnemy.position;
					mainEnemy.nextPos = cellCordToCartesian(nextPos.x,nextPos.y,map.mapSize);
					mainEnemy.state = MOVING;
			}
		}
	}

}

void passiveState(){}

void agressiveState(){}


