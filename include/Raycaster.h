#ifndef RAYCASTER_H
#define RAYCASTER_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>


#define MAX 100
#define M_PI 3.14159265358979323846
#define PROJECTION_PLANE_WIDTH 800
#define PROJECTION_PLANE_HEIGHT 640
#define RAYS 75
#define LINE_HEIGHT 320
enum VIEW_MODE{
	MAP2D, //For visually debugging the map on a grid
	PROJECTION3D,
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
float length(VECTOR2D* a, VECTOR2D* b);


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
typedef struct{
	float angle;
	VECTOR2D position;
}Entity;

void drawEntityOnMap(Entity* entity);

//Raycasting functions
void drawRays3D(Map2D* map,Entity* entity);//Draw according to the view of an entity
VECTOR2D castRayH(Map2D* map, Entity* entity);
VECTOR2D castRayV(Map2D* map, Entity* entity);
#endif
