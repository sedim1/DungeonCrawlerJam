#ifndef BFS_H
#define BFS_H

#include <stdio.h>
#include "glad/glad.h"
#include "Raycaster.h"
#include <stdbool.h>

#define MAX_SIZE MAX*MAX

typedef struct{ //front ------ back
	CellCord array[MAX_SIZE];
	int front;
	int back;
}Queue;

typedef struct{
	CellCord points[MAX_SIZE];
	int size;
}Path;

void startQueue(Queue* queue){
	queue->front = -1;
	queue->back = 0;
}

bool isFull(Queue* queue){
	return queue->back == MAX_SIZE;
}

bool isEmpty(Queue* queue){
	return queue->front == queue->back-1;
}

void enqueue(Queue* queue,CellCord* A){
	if(isFull(queue)){
		printf("ERROR::QUEUE OVERFLOW::\n");return;
	}
	queue->array[queue->back] = *A;
	queue->back+=1;
}

CellCord dequeue(Queue* queue){
	CellCord v = {-1,-1};
	if(isEmpty(queue))
		return v;
	v = queue->array[queue->front+1];
	queue->front+=1;
	return v;
}

void drawBFS(Map2D* map,CellCord* start, CellCord* end){
        //Prepare stuff before doing the search
        Queue q; startQueue(&q);
        bool reachedEnd = false;
        bool visited[MAX * MAX] = {false};//size width * height
        int dr[4] = {1,-1,0,0};
        int dc[4] = {0, 0,-1,1};
        CellCord neighbor = {0,0};
        CellCord cameFrom[MAX * MAX]; //sizef width * height
	///----------------------------///
        reachedEnd = (start->x == end->x) && ( start->y == end->y);
	if(reachedEnd)
		return;
        ///---------------------------/////
        enqueue(&q,start);//Mark starting point as visited
        visited[start->y * map->mapWidth + start->x] = true;
        while(!isEmpty(&q)){
                //Get current position
                CellCord current = dequeue(&q);
                //Check if it has reached its destination
                reachedEnd = (current.x == end->x) && ( current.y == end->y);
                if(reachedEnd)
                        break;
		//Explore neighbors
                for(int i = 0; i < 4; i++){
                        neighbor.y = current.y + dr[i];
                        neighbor.x = current.x + dc[i];
                        int mPos = neighbor.y * map->mapWidth + neighbor.x;
                        //Skip this iteration if it is out of bound
                        if(mPos < 0 || mPos >= map->mapWidth * map->mapHeight)
                                continue;
                        //Skip if it already visited it or it is blocked by a wall
                        if(visited[mPos] == true || map->buffer[mPos] == 1)
                                continue;
                        //Add neighbor to the queue
                        enqueue(&q,&neighbor);
                        cameFrom[mPos] = current;
                        visited[mPos] = true;
			VECTOR2D pos = cellCordToCartesian(neighbor.x,neighbor.y,map->mapSize);
			glColor3f(0.0f,0.0f,1.0f);
			glPointSize(8);
			glBegin(GL_POINTS); 
			//glVertex3i(pos.x,pos.y,1.0f); 
			glEnd();
                }
        }
        if(!reachedEnd){//If queue is empty it means it is unreachable
                printf("ERROR::Unreachable location\n"); return;
        }
        CellCord current = *end;
	reachedEnd = false;
        while(!reachedEnd){
                reachedEnd = (current.x == start->x) && ( current.y == start->y);
		VECTOR2D pos = cellCordToCartesian(current.x,current.y,map->mapSize);
		glColor3f(1.0f,0.0f,0.0f);
		glPointSize(8);
		glBegin(GL_POINTS); 
		glVertex3i(pos.x,pos.y,1.0f); 
		glEnd();
                int mapPos = current.y * map->mapWidth + current.x;
                current = cameFrom[mapPos];
        }
}

CellCord getNextPosition(Map2D* map,CellCord* start, CellCord* end){
        //Prepare stuff before doing the search
	CellCord nextPos = {-1,-1};
        Queue q; startQueue(&q);
        bool reachedEnd = false;
        bool visited[MAX * MAX] = {false};//size width * height
        int dr[4] = {1,-1,0,0}; int dc[4] = {0, 0,-1,1};
        CellCord neighbor = {0,0}; CellCord cameFrom[MAX * MAX] = {{-1,-1}};
	///----------------------------///
        reachedEnd = (start->x == end->x) && ( start->y == end->y);
	if(reachedEnd){
		nextPos = *start; return nextPos;}
        ///---------------------------/////
        enqueue(&q,start);//Mark starting point as visited
        visited[start->y * map->mapWidth + start->x] = true;
        while(!isEmpty(&q)){
                //Get current position
                CellCord current = dequeue(&q);
                //Check if it has reached its destination
                reachedEnd = (current.x == end->x) && ( current.y == end->y);
                if(reachedEnd)
                        break;
		//Explore neighbors
                for(int i = 0; i < 4; i++){
                        neighbor.y = current.y + dr[i];
                        neighbor.x = current.x + dc[i];
                        int mPos = neighbor.y * map->mapWidth + neighbor.x;
                        //Skip this iteration if it is out of bound
                        if(mPos < 0 || mPos >= map->mapWidth * map->mapHeight)
                                continue;
                        //Skip if it already visited it or it is blocked by a wall
                        if(visited[mPos] == true || map->buffer[mPos] == 1)
                                continue;
                        //Add neighbor to the queue
                        enqueue(&q,&neighbor);
                        cameFrom[mPos] = current;
                        visited[mPos] = true;
			VECTOR2D pos = cellCordToCartesian(neighbor.x,neighbor.y,map->mapSize);
			//glColor3f(0.0f,0.0f,1.0f); glPointSize(8);
			//glBegin(GL_POINTS); glVertex3i(pos.x,pos.y,1.0f); glEnd();
                }
        }
        if(isEmpty(&q)){//If queue is empty it means it is unreachable
                printf("ERROR::Unreachable location\n");return nextPos;
        }
        //printf("----------------------\n");
        nextPos = *end;
	reachedEnd = false;
	int lastPos = 0;
	//Reconstruct path
        while(!reachedEnd){
		VECTOR2D pos = cellCordToCartesian(nextPos.x,nextPos.y,map->mapSize);
		//glColor3f(1.0f,0.0f,0.0f); glPointSize(8);
		//glBegin(GL_POINTS); glVertex3i(pos.x,pos.y,1.0f); glEnd();
                int mapPos = nextPos.y * map->mapWidth + nextPos.x;
		//printf("CameFrom %d %d\n",cameFrom[mapPos].x,cameFrom[mapPos].y);
                nextPos = cameFrom[mapPos];
                reachedEnd = (nextPos.x == start->x) && ( nextPos.y == start->y);
		if(!reachedEnd)
			lastPos = mapPos;
        }
	nextPos = cameFrom[lastPos];
        //printf("----------------------\n");
	//printf("Next Pos %d %d\n",nextPos.x,nextPos.y);
	return nextPos;
}


#endif
