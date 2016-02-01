// Helper functions for clock.c

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

GLint lEsf, itexG=4, texG[6], zona, merid=1, seg,min,hor;
time_t tmp;

//-----------------------------------------
void reloj(void);
void esfera(void);
void cambiaMat(GLint sel);
void manecilla(void);
void meridiano(void);
void horaLocal(void);
void dibTri(float v[3][3]);
void calcNormal(float v[3][3], float out[3]);
void v(float v[3][3], float x0,float y0,float z0, float x1,float y1,float z1, float x2,float y2,float z2);
void copia2(GLfloat v[4][3]);
int LoadBMP(char *filename, int tex_name);
//-----------------------------------------


//dibuja el reloj
void reloj(void){
	GLint i;
	char tmp3[15];

	//esfera
	if(itexG==0||itexG==5) cambiaMat(0);  else cambiaMat(1);
	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D,texG[itexG]);
		glRotatef(-15*zona, 0,1,0);
		glCallList(lEsf);
	glPopMatrix();
	//segundero
	cambiaMat(2);
	glPushMatrix();
		glTranslatef(0,0,2);
		glRotatef(-6*seg, 0,0,1);
		glScalef(0.5,1,1);
		manecilla();
	glPopMatrix();
	//minutero
	glPushMatrix();
		glRotatef(-6*(min+seg/60.0), 0,0,1);
		glScalef(1,0.95,1);
		manecilla();
	glPopMatrix();
	//horaria
	glPushMatrix();
		glTranslatef(0,0,-2);
		glRotatef(-30*(hor+min/60.0), 0,0,1);
		glScalef(1,0.85,1);
		manecilla();
	glPopMatrix();
	//marcas
	glPushMatrix();
		cambiaMat(1);
		for(i=1;i<=12;i++){
			glRotatef(30, 0,0,1);
			glBegin(GL_TRIANGLES);
				glNormal3f(0,0,1);
				glVertex3f(0,47,0);		glVertex3f(1,49,0);		glVertex3f(-1,49,0);
				glNormal3f(0,0,-1);
				glVertex3f(0,47,-0.5);	glVertex3f(-1,49,-0.5);	glVertex3f(1,49,-0.5);
			glEnd();
		}
	glPopMatrix();
	//texto
	cambiaMat(3);
	sprintf(tmp3, "%02d:%02d:%02d UTC%+02d", hor,min,seg,zona);
	glRasterPos3f(-7,0,35);
	for(i=0;i<=14;i++)   glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, tmp3[i]);
	//meridiano
	if(merid){  cambiaMat(4);  meridiano();  }
}


void esfera(void){
	GLUquadricObj *Qflat=gluNewQuadric();

	gluQuadricDrawStyle(Qflat, GLU_FILL);
	gluQuadricNormals(Qflat, GLU_SMOOTH);
	gluQuadricOrientation(Qflat, GLU_OUTSIDE);
	gluQuadricTexture(Qflat, GL_TRUE);
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
		glFrontFace(GL_CCW);
		glRotatef(-90, 1,0,0);
		gluSphere(Qflat, 25, 30, 30);
	glPopMatrix();
	gluDeleteQuadric(Qflat);
	glDisable(GL_TEXTURE_2D);
}


