// Helper functions for house.c

#include <math.h>
#define  PI  3.14159265

//------------------------------------------------------------
//------------------------------------------------------------
int LoadBMP(char *filename, int tex_name);
void escalera(GLfloat base, GLfloat altura, GLfloat ancho, GLint nEsc, GLint tx[6]);
void vCristal(GLfloat pt0[3], GLfloat pt1[3], GLfloat profund, GLfloat altura);
void vCubo(GLfloat pt0[3], GLfloat pt1[3], GLfloat profund, GLfloat altura, GLint tx[6]);
void room(GLint lado);
void casa(GLfloat baseR);
//------------------------------------------------------------
//------------------------------------------------------------



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






void escalera(GLfloat base, GLfloat altura, GLfloat ancho, GLint nEsc, GLint tx[6]){
	GLint i;
	GLfloat hEsc=altura/nEsc, pEsc=base/nEsc, v[2][3];
	for(i=0; i<nEsc; i++){
		v[0][0]=0;		v[0][1]=i*hEsc;		v[0][2]=-1*i*pEsc;
		v[1][0]=ancho;	v[1][1]=i*hEsc;		v[1][2]=-1*i*pEsc;
		vCubo(v[0],v[1], pEsc,hEsc, tx);
	}
}





//dibujar cristal vertical
void vCristal(GLfloat pt0[3], GLfloat pt1[3], GLfloat profund, GLfloat altura){
	GLfloat ang;
	GLdouble ancho;
	GLfloat p0[3], p1[3], p2[3], p3[3], p4[3], p5[3], p6[3], p7[3];
	ancho = sqrt( (pt0[0]-pt1[0])*(pt0[0]-pt1[0]) + (pt0[2]-pt1[2])*(pt0[2]-pt1[2]) );	//distancia entre pt0 y pt1
	glPushMatrix();

	//trasladar origen a pt0, eje +X sobre segmento pt0-pt1
	glTranslatef(pt0[0], pt0[1], pt0[2]);
	if( pt1[0]==pt0[0] ){  if( pt1[2]>=pt0[2] ) ang=90;  else ang=-90;  }  //pt1 sobre o debajo de pt0
	else{
		ang=atan( fabs(pt0[2]-pt1[2]) / fabs(pt0[0]-pt1[0]) );  //angulo desde el eje X hasta pt1 (0 a 90)
		ang*=180.0/PI;
		if( pt1[0]>pt0[0] ){  if( pt1[2]<pt0[2] ) ang*=-1.0;  }  //pt1 a la derecha y abajo de pt0
		else{
			if( pt1[2]>pt0[2] ) ang=180-ang;  //pt1 a la izquierda y arriba de pt0
			else ang+=180;  //pt1 a la izquierda y abajo de pt0
		}
	}
	glRotatef(-ang, 0.0,1.0,0.0);  //rotar desde +X hacia +Z  (y no hacia -Z)
	profund*=-1;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glColor4f(0,1,1,0.35);
	//dibujar muro vertical
	//(ahora el origen de coordenadas esta en pt0, y el eje +X hacia pt1)
	p0[0]=0;		p0[1]=0;		p0[2]=0;  //frente
	p1[0]=ancho;	p1[1]=0;		p1[2]=0;
	p2[0]=0;		p2[1]=altura;	p2[2]=0;
	p3[0]=ancho;	p3[1]=altura;	p3[2]=0;
	p4[0]=0;		p4[1]=0;		p4[2]=profund;  //atras
	p5[0]=ancho;	p5[1]=0;		p5[2]=profund;
	p6[0]=0;		p6[1]=altura;	p6[2]=profund;
	p7[0]=ancho;	p7[1]=altura;	p7[2]=profund;
	glFrontFace(GL_CW);
	glBegin(GL_QUADS);
		//frente
		glNormal3f(0,0,1);
		glVertex3fv(p0);	glVertex3fv(p2);	glVertex3fv(p3);	glVertex3fv(p1);
		//atras
		glNormal3f(0,0,-1);
		glVertex3fv(p5);	glVertex3fv(p7);	glVertex3fv(p6);	glVertex3fv(p4);
		//base
		glNormal3f(0,-1,0);
		glVertex3fv(p4);	glVertex3fv(p0);	glVertex3fv(p1);	glVertex3fv(p5);
		//derecha
		glNormal3f(1,0,0);
		glVertex3fv(p1);	glVertex3fv(p3);	glVertex3fv(p7);	glVertex3fv(p5);
		//arriba
		glNormal3f(0,1,0);
		glVertex3fv(p2);	glVertex3fv(p6);	glVertex3fv(p7);	glVertex3fv(p3);
		//izquierda
		glNormal3f(-1,0,0);
		glVertex3fv(p4);	glVertex3fv(p6);	glVertex3fv(p2);	glVertex3fv(p0);
	glEnd();

	glDisable(GL_BLEND);
	glPopMatrix();
}




