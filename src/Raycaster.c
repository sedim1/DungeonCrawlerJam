#include "Raycaster.h"

//Auxiliar Functions
float degToRad(float degrees){
    return (degrees * M_PI)/180.0f;
}

float angleAdjust(float degrees){
	float res =fmod(degrees,360.0f);
    	if(res < 0.0f){
        	res += 360.0f;
	}
	return res;
}

CellCord cartesianToCellCords(float x,float y,int cellSize){
	CellCord res = {0,0};
	res.x = (int)(x/cellSize);
	res.y = (int)(y/cellSize);
	return res;
}

VECTOR2D cellCordToCartesian(int x, int y,int cellSize){
        VECTOR2D cartesian;
        cartesian.x = (x * cellSize) + (cellSize/2.0f);
        cartesian.y = (y * cellSize) + (cellSize/2.0f);
        return cartesian;
}

float length(VECTOR2D* a, VECTOR2D* b){
	float l = 0.0f;
	float dx = b->x - a->x;
	float dy = b->y - a->y;
	l = sqrt(dx*dx + dy*dy);
	return l;
}

float moveToward(float from,float to,float delta){
	float res = from + delta;
	if(res >= to)
		res = to;
	return res;
}

VECTOR2D lerp(VECTOR2D* A, VECTOR2D* B,float t){
	VECTOR2D res;
	res.x = A->x + (t * (B->x - A->x));
	res.y = A->y + (t * (B->y - A->y));
	return res;
}

//Map2D functions
Map2D loadMap(char* path,enum VIEW_MODE mode){
    Map2D map = {0,0,0,{0},MAP2D};
    //map.mapWidth = 0; map.mapHeight = 0; map.mapSize = 0; map.projection = mode;
    int i = 0;
    FILE* fp = fopen(path,"r");
    if(fp){
        fscanf(fp,"%d %d %d",&map.mapWidth,&map.mapHeight,&map.mapSize);//Read map width, height and size
        //Load map elements
        while(!feof(fp)){
            fscanf(fp,"%d",&map.buffer[i]);
            i++;
        }
        printf("\nMAP LOADED %s \n",path);
        printf("[ WIDTH HEIGHT SIZE ]\n%d %d %d\n",map.mapWidth,map.mapHeight,map.mapSize);
        for(int i = 0; i < map.mapHeight; i++){
        	for(int j = 0; j < map.mapWidth; j++){
			printf("%d ",map.buffer[i * map.mapWidth + j]);
		}
		printf("\n");
	}
	fclose(fp);
    }
    else{
        printf("ERROR::COULD NOT FIND OR LOAD %s\n",path);
    }
    return map;
}

void drawMap2D(Map2D* map){
                        //mx = (int)(rx / map->mapSize); my = (int)(ry / map->mapSize);
                        //mPos = my * map->mapWidth + mx;
    int x = 0; int y = 0;
    for(y = 0; y < map->mapHeight; y++){
        for(x = 0; x < map->mapWidth; x++){
		int mPos = y * map->mapWidth + x;
            if(map->buffer[mPos]==1)//Wall
                glColor3f(1.0f,1.0f,1.0f);
            else
                glColor3f(0.3f,0.3f,0.3f);
            //Calculate points for the actual block
            int x1 = (x * map->mapSize); int y1 = (y * map->mapSize);
            int x2 = (x * map->mapSize) + map->mapSize; int y2 = (y * map->mapSize);
            int x3 = (x * map->mapSize); int y3 = (y * map->mapSize) + map->mapSize;
            int x4 = (x * map->mapSize) + map->mapSize; int y4 = (y * map->mapSize) + map->mapSize;
            //Draw points on screen
            glBegin(GL_TRIANGLES);
            glVertex2i(x1+1,y1+1);
           glVertex2i(x2-1,y2+1);
            glVertex2i(x3+1,y3-1);
            glVertex2i(x2-1,y2+1);
            glVertex2i(x3+1,y3-1);
            glVertex2i(x4-1,y4-1);
            glEnd();
        }
    }
}

void drawEntityOnMap(Entity* entity){
    float dx = cos(degToRad(entity->angle));
    float dy = -sin(degToRad(entity->angle));
    glColor3f(1.0f,1.0f,0.0f);
    glPointSize(8); glLineWidth(2);
    glBegin(GL_POINTS);glVertex3i(entity->position.x,entity->position.y,1);glEnd();
    glBegin(GL_LINES);glVertex3i(entity->position.x,entity->position.y,1);glVertex3i(entity->position.x + dx * 20,entity->position.y + dy * 20,1); glEnd();
}

