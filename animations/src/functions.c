// Helper functions for animations.c

//------------------------------------------------------------
//------------------------------------------------------------
int LoadBMP(char *filename, int tex_name);
void escalaVec(float escala, float *v, int size);
void copiaVec(float *vA, float *vDe, int size);
void calcNormal(float v[3][3], float out[3]);
void invertVec(float v[3]);
void castillo(float base[5], float baseT[6], float altura, float colorP[4], float colorT[4]);
void piso(float base, float altura, float baseT1, float baseT2, float alturaT, float colorP[4], float colorT[4]);
void jet(void);
void room(GLint lado);
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

    glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
   
    // Manipulacion en memoria de la textura
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
                        
    fclose (fp_archivo);  free (image);  return 1;
}



void escalaVec(float escala, float *v, int size){
	int i;
	for(i=0; i<size; i++) v[i]*=escala;
}



void copiaVec(float *vA, float *vDe, int size){
	int i;
	for(i=0; i<size; i++) vA[i]=vDe[i];
}



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



void invertVec(float v[3]){
	v[0]*= -1.0f;   v[1]*= -1.0f;   v[2]*= -1.0f;
}


//------------------------------------------------------------
//------------------------------------------------------------


void castillo(float base[5], float baseT[6], float altura, float colorP[4], float colorT[4]){
	float alturaP=altura/6.0f, normal[3], v[4][3];
	float alturaT=alturaP*0.25;
	glPushMatrix();
	//piso1
	glTranslatef(0.0, -1.0*altura/2.0, 0.0);
	piso(base[0], alturaP, base[1], baseT[0], alturaT, colorP, colorT);
	//piso2
	glTranslatef(0.0, alturaP, 0.0);
	piso(base[1], alturaP, base[2], baseT[1], alturaT, colorP, colorT);
	//piso3
	glTranslatef(0.0, alturaP, 0.0);
	piso(base[2], alturaP, base[3], baseT[2], alturaT, colorP, colorT);
	//piso4
	glTranslatef(0.0, alturaP, 0.0);
	piso(base[3], alturaP, base[4], baseT[3], alturaT, colorP, colorT);
	//piso5
	glTranslatef(0.0, alturaP, 0.0);
	piso(base[4], alturaP, baseT[5], baseT[4], alturaT, colorP, colorT);
	//punta
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0, -1.0*altura/2.0, 0.0);
	glFrontFace(GL_CW);
	glColor4fv(colorT);
	glBegin(GL_TRIANGLES);
		//triangulos
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f(-1.0*baseT[5], altura-alturaP+alturaT, baseT[5]);
		glVertex3f(0.0, altura, baseT[5]);
		glVertex3f(baseT[5], altura-alturaP+alturaT, baseT[5]); //*
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f(baseT[5], altura-alturaP+alturaT, -1.0*baseT[5]);
		glVertex3f(0.0, altura, -1.0*baseT[5]);
		glVertex3f(-1.0*baseT[5], altura-alturaP+alturaT, -1.0*baseT[5]);
	glEnd();
	glFrontFace(GL_CCW);
	glBegin(GL_QUADS);
		//lateral derecha
		v[0][0]=0.0;        v[0][1]=altura;     v[0][2]=-1.0*baseT[5];
		v[1][0]=0.0;        v[1][1]=altura;     v[1][2]=baseT[5];
		v[2][0]=baseT[5];   v[2][1]=altura-alturaP+alturaT;     v[2][2]=baseT[5];
		v[3][0]=baseT[5];   v[3][1]=altura-alturaP+alturaT;     v[3][2]=-1.0*baseT[5];
		calcNormal(v,normal);   glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
		glVertex3fv(v[3]); //*

		v[0][0]=baseT[5];   v[0][1]=altura-alturaP+alturaT;   v[0][2]=-1.0*baseT[5];
		v[1][0]=baseT[5];   v[1][1]=altura-alturaP+alturaT;   v[1][2]=baseT[5];
		v[2][0]=baseT[4];   v[2][1]=altura-alturaP-alturaT;   v[2][2]=baseT[4];
		v[3][0]=baseT[4];   v[3][1]=altura-alturaP-alturaT;   v[3][2]=-1.0*baseT[4];
		calcNormal(v,normal);   glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
		glVertex3fv(v[3]); //*

		//lateral izquierda
		v[0][0]=0.0;        v[0][1]=altura;     v[0][2]=baseT[5];
		v[1][0]=0.0;        v[1][1]=altura;     v[1][2]=-1.0*baseT[5];
		v[2][0]=-1.0*baseT[5];   v[2][1]=altura-alturaP+alturaT;     v[2][2]=-1.0*baseT[5];
		v[3][0]=-1.0*baseT[5];   v[3][1]=altura-alturaP+alturaT;     v[3][2]=baseT[5];
		calcNormal(v,normal);   glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
		glVertex3fv(v[3]); //*

		v[0][0]=-1.0*baseT[5];   v[0][1]=altura-alturaP+alturaT;   v[0][2]=baseT[5];
		v[1][0]=-1.0*baseT[5];   v[1][1]=altura-alturaP+alturaT;   v[1][2]=-1.0*baseT[5];
		v[2][0]=-1.0*baseT[4];   v[2][1]=altura-alturaP-alturaT;   v[2][2]=-1.0*baseT[4];
		v[3][0]=-1.0*baseT[4];   v[3][1]=altura-alturaP-alturaT;   v[3][2]=baseT[4];
		calcNormal(v,normal);   glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
		glVertex3fv(v[3]); //*
	glEnd();

	glPopMatrix();
}