//dibujar muro vertical con textura
void vCubo(GLfloat pt0[3], GLfloat pt1[3], GLfloat profund, GLfloat altura, GLint tx[6]){
	GLfloat ang;
	GLdouble ancho;
	GLfloat p0[3], p1[3], p2[3], p3[3], p4[3], p5[3], p6[3], p7[3];
	ancho = sqrt( (pt0[0]-pt1[0])*(pt0[0]-pt1[0]) + (pt0[2]-pt1[2])*(pt0[2]-pt1[2]) );	//distancia entre pt0 y pt1
	glPushMatrix();

	//trasladar origen a pt0, eje +X sobre segmento pt0-pt1
	glTranslatef(pt0[0], pt0[1], pt0[2]);
	if( pt1[0]==pt0[0] ){  if( pt1[2]>=pt0[2] ) ang=90;  else ang=-90;  }  //pt1 sobre o debajo de pt0
	else{
		ang=atan( fabs(pt0[2]-pt1[2]) / fabs(pt0[0]-pt1[0]) );  //angulo desde el eje X hasta pt1 (0 a 90)
		ang*=180.0/PI;
		if( pt1[0]>pt0[0] ){  if( pt1[2]<pt0[2] ) ang*=-1.0;  }  //pt1 a la derecha y abajo de pt0
		else{
			if( pt1[2]>pt0[2] ) ang=180-ang;  //pt1 a la izquierda y arriba de pt0
			else ang+=180;  //pt1 a la izquierda y abajo de pt0
		}
	}
	glRotatef(-ang, 0.0,1.0,0.0);  //rotar desde +X hacia +Z  (y no hacia -Z)
	profund*=-1;
	//dibujar muro vertical
	//(ahora el origen de coordenadas esta en pt0, y el eje +X hacia pt1)
	p0[0]=0;		p0[1]=0;		p0[2]=0;  //frente
	p1[0]=ancho;	p1[1]=0;		p1[2]=0;
	p2[0]=0;		p2[1]=altura;	p2[2]=0;
	p3[0]=ancho;	p3[1]=altura;	p3[2]=0;
	p4[0]=0;		p4[1]=0;		p4[2]=profund;  //atras
	p5[0]=ancho;	p5[1]=0;		p5[2]=profund;
	p6[0]=0;		p6[1]=altura;	p6[2]=profund;
	p7[0]=ancho;	p7[1]=altura;	p7[2]=profund;
	glFrontFace(GL_CW);
	glBindTexture(GL_TEXTURE_2D,tx[0]);
	glBegin(GL_QUADS);
		//frente
		glNormal3f(0,0,1);
		glTexCoord2f(0,0);  glVertex3fv(p0);
		glTexCoord2f(0,1);  glVertex3fv(p2);
		glTexCoord2f(1,1);  glVertex3fv(p3);
		glTexCoord2f(1,0);  glVertex3fv(p1);
	glEnd();
	if(tx[1]!=-1) glBindTexture(GL_TEXTURE_2D,tx[1]);
	glBegin(GL_QUADS);
		//atras
		glNormal3f(0,0,-1);
		glTexCoord2f(0,0);  glVertex3fv(p5);
		glTexCoord2f(0,1);  glVertex3fv(p7);
		glTexCoord2f(1,1);  glVertex3fv(p6);
		glTexCoord2f(1,0);  glVertex3fv(p4);
	glEnd();
	if(tx[1]!=-1) glBindTexture(GL_TEXTURE_2D,tx[5]);
	glBegin(GL_QUADS);
		//base
		glNormal3f(0,-1,0);
		glTexCoord2f(0,0);  glVertex3fv(p4);
		glTexCoord2f(0,1);  glVertex3fv(p0);
		glTexCoord2f(1,1);  glVertex3fv(p1);
		glTexCoord2f(1,0);  glVertex3fv(p5);
	glEnd();
	if(tx[1]!=-1) glBindTexture(GL_TEXTURE_2D,tx[4]);
	glBegin(GL_QUADS);
		//derecha
		glNormal3f(1,0,0);
		glTexCoord2f(0,0);  glVertex3fv(p1);
		glTexCoord2f(0,1);  glVertex3fv(p3);
		glTexCoord2f(1,1);  glVertex3fv(p7);
		glTexCoord2f(1,0);  glVertex3fv(p5);
	glEnd();
	if(tx[1]!=-1) glBindTexture(GL_TEXTURE_2D,tx[3]);
	glBegin(GL_QUADS);
		//arriba
		glNormal3f(0,1,0);
		glTexCoord2f(0,0);  glVertex3fv(p2);
		glTexCoord2f(0,1);  glVertex3fv(p6);
		glTexCoord2f(1,1);  glVertex3fv(p7);
		glTexCoord2f(1,0);  glVertex3fv(p3);
	glEnd();
	if(tx[1]!=-1) glBindTexture(GL_TEXTURE_2D,tx[2]);
	glBegin(GL_QUADS);
		//izquierda
		glNormal3f(-1,0,0);
		glTexCoord2f(0,0);  glVertex3fv(p4);
		glTexCoord2f(0,1);  glVertex3fv(p6);
		glTexCoord2f(1,1);  glVertex3fv(p2);
		glTexCoord2f(1,0);  glVertex3fv(p0);
	glEnd();

	glPopMatrix();
}