//Raycasting Functions
VECTOR2D castRayH(Map2D* map, Entity* entity){
	VECTOR2D ray;
	float rayAngle = entity->angle;
	float rx,ry, xo, yo;
	int r = 0;int dof = 0; int mPos , mx, my;
	CellCord playerPos = cartesianToCellCords(entity->position.x,entity->position.y,map->mapSize);
        if(fabs(fmod(rayAngle,180.0f)) == 0.0f){//looking left or right
                rayAngle+=0.0001f;
        }
	float aTan = -1.0f/tan(degToRad(rayAngle));
        if(rayAngle > 180.0f)//Looking Down
        {
                ry = playerPos.y * map->mapSize + map->mapSize;
                rx = -(entity->position.y-ry) * aTan + entity->position.x;
                yo = map->mapSize;
                xo = yo * aTan;
        }
        if(rayAngle < 180.0f){//Looking Up
                ry = playerPos.y * map->mapSize - 0.0001f;
                rx = -(entity->position.y-ry) * aTan + entity->position.x;
        	yo = -map->mapSize;
                xo = yo * aTan;
        }
	while(dof<8){//Check if it hits a wall on the map layout
                        mx = (int)(rx / map->mapSize); my = (int)(ry / map->mapSize);
                        mPos = my * map->mapWidth + mx;
                        if(mPos >= 0 && mPos < map->mapWidth*map->mapHeight && map->buffer[mPos]){
                                dof = 8;
                        }
                        else{
                                rx += xo; ry += yo; dof+=1;
                        }
                }
	ray.x = rx; ray.y = ry;
	return ray;
}

VECTOR2D castRayV(Map2D* map, Entity* entity){
	VECTOR2D ray;
	float rayAngle = entity->angle;
	float rx,ry, xo, yo;
	int r = 0;int dof = 0; int mPos , mx, my;
	CellCord playerPos = cartesianToCellCords(entity->position.x,entity->position.y,map->mapSize);
                if(fabs(fmod(rayAngle,90.0f))==0.0f){//looking up or down
                        rayAngle+=0.0001f;
                }
		float nTan = -tan(degToRad(rayAngle));
                if(rayAngle > 90.0f && rayAngle < 270.0f)//Looking left
                {
                        rx = playerPos.x * map->mapSize - 0.0001f;
                        ry = -(entity->position.x-rx) * nTan + entity->position.y;
                        xo = -map->mapSize;
                        yo = (xo * nTan);
                }
                if(rayAngle < 90.0f || rayAngle > 270.0f){//Looking right
                        rx = playerPos.x * map->mapSize + map->mapSize;
                        ry = -(entity->position.x-rx) * nTan + entity->position.y;
                        xo = map->mapSize;
                        yo = (xo * nTan);
                }
		while(dof<8){//Check if it hits a wall on the map layout
                        mx = (int)(rx / map->mapSize); my = (int)(ry / map->mapSize);
                        mPos = my * map->mapWidth + mx;
                        if(mPos >= 0 && mPos < map->mapWidth*map->mapHeight && map->buffer[mPos]){
                                dof = 8;
                        }
                        else{
                                rx += xo; ry += yo; dof+=1;
                        }
                }
		ray.x = rx; ray.y = ry;
		return ray;

}


void drawRays3D(Map2D* map,Entity* entity){
	int rays = 0;
	float distH,distV,dist;
	Entity r = *entity;//The ray represented as an entity;
        float angleStep = (float)FOV / (float)RAYS;
	r.angle = entity->angle + ((float)FOV/2.0f); r.angle = angleAdjust(r.angle);
	//Get the position in cell cord where the ray starts to get caste
	for(rays = 0; rays < RAYS;rays++){
		VECTOR2D rayH = castRayH(map,&r);
		VECTOR2D rayV = castRayV(map,&r);
		VECTOR2D ray;
		distH = length(&entity->position,&rayH);
		distV = length(&entity->position,&rayV);
		ray = (distH < distV) ? rayH : rayV;
		dist = (distH < distV) ? distH : distV; //Save distance of the selected wall
		glColor3f(1.0f,0.0f,0.0f);glLineWidth(2);
		glBegin(GL_LINES);
		glVertex3i(entity->position.x,entity->position.y,1);glVertex3i(ray.x,ray.y,1);
		glEnd();
		r.angle -= angleStep; r.angle = angleAdjust(r.angle);
	}
}