//cambia el material actual
void cambiaMat(GLint sel){
	float ambiente[4], difusa[4], especular[4], brillo;
	switch(sel){
		case 0:		//laton
			ambiente[0]=0.329412;  ambiente[1]=0.223529;  ambiente[2]=0.027451;  ambiente[3]=1.0;
			difusa[0]=0.780392;  difusa[1]=0.568627;  difusa[2]=0.113725;  difusa[3]=1.0;
			especular[0]=0.992157;  especular[1]=0.941176;  especular[2]=0.807843;  especular[3]=1.0;
			brillo=27.8974;
			break;
		case 1:		//cromo
			ambiente[0]=0.25;  ambiente[1]=0.25;  ambiente[2]=0.25;  ambiente[3]=1.0;
			difusa[0]=0.4;  difusa[1]=0.4;  difusa[2]=0.4;  difusa[3]=1.0;
			especular[0]=0.774597;  especular[1]=0.774597;  especular[2]=0.774597;  especular[3]=1.0;
			brillo=76.8;
			break;
		case 2:		//cobre pulido
			ambiente[0]=0.2295;  ambiente[1]=0.08825;  ambiente[2]=0.0275;  ambiente[3]=1.0;
			difusa[0]=0.5508;  difusa[1]=0.2118;  difusa[2]=0.066;  difusa[3]=1.0;
			especular[0]=0.580594;  especular[1]=0.223257;  especular[2]=0.0695701;  especular[3]=1.0;
			brillo=51.2;
			break;
		case 3:		//texto
			especular[0]=0.774597;  especular[1]=0.774597;  especular[2]=0.774597;  especular[3]=1.0;
			ambiente[0]=especular[0];	ambiente[1]=especular[1];
			ambiente[2]=especular[2];	ambiente[3]=especular[3];
			difusa[0]=especular[0];		difusa[1]=especular[1];		difusa[2]=especular[2];
			brillo=76.8;
			break;
		case 4:		//cristal
			ambiente[0]=0;  ambiente[1]=0.25;  ambiente[2]=0.25;  ambiente[3]=0.35;
			difusa[0]=0;  difusa[1]=0.4;  difusa[2]=0.4;  difusa[3]=0.35;
			especular[0]=0.7;  especular[1]=0.7;  especular[2]=0.7;  especular[3]=0.35;
			brillo=76.8;
		break;
		default:  break;
	}
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambiente);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, difusa);
	glMaterialfv(GL_FRONT, GL_SPECULAR, especular);
	glMateriali(GL_FRONT, GL_SHININESS, brillo);
}


//establece 'zona' para la hora local
void zonaLocal(void){
	struct tm *tmp2;
	GLint horaLoc;
	time(&tmp);  tmp2=localtime(&tmp);  horaLoc=tmp2->tm_hour;  //hora local
	tmp2=gmtime(&tmp);  //hora UTC
	zona=horaLoc - tmp2->tm_hour;
	if(zona<-12) zona+=24;
	if(zona>12) zona-=24;
}


//dibuja un triangulo
void dibTri(float v[3][3]){
	GLfloat normal[3];
	calcNormal(v,normal);   glNormal3fv(normal);
	glVertex3fv(v[0]);   glVertex3fv(v[1]);   glVertex3fv(v[2]);
}


//dibuja una manecilla
void manecilla(void){
	float t1[3][3];
	glBegin(GL_TRIANGLES);
		v(t1, -6,20,0, 0,35,2, 0,45,0);		dibTri(t1); //izq
		v(t1, -6,20,0, -2.5,20,2, 0,35,2);	dibTri(t1);	
		v(t1, 6,20,0, 0,45,0, 0,35,2);		dibTri(t1); //der
		v(t1, 6,20,0, 0,35,2, 2.5,20,2);	dibTri(t1);
		v(t1, -6,20,0, 0,45,0, 6,20,0);		dibTri(t1); //atras
		v(t1, 2.5,20,2, 0,35,2, -2.5,20,2);	dibTri(t1); //frente
	glEnd();
}

//dibujar meridiano de cristal de 14 grados, radio 26
void meridiano(void){
	GLfloat p[4][3], normal[3], ang;
	glPushMatrix();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glBegin(GL_QUADS);
	p[0][1]=25.605;  p[0][2]=4.5149;  p[0][0]=-0.1219*p[0][2];	//punto sup-izq, a 10 grados de la cima
	p[1][0]=-1*p[0][0];  p[1][1]=p[0][1];  p[1][2]=p[0][2];		//punto sup-der, a 10 grados de la cima
	for(ang=0.349;ang<=2.967;ang+=0.174533){
		p[2][1]=26*cos(ang); p[2][2]=26*sin(ang);  p[2][0]=-0.1219*p[2][2]; //punto inf-izq, a ang grados de la cima
		p[3][0]=-1*p[2][0];  p[3][1]=p[2][1];  p[3][2]=p[2][2]; //punto inf-der, a ang grados de la cima
		calcNormal(p,normal);   normal[0]*=-1;  normal[1]*=-1;  normal[2]*=-1;  glNormal3fv(normal);
		glVertex3fv(p[0]);   glVertex3fv(p[2]);   glVertex3fv(p[3]);   glVertex3fv(p[1]);
		copia2(p);
	}
	glEnd();

	glDisable(GL_BLEND);
	glPopMatrix();
}