//escenario
void room(GLint lado){
	GLint tPiso, tCielo;
	GLUquadricObj *Qflat;
	Qflat = gluNewQuadric();
	gluQuadricDrawStyle(Qflat, GLU_FILL);
	gluQuadricNormals(Qflat, GLU_SMOOTH);
	gluQuadricOrientation(Qflat, GLU_INSIDE);
	gluQuadricTexture(Qflat, GL_TRUE);

	//cargar texturas
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &tPiso);
	LoadBMP("img/pasto.bmp",tPiso);
	glGenTextures(1, &tCielo);
	LoadBMP("img/cielo.bmp",tCielo);
	glPushMatrix();
	glColor3f(1.0, 1.0, 1.0);

	//cielo
	glFrontFace(GL_CCW);
	glBindTexture(GL_TEXTURE_2D,tCielo);
	glRotatef(90.0, 0.0,0.0,1.0);
	gluSphere(Qflat, lado, 20, 20);
	//piso
	glPopMatrix();
	glPushMatrix();
	glFrontFace(GL_CW);
	glBindTexture(GL_TEXTURE_2D,tPiso);
	glBegin(GL_QUADS);
		glNormal3f(0.0, 1.0, 0.0);
		glTexCoord2f(0.0, 0.0);		glVertex3f(-1.0*lado, 0.0, lado);
		glTexCoord2f(0.0, 1.0);		glVertex3f(-1.0*lado, 0.0, -1.0*lado);
		glTexCoord2f(1.0, 1.0);		glVertex3f(lado, 0.0, -1.0*lado);
		glTexCoord2f(1.0, 0.0);		glVertex3f(lado, 0.0, lado);
	glEnd();

	glFrontFace(GL_CCW);
	glPopMatrix();
}