void piso(float base, float altura, float baseT1, float baseT2, float alturaT, float colorP[4], float colorT[4]){
	float v[4][3], normal[3];

	glPushMatrix();
	glFrontFace(GL_CW);

	glColor4fv(colorP);
	//base -------------------------------
	glBegin(GL_QUADS);
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-1.0*base, 0.0, base);
		glVertex3f(base, 0.0, base);
		glVertex3f(base, 0.0, -1.0*base);
		glVertex3f(-1.0*base, 0.0, -1.0*base);
	glEnd();
	//paredes ----------------------------
	glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f(-1.0*base, 0.0, base);
		glVertex3f(-1.0*base, altura, base);
		glVertex3f(base, altura, base);
		glVertex3f(base, 0.0, base); //*
		glNormal3f(1.0, 0.0, 0.0);
		glVertex3f(base, 0.0, base);
		glVertex3f(base, altura, base);
		glVertex3f(base, altura, -1.0*base);
		glVertex3f(base, 0.0, -1.0*base); //*
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f(base, 0.0, -1.0*base);
		glVertex3f(base, altura, -1.0*base);
		glVertex3f(-1.0*base, altura, -1.0*base);
		glVertex3f(-1.0*base, 0.0, -1.0*base); //*
		glNormal3f(-1.0, 0.0, 0.0);
		glVertex3f(-1.0*base, 0.0, -1.0*base);
		glVertex3f(-1.0*base, altura, -1.0*base);
		glVertex3f(-1.0*base, altura, base);
		glVertex3f(-1.0*base, 0.0, base); //*
	glEnd();
	//tejados ------------------------------------
	glColor4fv(colorT);
	glBegin(GL_QUADS);  //t1
		v[0][0]=-1.0*baseT2;   v[0][1]=altura-alturaT;   v[0][2]=baseT2;
		v[1][0]=-1.0*baseT1;   v[1][1]=altura+alturaT;   v[1][2]=baseT1;
		v[2][0]=baseT1;    v[2][1]=altura+alturaT;   v[2][2]=baseT1;
		v[3][0]=baseT2;    v[3][1]=altura-alturaT;   v[3][2]=baseT2;
		calcNormal(v,normal);   invertVec(normal);   glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
		glVertex3fv(v[3]); //*

		v[0][0]=baseT2;    v[0][1]=altura-alturaT;   v[0][2]=baseT2;
		v[1][0]=baseT1;    v[1][1]=altura+alturaT;   v[1][2]=baseT1;
		v[2][0]=baseT1;    v[2][1]=altura+alturaT;   v[2][2]=-1.0*baseT1;
		v[3][0]=baseT2;    v[3][1]=altura-alturaT;   v[3][2]=-1.0*baseT2;
		calcNormal(v,normal);   invertVec(normal);   glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
		glVertex3fv(v[3]); //*

		v[0][0]=baseT2;    v[0][1]=altura-alturaT;   v[0][2]=-1.0*baseT2;
		v[1][0]=baseT1;    v[1][1]=altura+alturaT;   v[1][2]=-1.0*baseT1;
		v[2][0]=-1.0*baseT1;   v[2][1]=altura+alturaT;   v[2][2]=-1.0*baseT1;
		v[3][0]=-1.0*baseT2;   v[3][1]=altura-alturaT;   v[3][2]=-1.0*baseT2;
		calcNormal(v,normal);   invertVec(normal);   glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
		glVertex3fv(v[3]); //*

		v[0][0]=-1.0*baseT2;   v[0][1]=altura-alturaT;   v[0][2]=-1.0*baseT2;
		v[1][0]=-1.0*baseT1;   v[1][1]=altura+alturaT;   v[1][2]=-1.0*baseT1;
		v[2][0]=-1.0*baseT1;   v[2][1]=altura+alturaT;   v[2][2]=baseT1;
		v[3][0]=-1.0*baseT2;   v[3][1]=altura-alturaT;   v[3][2]=baseT2;
		calcNormal(v,normal);   invertVec(normal);   glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
		glVertex3fv(v[3]); //*
	glEnd();

	glBegin(GL_QUADS);  //parte baja del tejado
		v[0][0]=-1.0*base;   v[0][1]=altura;   v[0][2]=base;
		v[1][0]=-1.0*baseT2;  v[1][1]=altura-alturaT;   v[1][2]=baseT2;
		v[2][0]=baseT2;   v[2][1]=altura-alturaT;   v[2][2]=baseT2;
		v[3][0]=base;    v[3][1]=altura;   v[3][2]=base;
		calcNormal(v,normal);   invertVec(normal);   glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
		glVertex3fv(v[3]); //*

		v[0][0]=base;   v[0][1]=altura;   v[0][2]=base;
		v[1][0]=baseT2;  v[1][1]=altura-alturaT;   v[1][2]=baseT2;
		v[2][0]=baseT2;  v[2][1]=altura-alturaT;   v[2][2]=-1.0*baseT2;
		v[3][0]=base;   v[3][1]=altura;   v[3][2]=-1.0*base;
		calcNormal(v,normal);   invertVec(normal);   glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
		glVertex3fv(v[3]); //*

		v[0][0]=base;    v[0][1]=altura;   v[0][2]=-1.0*base;
		v[1][0]=baseT2;   v[1][1]=altura-alturaT;   v[1][2]=-1.0*baseT2;
		v[2][0]=-1.0*baseT2;  v[2][1]=altura-alturaT;   v[2][2]=-1.0*baseT2;
		v[3][0]=-1.0*base;   v[3][1]=altura;   v[3][2]=-1.0*base;
		calcNormal(v,normal);   invertVec(normal);   glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
		glVertex3fv(v[3]); //*

		v[0][0]=-1.0*base;   v[0][1]=altura;   v[0][2]=-1.0*base;
		v[1][0]=-1.0*baseT2;  v[1][1]=altura-alturaT;   v[1][2]=-1.0*baseT2;
		v[2][0]=-1.0*baseT2;  v[2][1]=altura-alturaT;   v[2][2]=baseT2;
		v[3][0]=-1.0*base;   v[3][1]=altura;   v[3][2]=base;
		calcNormal(v,normal);   invertVec(normal);   glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
		glVertex3fv(v[3]); //*
	glEnd();

	glPopMatrix();
}