//asigna x0,y0,z0 al vector v[0], y etc
void v(float v[3][3], float x0,float y0,float z0, float x1,float y1,float z1, float x2,float y2,float z2){
	v[0][0]=x0;  v[0][1]=y0;  v[0][2]=z0;
	v[1][0]=x1;  v[1][1]=y1;  v[1][2]=z1;
	v[2][0]=x2;  v[2][1]=y2;  v[2][2]=z2;
}


//copiar v[2] y v[3], a v[0] y v[1], respectivamente
void copia2(GLfloat v[4][3]){
	v[0][0]=v[2][0];  v[0][1]=v[2][1];  v[0][2]=v[2][2];
	v[1][0]=v[3][0];  v[1][1]=v[3][1];  v[1][2]=v[3][2];
}


//calcula vector normal
void calcNormal(float v[3][3], float out[3]){
	float v1[3], v2[3], length;
	static const int x = 0;
	static const int y = 1;
	static const int z = 2;
	// Calcular dos vectores
	v1[x] = v[0][x] - v[1][x];
	v1[y] = v[0][y] - v[1][y];
	v1[z] = v[0][z] - v[1][z];
	v2[x] = v[1][x] - v[2][x];
	v2[y] = v[1][y] - v[2][y];
	v2[z] = v[1][z] - v[2][z];
	// Obtener en out el producto cruz de los dos vectores
	out[x] = v1[y]*v2[z] - v1[z]*v2[y];
	out[y] = v1[z]*v2[x] - v1[x]*v2[z];
	out[z] = v1[x]*v2[y] - v1[y]*v2[x];
	// Normalizar el vector
	length = (float)sqrt(  (out[0]*out[0]) + (out[1]*out[1]) + (out[2]*out[2])  );
	if(length == 0.0f)  length = 1.0f;
	out[0] /= length;
	out[1] /= length;
	out[2] /= length;
}


int LoadBMP(char *filename, int tex_name){
    #define SAIR        {fclose(fp_archivo); return -1;}
    #define CTOI(C)     (*(int*)&C)
    GLubyte     *image;
    GLubyte     Header[0x54]; 
    GLuint      DataPos, imageSize, i, t;
    GLsizei     Width,Height;
    int retorno;  //int nb = 0;
    // Abre un archivo y efectua la lectura del encabezado del archivo BMP
    FILE * fp_archivo = fopen(filename,"rb");
    if (!fp_archivo)  return -1;
    if (fread(Header,1,0x36,fp_archivo)!=0x36)  SAIR;
    if (Header[0]!='B' || Header[1]!='M')  SAIR;
    if (CTOI(Header[0x1E])!=0)  SAIR;
    if (CTOI(Header[0x1C])!=24)  SAIR;

    // Recupera los atributos de la altura y ancho de la imagen
    Width   = CTOI(Header[0x12]);
    Height  = CTOI(Header[0x16]);
    ( CTOI(Header[0x0A]) == 0 ) ? ( DataPos=0x36 ) : ( DataPos = CTOI(Header[0x0A]) );
    imageSize=Width*Height*3;
    
    // Llama a la imagen
    image = (GLubyte *) malloc ( imageSize );
	retorno = fread(image,1,imageSize,fp_archivo);
    if ( (unsigned)retorno != imageSize){  free (image);  SAIR;  }

    // Invierte los valores de R y B
    for ( i = 0; i < imageSize; i += 3 ){
        t = image[i];
        image[i] = image[i+2];
        image[i+2] = t;
	}

    // Tratamiento de textura para OpenGL
	glBindTexture(GL_TEXTURE_2D, tex_name);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

    glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
   
    // Manipulacion en memoria de la textura
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
                        
    fclose (fp_archivo);  free (image);  return 1;
}
