#ifndef RAYCASTER_H
#define RAYCASTER_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "glad/glad.h"


#define MAX 100
#define M_PI 3.14159265358979323846

enum VIEW_MODE{
	MAP2D, //For visually debugging the map on a grid
	EYE_VIEW,
};

//
float degToRad(float* degrees);

//For handling the 2D map
typedef struct{
	int mapWidth;
	int mapHeight;
	int mapSize; //Unit size per block
	int buffer[MAX][MAX];
	enum VIEW_MODE projection;
}Map2D;

Map2D loadMap(char* path, enum VIEW_MODE projection);
void drawMap2D(Map2D* map);

//For handling entitites in the world (Player and enemy)
typedef struct{
	float angle;
	float xPos;
	float yPos;
}Entity;

void drawEntityOnMap(Entity* entity);



#endif