//------------------------------------------------------------
//------------------------------------------------------------

void jet(void){
	GLint tNariz, tLiso, tAla0, tAla1, tCostado;
	
	glGenTextures(1, &tNariz);
	LoadBMP("img/nariz.bmp",tNariz);

	glGenTextures(1, &tLiso);
	LoadBMP("img/liso.bmp",tLiso);

	glGenTextures(1, &tCostado);
	LoadBMP("img/costado.bmp",tCostado);

	glGenTextures(1, &tAla0);
	LoadBMP("img/ala0.bmp",tAla0);

	glGenTextures(1, &tAla1);
	LoadBMP("img/ala1.bmp",tAla1);

	// nariz /////////////////////////////
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,tLiso);
	glBegin(GL_TRIANGLES);
            glTexCoord2f(0.0, 0.5);		glVertex3f(0.0f, 0.0f, 60.0f);  //abajo
            glTexCoord2f(1.0, 0.0);		glVertex3f(-15.0f, 0.0f, 30.0f);
            glTexCoord2f(1.0, 1.0);		glVertex3f(15.0f,0.0f,30.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D,tNariz);
	glBegin(GL_TRIANGLES);
            glTexCoord2f(1.0, 0.0);		glVertex3f(15.0f,0.0f,30.0f);  //derecha
            glTexCoord2f(1.0, 1.0);		glVertex3f(0.0f, 15.0f, 30.0f);
            glTexCoord2f(0.0, 0.796);	glVertex3f(0.0f, 0.0f, 60.0f);
	
            glTexCoord2f(0.0, 0.796);	glVertex3f(0.0f, 0.0f, 60.0f);  //izquierda
            glTexCoord2f(1.0, 1.0);		glVertex3f(0.0f, 15.0f, 30.0f);
            glTexCoord2f(1.0, 0.0);		glVertex3f(-15.0f,0.0f,30.0f);
	glEnd();


	// cuerpo ////////////////////////	
	glBindTexture(GL_TEXTURE_2D,tCostado);
	glBegin(GL_TRIANGLES);
            glTexCoord2f(0.0, 0.0);		glVertex3f(-15.0f,0.0f,30.0f);  //izq
            glTexCoord2f(0.0, 1.0);		glVertex3f(0.0f, 15.0f, 30.0f);
            glTexCoord2f(1.0, 0.0);		glVertex3f(0.0f, 0.0f, -56.0f);

            glTexCoord2f(1.0, 0.0);		glVertex3f(0.0f, 0.0f, -56.0f);  //der
            glTexCoord2f(0.0, 1.0);		glVertex3f(0.0f, 15.0f, 30.0f);
            glTexCoord2f(0.0, 0.0);		glVertex3f(15.0f,0.0f,30.0f);	
	glEnd();
	glBindTexture(GL_TEXTURE_2D,tLiso);
	glBegin(GL_TRIANGLES);
            glTexCoord2f(0.0, 0.0);		glVertex3f(15.0f,0.0f,30.0f);  //abajo
            glTexCoord2f(0.0, 1.0);		glVertex3f(-15.0f, 0.0f, 30.0f);
            glTexCoord2f(3.0, 0.5);		glVertex3f(0.0f, 0.0f, -56.0f);
	glEnd();


	// alas  ///////////////////////////////////////////
	glBindTexture(GL_TEXTURE_2D,tLiso);
	glBegin(GL_TRIANGLES);
            glTexCoord2f(1.0, 1.0);		glVertex3f(0.0f,2.0f,27.0f);  //abajo
            glTexCoord2f(0.0, 0.0);		glVertex3f(-60.0f, 2.0f, -8.0f);
            glTexCoord2f(0.0, 2.0);		glVertex3f(60.0f, 2.0f, -8.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D,tAla1);
	glBegin(GL_TRIANGLES);
            glTexCoord2f(0.0, 0.07);	glVertex3f(60.0f, 2.0f, -8.0f);  //der
            glTexCoord2f(0.47, 0.765);	glVertex3f(0.0f, 7.0f, -8.0f);
            glTexCoord2f(0.672, 0.141);	glVertex3f(0.0f,2.0f,27.0f);
	
			glTexCoord2f(0.672, 0.141);	glVertex3f(0.0f,2.0f,27.0f);  //izq
            glTexCoord2f(0.47, 0.765);	glVertex3f(0.0f, 7.0f, -8.0f);
            glTexCoord2f(0.0, 0.07);	glVertex3f(-60.0f, 2.0f, -8.0f);            
	glEnd();
	glBindTexture(GL_TEXTURE_2D,tAla0);
	glBegin(GL_TRIANGLES);
			glTexCoord2f(0.0, 0.07);	glVertex3f(60.0f, 2.0f, -8.0f);  //tapa de atras
            glTexCoord2f(0.672, 0.141);	glVertex3f(-60.0f, 2.0f, -8.0f);
            glTexCoord2f(0.47, 0.765);	glVertex3f(0.0f,7.0f,-8.0f);
	glEnd();


	// alas cola ///////////////////////////////
	glBindTexture(GL_TEXTURE_2D,tLiso);
	glBegin(GL_TRIANGLES);
            glTexCoord2f(1.0, 0.0);		glVertex3f(-30.0f, -0.50f, -57.0f);  // abajo
            glTexCoord2f(0.0, 0.0);		glVertex3f(30.0f, -0.50f, -57.0f);
            glTexCoord2f(0.5, 1.0);		glVertex3f(0.0f,-0.50f,-40.0f);
    glEnd();
	glBindTexture(GL_TEXTURE_2D,tAla0);
	glBegin(GL_TRIANGLES);
            glTexCoord2f(0.672, 0.141);	glVertex3f(0.0f,-0.5f,-40.0f);  //der
            glTexCoord2f(0.0, 0.07);	glVertex3f(30.0f, -0.5f, -57.0f);
            glTexCoord2f(0.47, 0.765);	glVertex3f(0.0f, 4.0f, -57.0f);

            glTexCoord2f(0.47, 0.765);	glVertex3f(0.0f, 4.0f, -57.0f);  //izq
            glTexCoord2f(0.0, 0.07);	glVertex3f(-30.0f, -0.5f, -57.0f);
            glTexCoord2f(0.672, 0.141);	glVertex3f(0.0f,-0.5f,-40.0f);

            glTexCoord2f(0.0, 0.07);	glVertex3f(30.0f,-0.5f,-57.0f);  //tapa de atras
            glTexCoord2f(0.672, 0.141);	glVertex3f(-30.0f, -0.5f, -57.0f);
            glTexCoord2f(0.47, 0.765);	glVertex3f(0.0f, 4.0f, -57.0f);
	glEnd();


	// vertical ///////////////////////////////
	glBindTexture(GL_TEXTURE_2D,tAla0);
	glBegin(GL_TRIANGLES);
            glTexCoord2f(0.672, 0.141);	glVertex3f(0.0f,0.5f,-40.0f);  //der
            glTexCoord2f(0.0, 0.07);	glVertex3f(3.0f, 0.5f, -57.0f);
            glTexCoord2f(0.47, 0.765);	glVertex3f(0.0f, 25.0f, -65.0f);

            glTexCoord2f(0.47, 0.765);	glVertex3f(0.0f, 25.0f, -65.0f);  //izq
            glTexCoord2f(0.0, 0.07);	glVertex3f(-3.0f, 0.5f, -57.0f);
            glTexCoord2f(0.672, 0.141);	glVertex3f(0.0f,0.5f,-40.0f);

            glTexCoord2f(0.0, 0.07);	glVertex3f(3.0f,0.5f,-57.0f);  //tapa de atras
            glTexCoord2f(0.672, 0.141);	glVertex3f(-3.0f, 0.5f, -57.0f);
            glTexCoord2f(0.47, 0.765);	glVertex3f(0.0f, 25.0f, -65.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}



