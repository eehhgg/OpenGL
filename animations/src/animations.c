// 3D animations.
// November 21, 2005.

#include <gl/glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "functions.c"
#define  PI  3.141592
#define  LADO  200

GLint winWidth=640, winHeight=350;		//tamaño de la ventana
GLfloat alpha=0,beta=0,gamma=0, ratio;	//angulos de rotacion, aspectRatio de la ventana
GLint px0, py0;							//posicion0 del mouse
GLfloat Vx,Vy,Vz, Vtx,Vtz, Vtheta;		//coordenadas de lookAt
GLint step=5;							//tamaño de un paso al caminar
GLint Ljet, Lroom, Lcastle;		//listas de despliegue
GLfloat esfBeta=0;				//angulo de la esfera roja en la escena

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void init(void);
void display(void);

void idle(void);
void teclado(unsigned char key, int x, int y);
void tecladoE(int key, int x, int y);
void winReshapeFcn(GLint newWidth, GLint newHeight);
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void main(int argc, char **argv) 
{
	glutInit(&argc,argv);
	init();
	glutDisplayFunc(display);
	glutSpecialFunc(tecladoE);
	glutKeyboardFunc(teclado);
	glutReshapeFunc(winReshapeFcn);
	glutIdleFunc(idle);
	glutMainLoop();
}

void init(void){
	float luz_ambiente[] = {0.45, 0.45, 0.45, 1.0};
	float luz0_especular[] = {1.0, 1.0, 1.0, 1.0};
	float luz0_difusa[] = {0.35, 0.35, 0.35, 1.0};
	float luz0_posicion[] = {200.0, 200.0, 200.0, 1.0};
	float luz0_direccion[] = {0.0, 0.0, 0.0};
	float reflectancia[] = {0.5, 0.5, 0.5, 1.0};

	float bases[5]={69.6, 63.15, 51.3, 39.5, 35.95};
	float basesT[6]={82.45, 77.4, 64.05, 52.5, 47.25, 30.0};
	float altura=300.0;
	float color1[]= {0.0, 0.0, 0.0, 1.0};
	float color2[]= {0.7, 0.0, 0.0, 1.0};
	float basesX[5], basesTX[6], alturaX;

	ratio=(GLfloat)winWidth/(GLfloat)winHeight;
	Vx=Vz=0.0;		Vy=30.0;		Vtheta=3*PI/2.0;

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition(30, 50);
	glutCreateWindow("Animacion");

	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,luz_ambiente);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, reflectancia);
	glMateriali(GL_FRONT, GL_SHININESS, 60);

	glLightfv(GL_LIGHT0,GL_DIFFUSE,luz0_difusa);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,luz_ambiente);
	glLightfv(GL_LIGHT0,GL_SPECULAR,luz0_especular);
	glLightfv(GL_LIGHT0,GL_POSITION,luz0_posicion);
	glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,luz0_direccion);
	glEnable(GL_LIGHT0);

	//lista room
	Lroom=glGenLists(1);	glNewList(Lroom,GL_COMPILE);	room(LADO);		glEndList();
	//lista jet
	Ljet=glGenLists(1);		glNewList(Ljet,GL_COMPILE);		jet();		glEndList();
	//lista castle1
	Lcastle=glGenLists(1);
	copiaVec(basesX,bases,5);			escalaVec(0.1, basesX, 5);
	copiaVec(basesTX,basesT,6);			escalaVec(0.1, basesTX, 6);
	alturaX = 0.1*altura;
	glNewList(Lcastle,GL_COMPILE);		castillo(basesX, basesTX, alturaX, color1, color2);		glEndList();
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, ratio, 5, 450);

	Vtx = Vx + step*cos(Vtheta);
	Vtz = Vz + step*sin(Vtheta);
	gluLookAt(Vx,Vy,Vz,  Vtx,Vy,Vtz, 0.0,1.0,0.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();

	//room
	glCallList(Lroom);

	//castillo
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0, 25.0, -100.0);
	glRotatef(alpha, 1.0f, 0.0f, 0.0f);
	glRotatef(beta, 0.0f, 1.0f, 0.0f);
	glRotatef(gamma, 0.0f, 0.0f, 1.0f);
	glCallList(Lcastle);

	//jet
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0, 25.0, 100.0);
	glScalef(0.3, 0.3, 0.3);
	glRotatef(alpha, 1.0f, 0.0f, 0.0f);
	glRotatef(beta, 0.0f, 1.0f, 0.0f);
	glCallList(Ljet);

	//esfera
	glPopMatrix();
	glPushMatrix();
	glRotatef(esfBeta, 0.0,1.0,0.0);
	glTranslatef(170, 15.0, 0.0);
	glRotatef(90, 0.0,1.0,0.0);
	glRotatef(-2*gamma, 0.0,0.0,1.0);
	glColor3f(0.7,0.0,0.0);
	glutWireSphere(10.0, 10,10);

	//cono
	glPopMatrix();
	glPushMatrix();
	glRotatef(esfBeta-120, 0.0,1.0,0.0);
	glTranslatef(170, 15.0, 0.0);
	//glRotatef(90, 0.0,1.0,0.0);
	glRotatef(-2*gamma, 0.0,0.0,1.0);
	glColor3f(0.0,0.7,0.0);
	glutWireCone(10.0, 10.0, 10,10);

	//cubo
	glPopMatrix();
	glPushMatrix();
	glRotatef(esfBeta-240, 0.0,1.0,0.0);
	glTranslatef(170.0, 25.0, 0.0);
	glRotatef(45, -1.0,-1.0,1.0);
	glRotatef(-2*gamma, 1.0,1.0,1.0);
	glColor3f(0.0,0.0,1.0);
	glutSolidCube(20.0);

	glPopMatrix();
	glutSwapBuffers();
}



