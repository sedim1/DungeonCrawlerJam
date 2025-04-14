#include "Raycaster.h"

float degToRad(float* degrees){
    if(*degrees>359.0f)
        *degrees-=360.0f;
    else if(*degrees<0.0f)
        *degrees+=360;
    return (*degrees * M_PI)/180.0f;
}

//Map2D functions
Map2D loadMap(char* path,enum VIEW_MODE mode){
    Map2D map;
    map.mapWidth = 0; map.mapHeight = 0; map.mapSize = 0; map.projection = mode;
    int i = 0;
    int j = 0;
    FILE* fp = fopen(path,"r");
    if(fp){
        fscanf(fp,"%d %d %d",&map.mapWidth,&map.mapHeight,&map.mapSize);//Read map width, height and size
        //Load map elements
        while(!feof(fp)){
            fscanf(fp,"%d",&map.buffer[i][j]);
            j++;
            if(j>=map.mapWidth){
                j = 0; i++;
            }
        }
        printf("MAP LOADED %s \n",path);
        printf("[ WIDTH HEIGHT SIZE ]\n%d %d %d\n[ MAP LAYOUT ]\n",map.mapWidth,map.mapHeight,map.mapSize);
        for(i = 0; i < map.mapHeight;i++){
            for(j = 0; j < map.mapWidth;j++)
                printf("%d ",map.buffer[i][j]);
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
    int x = 0; int y = 0;
    for(y = 0; y < map->mapHeight; y++){
        for(x = 0; x < map->mapWidth; x++){
            if(map->buffer[y][x]==1)//Wall
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
    float dx = cos(degToRad(&entity->angle));
    float dy = -sin(degToRad(&entity->angle));
    glColor3f(1.0f,1.0f,0.0f);
    glPointSize(8); glLineWidth(2);
    glBegin(GL_POINTS);glVertex3i(entity->xPos,entity->yPos,1);glEnd();
    glBegin(GL_LINES);glVertex3i(entity->xPos,entity->yPos,1);glVertex3i(entity->xPos + dx * 20,entity->yPos + dy * 20,1); glEnd();
}
