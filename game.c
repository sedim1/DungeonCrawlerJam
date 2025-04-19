#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define M_PI 3.14159265358979323846
#define D_PI 2*M_PI
#define CELL_SIZE 64
#define MAPX 8
#define MAPY 8
#define PIXEL_SIZE 8
#define MAX_DOF 8

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 512;


void init();
void display();
void buttons(unsigned char key,int x, int y);

//2D map
void drawPlayer();
void drawMap2D();

//Auxiliar
float angleAdjust(float angle);
float distance(float x1, float y1, float x2, float y2);

//Player
float px,py,pa, pdx, pdy;
float rotSpeed = 0.2f;

//Rendering functions;
void drawRays3D();

//gameMap
int walls[]={
	1,1,1,1,1,1,1,1,
	1,0,0,1,0,0,0,1,
	1,0,0,1,0,0,0,1,
	1,0,0,1,0,0,0,1,
	1,0,0,0,0,1,0,1,
	1,0,0,0,1,0,0,1,
	1,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1,
};

int main(int argc, char* argv[]){
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(SCREEN_WIDTH,SCREEN_HEIGHT);
	glutCreateWindow("Dungeon Crawler Jam");
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(buttons);
	glutMainLoop();
	return 0;
}

void drawPlayer(){
	glColor3f(1.0,1.0f,0.0); glPointSize(8); glLineWidth(2);
	glBegin(GL_POINTS); glVertex2i(px,py); glEnd();
	glBegin(GL_LINES); glVertex2i(px,py); glVertex2i(px+pdx*20,py+pdy*20);glEnd();
}

void drawMap2D(){
	int xo, yo;
	for(int y = 0; y < MAPY; y++){
		for(int x = 0; x < MAPX; x++){
			if(walls[y * MAPX + x]>0){glColor3f(1,1,1);}
			else {glColor3f(0.2,0.2,0.2);}
			xo = x * CELL_SIZE; yo = y * CELL_SIZE;
			glBegin(GL_QUADS);
			glVertex2i(xo+1,yo+1); 
			glVertex2i(xo+1,yo + CELL_SIZE-1); 
			glVertex2i(xo+CELL_SIZE-1,yo+CELL_SIZE-1); 
			glVertex2i(xo+CELL_SIZE-1,yo+1);
			glEnd();
		}
	}
}

void init(){
	glClearColor(0.3,0.3,0.3,0);
	gluOrtho2D(0,SCREEN_WIDTH,SCREEN_HEIGHT,0);
	px = 300; py = 300; pa = 0; pdx = cos(pa); pdy = sin(pa);
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawMap2D();
	drawRays3D();
	drawPlayer();
	glutSwapBuffers();
}

void buttons(unsigned char key,int x, int y){
	if(key == 'a'){ pa -= 0.1; pa = angleAdjust(pa); pdx = cos(pa); pdy = sin(pa);}
	if(key == 'd'){ pa += 0.1; pa = angleAdjust(pa); pdx = cos(pa); pdy = sin(pa);}
	if(key == 'w'){ px += pdx * 5.0f; py += pdy * 5.0f;}
	if(key == 's'){ px -= pdx * 5.0f; py -= pdy * 5.0f;}
	glutPostRedisplay();
}

void drawRays3D(){
	int r = 0, mx = 0, my = 0, mp = 0, dof= 0 ; float rx = 0, ry = 0, ra = 0, xo = 0, yo = 0;
	ra = pa;
	float distV = 10000.0f, distH = 10000.0f, dist = 0.0f;
	float hx = 0.0, hy = 0.0, vx = 0.0, vy = 0.0;//Horizontal and vertical rays
	printf("ra %.2f\n",(ra * 180.0f)/M_PI);
	for(r = 0; r < 1; r++){
		//Check Horizontal lines
		dof = 0;
		hx = px; hy = py;
		float aTan = -1.0f/tan(ra);
		//loking up
		if(ra > M_PI){ ry = (((int)(py)/CELL_SIZE) * CELL_SIZE) - 0.0001f; rx = (py-ry) * aTan + px; yo = -CELL_SIZE; xo = -yo * aTan;}
		//lookin down
		if(ra < M_PI){ ry = (((int)(py)/CELL_SIZE) * CELL_SIZE) + CELL_SIZE; rx = (py-ry) * aTan + px; yo = CELL_SIZE; xo = -yo * aTan;}
		if(ra == 0 || ra == M_PI){rx = px; ry = py; dof = MAX_DOF;} //looking left or right
		while(dof < MAX_DOF){
			mx = ((int)(rx))/CELL_SIZE; my = ((int)(ry))/CELL_SIZE; mp = my * MAPX + mx;
			if( mp >= 0 && mp < MAPX * MAPY && walls[mp] > 0){ dof = MAX_DOF; hx = rx; hy = rx; distH = distance(px,py,rx,ry);}
			else { rx += xo; ry += yo; dof+=1;}
		}
		//glLineWidth(4); glColor3f(0.0,1.0,0.0f);
		//glBegin(GL_LINES); glVertex2i(px,py); glVertex2i(rx,ry);glEnd();
		//Vertical lines
		dof = 0;
		vx = px; vy = py;
		float nTan = -tan(ra);
		//looking left
		if(ra > M_PI/2.0f && ra < (3*M_PI)/2.0f){ 
			rx = (((int)(px)/CELL_SIZE) * CELL_SIZE) - 0.0001f  ; ry = (px-rx) * nTan + py; xo = -CELL_SIZE; yo = -xo * nTan; } 
		//lookin right
		if(ra < M_PI/2.0f || ra > (3*M_PI)/2.0f){ 
			rx = (((int)(px)/CELL_SIZE) * CELL_SIZE) + CELL_SIZE; ry = (px-rx) * nTan + py; xo = CELL_SIZE; yo = -xo * nTan;} 
		if(ra == M_PI/2.0f || ra == (3 * M_PI)/2){rx = px; ry = py; dof = MAX_DOF;} //looking up or down
		while(dof < MAX_DOF){
			mx = ((int)(rx))/CELL_SIZE; my = ((int)(ry))/CELL_SIZE; mp = my * MAPX + mx;
			if( mp >= 0 && mp < MAPX * MAPY && walls[mp] > 0){ dof = MAX_DOF; vx = rx; vy = ry; distV = distance(px,py,rx,ry);}
			else { rx += xo; ry += yo; dof+=1;}
		}
		if(distV < distH){ 
			rx = vx; ry = vy; dist = distV;
		}
		if(distH < distV){ 
			rx = hx; ry = hy; dist = distH;
		}
		//Vertical lines
		glLineWidth(2); glColor3f(1.0,0.0,0.0f);
		glBegin(GL_LINES); glVertex2i(px,py); glVertex2i(rx,ry);glEnd();
	}
}

//Auxiliar
float angleAdjust(float angle){
	float res = angle;
	if(angle < 0){res += 2 * M_PI;}
	else if(angle >= 2* M_PI){res -= 2*M_PI;}
	return res;
}

float distance(float x1, float y1, float x2, float y2){
	return (sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)));
}
