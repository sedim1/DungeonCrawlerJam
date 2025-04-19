#ifndef RAYCASTER_H
#define RAYCASTER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "glad/glad.h"
#include "GLFW/glfw3.h"


#define MAX 100
#define M_PI 3.14159265358979323846
#define PROJECTION_WIDTH 800
#define PROJECTION_HEIGHT 640
#define FOV 60
#define PIXEL_SIZE 8
#define RAYS (PROJECTION_WIDTH / PIXEL_SIZE)
#define CELLSIZE 64
#define TEXTURE_RESOLUTION 32
#define MAP_SCALE 2
#define BASE_FLOOR_SCALE 160
#define FLOOR_SCALE (BASE_FLOOR_SCALE * MAP_SCALE)

#define MAX_DIST 400

enum VIEW_MODE{
	MAP2D, //For visually debugging the map on a grid
	PROJECTION3D,
};

enum DIRECTION{
	UP,
	DOWN,
	LEFT,
	RIGHT,
};


typedef struct{
	int x;
	int y;
}CellCord;

typedef struct{
	float x;
	float y;
}VECTOR2D;

//Auxiliar Functions to help us
float degToRad(float degrees);
float angleAdjust(float degrees);
CellCord cartesianToCellCords(float x, float y, int cellSize);
VECTOR2D cellCordToCartesian(int x, int y, int cellSize);
float length(VECTOR2D* a, VECTOR2D* b);
VECTOR2D lerp(VECTOR2D* A, VECTOR2D* B,float t);
float moveToward(float from,float to,float delta);
bool equalCellCords(CellCord* A, CellCord *B);

//For handling the 2D map
typedef struct{
	int mapWidth;
	int mapHeight;
	int mapSize; //Unit size per block
	int buffer[MAX * MAX];
	enum VIEW_MODE projection;
}Map2D;

Map2D loadMap(char* path, enum VIEW_MODE projection);
void drawMap2D(Map2D* map);

//For handling entitites in the world (Player and enemy)
enum EntityState {
	IDLE,
	MOVING,
	ROTATING,
};

//Objects in the world
typedef struct{
	float angle;
	VECTOR2D position; //Actual position
	VECTOR2D currentPos; //Actual position
	VECTOR2D nextPos; //Actual position
	enum EntityState state;
}Entity;

typedef struct{
	float angle;
	int x,y,z; //Position
}Sprite;

void drawEntityOnMap(Entity* entity,float r,float g,float b);

//Raycasting functions for debugging
void drawRays3D(Map2D* map,Entity* entity);//Draw according to the view of an entity
VECTOR2D castRayH(Map2D* map, Entity* entity, int* mapVal);
VECTOR2D castRayV(Map2D* map, Entity* entity, int* mapVal);

//For shading
float fogFactor(float distance);

#endif