void casa(GLfloat baseR){
	GLfloat v[3][3], hMuro=67.273, base=310, pMuro=6, refl[4], lado=50;
	GLint tMuro, tPiso, tex[6];
	GLint  tAvion, tEiffel, tGato, tSkysurf, tCastle, tCowco, tDragon, tMatrix, tVoid;

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &tMuro);
	LoadBMP("img/pared.bmp",tMuro);
	glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glGenTextures(1, &tPiso);
	LoadBMP("img/piso1.bmp",tPiso);
	glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glGenTextures(1, &tAvion);
	LoadBMP("img/avion.bmp",tAvion);
	glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glGenTextures(1, &tEiffel);
	LoadBMP("img/eiffel.bmp",tEiffel);
	glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glGenTextures(1, &tGato);
	LoadBMP("img/gato.bmp",tGato);
	glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glGenTextures(1, &tSkysurf);
	LoadBMP("img/skysurf.bmp",tSkysurf);
	glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glGenTextures(1, &tCastle);
	LoadBMP("img/castle.bmp",tCastle);
	glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glGenTextures(1, &tCowco);
	LoadBMP("img/cowco.bmp",tCowco);
	glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glGenTextures(1, &tDragon);
	LoadBMP("img/dragon.bmp",tDragon);
	glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glGenTextures(1, &tMatrix);
	LoadBMP("img/matrix.bmp",tMatrix);
	glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glGenTextures(1, &tVoid);
	LoadBMP("img/void.bmp",tVoid);
	glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glPushMatrix();
	glScalef(baseR/base, baseR/base, baseR/base);
	glColor3f(1.0, 1.0, 1.0);
	glTranslatef(-base/2,0,-base/2);

	//planta baja
	refl[0]=0.15;	refl[1]=0.15;	refl[2]=0.15;	refl[3]=1.0;
	glMaterialfv(GL_FRONT, GL_SPECULAR, refl);
	tex[0]=tPiso;		tex[1]=-1;	//piso
	v[0][0]=pMuro;		v[0][1]=1;	v[0][2]=264;
	v[1][0]=279;		v[1][1]=1;	v[1][2]=264;
	vCubo(v[0],v[1], 264-pMuro,pMuro, tex);
	v[0][0]=pMuro;		v[0][1]=1;	v[0][2]=310-pMuro;
	v[1][0]=135-pMuro;	v[1][1]=1;	v[1][2]=310-pMuro;
	vCubo(v[0],v[1], 46-pMuro,pMuro, tex);
	v[0][0]=279;		v[0][1]=1;	v[0][2]=103-pMuro;
	v[1][0]=300-pMuro;	v[1][1]=1;	v[1][2]=103-pMuro;
	vCubo(v[0],v[1], 103-2*pMuro,pMuro, tex);
	tex[0]=tMuro;	//muros
	v[0][0]=135;	v[0][1]=0;	v[0][2]=310;
	v[1][0]=135;	v[1][1]=0;	v[1][2]=264;
	vCubo(v[0],v[1], pMuro,hMuro, tex);
	v[0][0]=0;		v[0][1]=0;	v[0][2]=310;
	v[1][0]=135;	v[1][1]=0;	v[1][2]=310;
	vCubo(v[0],v[1], pMuro,hMuro, tex);
	v[0][0]=0;		v[0][1]=0;	v[0][2]=0;
	v[1][0]=0;		v[1][1]=0;	v[1][2]=310;
	vCubo(v[0],v[1], pMuro,hMuro, tex);
	v[0][0]=300;	v[0][1]=0;	v[0][2]=0;
	v[1][0]=0;		v[1][1]=0;	v[1][2]=0;
	vCubo(v[0],v[1], pMuro,hMuro, tex);
	v[0][0]=300;	v[0][1]=0;	v[0][2]=103;
	v[1][0]=300;	v[1][1]=0;	v[1][2]=0;
	vCubo(v[0],v[1], pMuro,hMuro, tex);
	v[0][0]=232;	v[0][1]=pMuro;	v[0][2]=103;
	v[1][0]=279;	v[1][1]=pMuro;	v[1][2]=103;
	vCubo(v[0],v[1], pMuro,hMuro-pMuro, tex);
	//escalera
	v[0][0]=209;	v[0][1]=20+pMuro;	v[0][2]=264;
	v[1][0]=279;	v[1][1]=20+pMuro;	v[1][2]=264;
	vCubo(v[0],v[1], 24,pMuro, tex);
	glPushMatrix();
	glTranslatef(244,pMuro,207);
	glRotatef(180, 0,1,0);
	escalera(33,20,35,3,tex);  //parte1
	glPopMatrix();
	glPushMatrix();
	glTranslatef(244,20+2*pMuro,240);
	escalera(91,hMuro-20-2*pMuro,35,8,tex);  //parte2
	glPopMatrix();
	
	//primer piso
	tex[0]=tPiso;  //piso
	v[0][0]=pMuro;		v[0][1]=hMuro;	v[0][2]=264;
	v[1][0]=209;		v[1][1]=hMuro;	v[1][2]=264;
	vCubo(v[0],v[1], 264-pMuro,pMuro, tex);
	v[0][0]=pMuro;		v[0][1]=hMuro;	v[0][2]=310-pMuro;
	v[1][0]=135-pMuro;	v[1][1]=hMuro;	v[1][2]=310-pMuro;
	vCubo(v[0],v[1], 46-pMuro,pMuro, tex);
	v[0][0]=279;		v[0][1]=hMuro;	v[0][2]=103-pMuro;
	v[1][0]=300-pMuro;	v[1][1]=hMuro;	v[1][2]=103-pMuro;
	vCubo(v[0],v[1], 103-2*pMuro,pMuro, tex);
	v[0][0]=209;		v[0][1]=hMuro;	v[0][2]=149;
	v[1][0]=279;		v[1][1]=hMuro;	v[1][2]=149;
	vCubo(v[0],v[1], 149-pMuro,pMuro, tex);
	tex[0]=tMuro;  //muros
	v[0][0]=0;		v[0][1]=hMuro;	v[0][2]=310;
	v[1][0]=135;	v[1][1]=hMuro;	v[1][2]=310;
	vCubo(v[0],v[1], pMuro,hMuro, tex);
	v[0][0]=0;		v[0][1]=hMuro;	v[0][2]=103;
	v[1][0]=0;		v[1][1]=hMuro;	v[1][2]=310;
	vCubo(v[0],v[1], pMuro,hMuro, tex);
	v[0][0]=300;	v[0][1]=hMuro;	v[0][2]=0;
	v[1][0]=0;		v[1][1]=hMuro;	v[1][2]=0;
	vCubo(v[0],v[1], pMuro,hMuro, tex);
	v[0][0]=300;	v[0][1]=hMuro;	v[0][2]=103;
	v[1][0]=300;	v[1][1]=hMuro;	v[1][2]=0;
	vCubo(v[0],v[1], pMuro,hMuro, tex);

	//escalera posterior
	v[0][0]=-35;	v[0][1]=hMuro;	v[0][2]=-35;
	v[1][0]=-35;	v[1][1]=hMuro;	v[1][2]=103;
	vCubo(v[0],v[1], 35,pMuro, tex);
	glPushMatrix();
	glTranslatef(110,0,0);
	glRotatef(90, 0,1,0);
	escalera(110,hMuro,35,10,tex);
	glPopMatrix();

	//techo
	v[0][0]=0;			v[0][1]=2*hMuro;	v[0][2]=264+pMuro;
	v[1][0]=279+pMuro;	v[1][1]=2*hMuro;	v[1][2]=264+pMuro;
	vCubo(v[0],v[1], 264+pMuro,pMuro, tex);
	v[0][0]=0;			v[0][1]=2*hMuro;	v[0][2]=310;
	v[1][0]=135;		v[1][1]=2*hMuro;	v[1][2]=310;
	vCubo(v[0],v[1], 46,pMuro, tex);
	v[0][0]=279;		v[0][1]=2*hMuro;	v[0][2]=103;
	v[1][0]=300;		v[1][1]=2*hMuro;	v[1][2]=103;
	vCubo(v[0],v[1], 103,pMuro, tex);

	//cubos
	glPushMatrix();
	glTranslatef(150,hMuro+pMuro,150);		glRotatef(45, 0,1,0);	glTranslatef(lado/2, 0, -1*lado/2);
	tex[0]=tGato;	tex[1]=tAvion;	tex[2]=tCastle;	tex[4]=tEiffel;		tex[3]=tex[5]=tVoid;
	v[0][0]=0;			v[0][1]=0;		v[0][2]=0;
	v[1][0]=50;			v[1][1]=0;		v[1][2]=0;
	vCubo(v[0],v[1], lado,lado, tex);

	glTranslatef(0, -1*hMuro, 0);
	tex[0]=tSkysurf;	tex[1]=tMatrix;	tex[2]=tCowco;	tex[4]=tDragon;
	vCubo(v[0],v[1], lado,lado, tex);
	glPopMatrix();

	//cristales
	refl[0]=0.5;	refl[1]=0.5;	refl[2]=0.5;	refl[3]=1.0;
	glMaterialfv(GL_FRONT, GL_SPECULAR, refl);
	v[0][0]=135;		v[0][1]=0;	v[0][2]=264+pMuro;
	v[1][0]=279;		v[1][1]=0;	v[1][2]=264+pMuro;
	vCristal(v[0],v[1], pMuro,2*hMuro);
	v[0][0]=279+pMuro;	v[0][1]=0;	v[0][2]=264+pMuro;
	v[1][0]=279+pMuro;	v[1][1]=0;	v[1][2]=103;
	vCristal(v[0],v[1], pMuro,2*hMuro);
	v[0][0]=279;		v[0][1]=0;	v[0][2]=103;
	v[1][0]=300-pMuro;	v[1][1]=0;	v[1][2]=103;
	vCristal(v[0],v[1], pMuro,2*hMuro);
	v[0][0]=0;		v[0][1]=hMuro;	v[0][2]=pMuro;
	v[1][0]=0;		v[1][1]=hMuro;	v[1][2]=103;
	vCristal(v[0],v[1], pMuro,hMuro);
	v[0][0]=135;	v[0][1]=hMuro;	v[0][2]=310;
	v[1][0]=135;	v[1][1]=hMuro;	v[1][2]=264;
	vCristal(v[0],v[1], pMuro,hMuro);

	glPopMatrix();
}
