// 3D model of a house.
// November 26, 2005.

#include <gl/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include "functions.c"
#define  LADO  300

// Tamaño inicial de la ventana de despliegue
GLsizei winWidth=500, winHeight=500;
GLfloat alpha=0, beta=0;
GLint px0, py0, tex[6], tMuro, tPiso;
GLint Lroom, Lcasa;
GLfloat ratio;
GLfloat Vx,Vy,Vz, Vtx,Vtz, Vtheta;		//coordenadas de lookAt
GLint step=5;							//tamaño de un paso al caminar


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void init(void);
void display(void);
void casa(GLfloat baseR);
void vCristal(GLfloat pt0[3], GLfloat pt1[3], GLfloat profund, GLfloat altura);

void onMouse(GLint button, GLint state, GLint x, GLint y);
void onMotion(GLint x, GLint y);
void teclado(unsigned char key, int x, int y);
void tecladoE(int key, int x, int y);
void winReshapeFcn(GLint newWidth, GLint newHeight);
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void main(int argc, char **argv){
	glutInit(&argc,argv);
	init();
	//glutMouseFunc(onMouse);
	//glutMotionFunc(onMotion);
	glutDisplayFunc(display);
	glutSpecialFunc(tecladoE);
	glutKeyboardFunc(teclado);
	glutReshapeFunc(winReshapeFcn);
	glutMainLoop();
}

void init(void){
	GLfloat luz_ambiente[] = {0.45, 0.45, 0.45, 1.0};
	GLfloat luz0_especular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat luz0_difusa[] = {0.35, 0.35, 0.35, 1.0};
	GLfloat luz0_posicion[] = {200.0, 200.0, 200.0, 1.0};
	GLfloat luz0_direccion[] = {0.0, 0.0, 0.0};
	GLUquadricObj *Qflat;

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Objeto 3D compuesto");
	glColor3f(1.0, 1.0, 1.0);

	Qflat = gluNewQuadric();
	gluQuadricDrawStyle(Qflat, GLU_FILL);
	gluQuadricNormals(Qflat, GLU_SMOOTH);
	gluQuadricOrientation(Qflat, GLU_INSIDE);
	gluQuadricTexture(Qflat, GL_TRUE);

	ratio=(GLfloat)winWidth/(GLfloat)winHeight;
	Vx=0.0;		Vz=235.0;		Vy=30.0;		Vtheta=3*PI/2.0;

	//luz y sombreado
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,luz_ambiente);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMateriali(GL_FRONT, GL_SHININESS, 60);

	glLightfv(GL_LIGHT0,GL_DIFFUSE,luz0_difusa);
	glLightfv(GL_LIGHT0,GL_SPECULAR,luz0_especular);
	glLightfv(GL_LIGHT0,GL_POSITION,luz0_posicion);
	glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,luz0_direccion);
	glEnable(GL_LIGHT0);

	//lista room
	Lroom=glGenLists(1);	glNewList(Lroom,GL_COMPILE);	room(LADO);		glEndList();
	//lista casa
	Lcasa=glGenLists(1);	glNewList(Lcasa,GL_COMPILE);	casa(200);		glEndList();
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70.0, ratio, 5, 650);
	Vtx = Vx + step*cos(Vtheta);
	Vtz = Vz + step*sin(Vtheta);
	gluLookAt(Vx,Vy,Vz,  Vtx,Vy,Vtz, 0.0,1.0,0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(alpha, 1.0f, 0.0f, 0.0f);
	glRotatef(beta, 0.0f, 1.0f, 0.0f);

	glCallList(Lroom);
	glCallList(Lcasa);

	glutSwapBuffers();
}





void onMouse(GLint button, GLint state, GLint x, GLint y){
	if ( (button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN) ){
		px0 = x;  py0 = y;  }
}


void onMotion(GLint x, GLint y){
	alpha = (alpha + (y - py0));
	beta = (beta + (x - px0));
	px0 = x; py0 = y;
	glutPostRedisplay();
}


void teclado(unsigned char key, int x, int y){
	GLint borde=10;
	switch(key){
	case 27:  exit(0);
	case 'f': if(Vy < LADO-borde) Vy+=10;  break;
	case 'F': if(Vy < LADO-borde) Vy+=10;  break;
	case 'g': if(Vy > -1*LADO+borde) Vy-=10;  break;
	case 'G': if(Vy > -1*LADO+borde) Vy-=10;  break;
	}
	glutPostRedisplay();
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
	gluPerspective(70.0, ratio, 5, 650);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
