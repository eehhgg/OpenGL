// 3D world clock.
// Summer 2006.

#include <gl/glut.h>
#include "functions.c"

// Tamaño inicial de la ventana de despliegue
GLsizei winWidth=500, winHeight=500;
GLfloat alpha=0, beta=0;
GLint xi, yi;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void init(void);
void display(void);
void menus(void);
void Fm1(GLint sel);

void idle(void);
void tecladoE(int key, int x, int y);
void onMouse(int button, int state, int x, int y);
void onMotion(int x, int y);
void winReshapeFcn(GLint newWidth, GLint newHeight);
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void main(int argc, char **argv)
{
	glutInit(&argc,argv);
	init();
	glutDisplayFunc(display);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutIdleFunc(idle);
	glutSpecialFunc(tecladoE);
	glutReshapeFunc(winReshapeFcn);
	menus();

	glutMainLoop();
}

void init(void){
	float luz_ambiente[] = {0.75, 0.75, 0.75, 1.0};
	float luz0_especular[] = {1.0, 1.0, 1.0, 1.0};
	float luz0_posicion[] = {50.0, 50.0, 50.0, 1.0};
	float luz0_difusa[] = {0.55, 0.55, 0.55, 1.0};
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Reloj");

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,luz_ambiente);

	glLightfv(GL_LIGHT0,GL_AMBIENT,luz_ambiente);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,luz0_difusa);
	glLightfv(GL_LIGHT0,GL_SPECULAR,luz0_especular);
	glLightfv(GL_LIGHT0,GL_POSITION,luz0_posicion);
	glEnable(GL_LIGHT0);

	glGenTextures(1, &texG[0]);
	LoadBMP("img/earth.bmp",texG[0]);
	glGenTextures(1, &texG[1]);
	LoadBMP("img/earth1.bmp",texG[1]);
	glGenTextures(1, &texG[2]);
	LoadBMP("img/earth2.bmp",texG[2]);
	glGenTextures(1, &texG[3]);
	LoadBMP("img/earth3.bmp",texG[3]);
	glGenTextures(1, &texG[4]);
	LoadBMP("img/earth4.bmp",texG[4]);
	glGenTextures(1, &texG[5]);
	LoadBMP("img/earth5.bmp",texG[5]);

	lEsf=glGenLists(1);   glNewList(lEsf,GL_COMPILE);   esfera();   glEndList();
	zonaLocal();
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-50,50, -50,50, -50,50);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(alpha, 1.0f, 0.0f, 0.0f);
	glRotatef(beta, 0.0f, 1.0f, 0.0f);
	reloj();
	glutSwapBuffers();
}



//funciones glut --------------------------------------

void idle(void){
	struct tm *tmp2;
	time(&tmp); tmp2=gmtime(&tmp);
	seg=tmp2->tm_sec;
	min=tmp2->tm_min;
	hor=(tmp2->tm_hour+zona)%24;  if(hor<0) hor+=24;
	glutPostRedisplay();
}

void tecladoE(int key, int x, int y){
	switch(key){
	case GLUT_KEY_LEFT:		zona--;   if(zona<=-13) zona=12;	break;
	case GLUT_KEY_RIGHT:	zona++;   if(zona>=13)  zona=-12;	break;
	}
	glutPostRedisplay();
}

void onMouse(int button, int state, int x, int y){
	if ( (button == GLUT_LEFT_BUTTON) & (state == GLUT_DOWN) ){
		xi = x; yi = y;  }
}

void onMotion(GLint x, GLint y){
	alpha = alpha + y - yi;
	beta = beta + x - xi;
	if(alpha>=360) alpha-=360;		if(alpha<=-360) alpha+=360;
	if(beta>=360) beta-=360;		if(beta<=-360) beta+=360;
	xi = x; yi = y;
	glutPostRedisplay();
}

void winReshapeFcn(GLint newWidth, GLint newHeight)
{
	GLfloat wr;
	GLint esp;
	winWidth=newWidth;  winHeight=newHeight;

	if(!winHeight)  winHeight=1;
	if(!winWidth)  winWidth=1;
	wr=winHeight*1.0/winWidth;

	if(wr >= 1.0){	//ajustar al ancho
		esp=(winHeight-winWidth)/2;
		glViewport(0,esp,winWidth,winWidth);  }
	else{		//ajustar al alto
		esp=(winWidth-winHeight)/2;
		glViewport(esp,0,winHeight,winHeight);  }
}

void Fmtex(GLint sel){
	itexG=sel;
	glutPostRedisplay();
}

void Fm(GLint sel){
	switch(sel){
	case 6:  merid=(merid+1)%2;  break;
	case 7:  zonaLocal();  break;
	case 8:  exit(0);  break;
	}
	glutPostRedisplay();
}

void menus(void){
	int mtex,m;
	mtex=glutCreateMenu(Fmtex);
	glutAddMenuEntry("Oro",0);
	glutAddMenuEntry("Relieve",1);
	glutAddMenuEntry("Antiguo",2);
	glutAddMenuEntry("Ciudades",3);
	glutAddMenuEntry("Zonas horarias",4);
	glutAddMenuEntry("Noche",5);

	m=glutCreateMenu(Fm);
	glutAddSubMenu("Texturas",mtex);
	glutAddMenuEntry("Meridiano",6);
	glutAddMenuEntry("Hora local",7);
	glutAddMenuEntry("Salir",8);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