void idle(void){
	float var=1.0;		//variacion de angulo
	alpha+=var;   beta+=var;   gamma+=var;   esfBeta+=var/3.0;
	if(alpha>360) alpha-=360;
	if(beta>360) beta-=360;
	if(gamma>360) gamma-=360;
	if(esfBeta>360) esfBeta-=360;
	glutPostRedisplay();
}

void teclado(unsigned char key, int x, int y){
	if(key==27) exit(0);
}

void tecladoE(int key, int x, int y){
	GLfloat maxX, maxZ, phi;	//valores maximos, y angulo desde el centro hacia posicion actual
	GLint check=0, borde=10;
	switch(key){
	case GLUT_KEY_UP:		Vx=Vtx;  Vz=Vtz;  check=1;			break;
	case GLUT_KEY_DOWN:		Vx-=Vtx-Vx;  Vz-=Vtz-Vz;  check=1;	break;
	case GLUT_KEY_RIGHT:	Vtheta+=0.2;   if(Vtheta>=2*PI) Vtheta-=2*PI;	break;
	case GLUT_KEY_LEFT:		Vtheta-=0.2;   if(Vtheta<0) Vtheta+=2*PI;			break;
	}
	//validar posicion en la escena
	if( check  &&  ((Vx*Vx+Vz*Vz) > (LADO-5*borde)*(LADO-5*borde)) ){
		//determinar angulo (0 a PI/2) desde el eje X hacia el eje Z
		if(fabs(Vx)<1)		phi= PI/2.0;
		else if(fabs(Vz)<1)	phi= 0;
		else phi= atan(fabs(Vz)/fabs(Vx));
		//determinar angulo real (0 a 2*PI), desde el eje +X hacia el eje +Z
		if(Vx>0){	if(Vz<0) phi=2*PI-phi;  }
		else{		if(Vz<0) phi+=PI; else phi=PI-phi;  }
		//determinar coordenadas maximas para ese angulo
		maxX= (LADO-borde)*cos(phi);		maxZ= (LADO-borde)*sin(phi);
		//ajustar coordenadas
		if(fabs(Vx) > fabs(maxX)) Vx=maxX;
		if(fabs(Vz) > fabs(maxZ)) Vz=maxZ;
		check=0;
	}
	glutPostRedisplay();
}

void winReshapeFcn(GLint newWidth, GLint newHeight){
	winWidth=newWidth;
	winHeight= (newHeight<=0)? 1: newHeight;
	ratio = ((GLfloat)winWidth) / ((GLfloat)winHeight);
	glViewport(0, 0, winWidth, winHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, ratio, 5, 450);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