void room(GLint lado){
	GLint tPiso, tPared, tCielo;
	GLUquadricObj *Qflat;
	Qflat = gluNewQuadric();
	gluQuadricDrawStyle(Qflat, GLU_FILL);
	gluQuadricNormals(Qflat, GLU_SMOOTH);
	gluQuadricOrientation(Qflat, GLU_INSIDE);
	gluQuadricTexture(Qflat, GL_TRUE);

	//cargar texturas
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &tPiso);
	LoadBMP("img/lava.bmp",tPiso);
	glGenTextures(1, &tPared);
	LoadBMP("img/solder.bmp",tPared);
	glGenTextures(1, &tCielo);
	LoadBMP("img/nubes.bmp",tCielo);
	glPushMatrix();

	//cielo
	glFrontFace(GL_CCW);
	glBindTexture(GL_TEXTURE_2D,tCielo);
	glRotatef(90.0, 0.0,0.0,1.0);
	gluSphere(Qflat, lado+50, 20, 20);
	//pared
	glPopMatrix();
	glPushMatrix();
	glColor3f(0.0, 0.0, 1.0);
	glTranslatef(0.0, 0.25*lado, 0.0);
	glRotatef(90.0, 1.0,0.0,0.0);
	glBindTexture(GL_TEXTURE_2D,tPared);
	gluCylinder(Qflat, lado, lado, 0.25*lado, 30, 10);
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
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

