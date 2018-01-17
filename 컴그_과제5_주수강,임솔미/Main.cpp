#include <vector>
#include <ctype.h>

#include <windows.h>

#include <GL/gl.h>
#include <GL/freeglut.h>
#include <math.h>
#include <stdio.h>

struct pos3d {
	float x, y, z;
	float normal[3] = { 0,0,0 };
};

#define	BUFFER_SIZE	20000							// length of longest input line in ".obj" file (including continuations)
#define	FACE_SIZE	4096							// maximum number of vertices in a single polygon
#define	MAXSTRING	1024							// string buffer size
#define	SAME(_a, _b)	(strcmp(_a,_b) == 0)		// case sensitive string equality test
#define PI	3.14159265	
#define	checkImageWidth 64
#define	checkImageHeight 64
static GLubyte checkImage[checkImageHeight][checkImageWidth][4];
std::vector<pos3d> varray;
std::vector<std::vector<int> > polyarray;
int spinStartx = 0;
int spinStarty = 0;
int spinDx = 0;
int spinDy = 0;
int spinX = 0;
int spinY = 0;
int viewMode = -1;
int randerMode = 0;
int frameW = 500;
int frameH = 500;
bool textureFlag = false;


float IDi[3] = { 0.7, 0.7, 0.7 };
float ISp[3] = { 1, 1, 1 };
float IAm[3] = { 0.7, 0.7, 0.7 };

float KDi[3] = { 0.7, 0.7, 0.7 };
float KSp[3] = { 1, 1, 1 };
float KAm[3] = { 0.7, 0.7, 0.7 };
float betha = 10;

float light_position[] = { 10.0, 10.0, 10.0, 1 };
void makeCheckImage(void)
{
	int i, j, c;

	for (i = 0; i < checkImageHeight; i++) {
		for (j = 0; j < checkImageWidth; j++) {
			c = ((((i & 0x8) == 0) ^ ((j & 0x8)) == 0)) * 254 + 1;
			if (c == 1) {
				checkImage[i][j][0] = (GLubyte)c*251;
				checkImage[i][j][1] = (GLubyte)c*207;
				checkImage[i][j][2] = (GLubyte)c*64;
			}
			else {
				checkImage[i][j][0] = (GLubyte)c;
				checkImage[i][j][1] = (GLubyte)c;
				checkImage[i][j][2] = (GLubyte)c;
			}

			checkImage[i][j][3] = (GLubyte)255;
		}
	}
}
void init() {
	makeCheckImage();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, 4, checkImageWidth, checkImageHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);
}
bool openObj(char *filename, std::vector<pos3d> &varray, std::vector<std::vector<int> > &polyarray)
{
	FILE		*objFile;

	char		buffer[BUFFER_SIZE];
	char		token[BUFFER_SIZE];
	char		*next = NULL;
	char		*backslash = NULL;
	char		before_token[BUFFER_SIZE];
	int			width, nVertex, ntVertex, nPolygonCounts, nPolygonConnects;

	width = nVertex = ntVertex = nPolygonCounts = nPolygonConnects = 0;

	pos3d p;

	varray.clear();
	polyarray.clear();

	int vtxIndex = 0;
	int faceIndex = 0;

	if ((objFile = fopen(filename, "r")) == NULL)
		return false;

	while (fgets(buffer, BUFFER_SIZE, objFile) != NULL)	//한줄단위로 읽음
	{
		while ((backslash = strchr(buffer, '\\')) != NULL)
			if (fgets(backslash,
				(int)(BUFFER_SIZE - strlen(buffer)),
				objFile) == NULL)
				break;

		for (next = buffer; *next != '\0' && isspace(*next); next++)
			;	// EMPTY 

		if (*next == '\0' || *next == '#' ||
			*next == '!' || *next == '$')
			continue;

		sscanf(next, "%s%n", token, &width);
		next += width;

		if (SAME(token, "v")) {
			sscanf(next, "%f%f%f", &p.x, &p.y, &p.z);
			varray.push_back(p);
		}
		else if (SAME(token, "vt")) {
			sscanf(next, "%f%f%f", &p.x, &p.y, &p.z);
		}
		else if (SAME(token, "f") || SAME(token, "fo")) {
			int		vtxCnt;
			char	vertexData[256];

			std::vector<int> pindices;

			for (vtxCnt = 0; vtxCnt < FACE_SIZE; vtxCnt++) {
				if (sscanf(next, "%s%n", vertexData, &width) != 1)
					break;
				//printf("%s \n", next);
				next += width;
				pindices.push_back(atoi(vertexData) - 1);
			}
			polyarray.push_back(pindices);
		}

		strcpy(before_token, token);
	}

	fclose(objFile);

	return true;
}
void calRotatePos(float* x, float* y, float* z) {
	float tempX, tempY, tempZ;
	float temp2X, temp2Y, temp2Z;
	tempX = *x;
	tempY = cos(spinDy*PI / 180)**y - sin(spinDy*PI / 180)**z;
	tempZ = sin(spinDy*PI / 180)**y + cos(spinDy*PI / 180)**z;
	temp2X = cos(spinDx*PI / 180)*tempX + sin(spinDx*PI / 180)*tempZ;
	temp2Y = tempY;
	temp2Z = -sin(spinDx*PI / 180)*tempX + cos(spinDx*PI / 180)*tempZ;
	*x = temp2X;
	*y = temp2Y;
	*z = temp2Z;
}
void calColor(float x, float y, float z,float nVector[] ,float* r, float* g, float* b) {
	float vx, vy, vz, lx,ly,lz,hx,hy,hz;
	float lVector[3];
	float vVector[3];
	float hVector[3];
	vx = 0 - x;
	vy = 0 - y;
	vz = 20 - z;
	vVector[0] = vx / sqrt(vx*vx + vy*vy + vz*vz);
	vVector[1] = vy / sqrt(vx*vx + vy*vy + vz*vz);
	vVector[2] = vz / sqrt(vx*vx + vy*vy + vz*vz);
	lx = light_position[0] - x;
	ly = light_position[1] - y;
	lz = light_position[2] - z;
	lVector[0] = lx / sqrt(lx*lx + ly*ly + lz*lz);
	lVector[1] = ly / sqrt(lx*lx + ly*ly + lz*lz);
	lVector[2] = lz / sqrt(lx*lx + ly*ly + lz*lz);
	hx = lVector[0] + vVector[0];
	hy = lVector[1] + vVector[1];
	hz = lVector[2] + vVector[2];
	hVector[0] = hx / sqrt(hx*hx + hy*hy + hz*hz);
	hVector[1] = hy / sqrt(hx*hx + hy*hy + hz*hz);
	hVector[2] = hz / sqrt(hx*hx + hy*hy + hz*hz);
	float l_n = lVector[0] * nVector[0] + lVector[1] * nVector[1] + lVector[2] * nVector[2];
	float n_h = nVector[0] * hVector[0] + nVector[1] * hVector[1] + nVector[2] * hVector[2];

	*r = KDi[0] * IDi[0] * l_n + KSp[0] * ISp[0] * pow(n_h, betha) + KAm[0] * IAm[0];
	*g = KDi[1] * IDi[1] * l_n + KSp[1] * ISp[1] * pow(n_h, betha) + KAm[1] * IAm[1];
	*b = KDi[2] * IDi[2] * l_n + KSp[2] * ISp[2] * pow(n_h, betha) + KAm[2] * IAm[2];
}
void calU(float* u, float a, float b) {
	float k = 1 / sqrt(a*a + b*b);
	if (b > 0) {
		if (a > 0) {
			*u = (asin(k*b) / (2 * PI)+0.25)-1*(int)(asin(k*b) / (2 * PI) + 0.25)%2;
		}
		else {
			*u = ((PI - asin(k*b)) / (2 * PI) + 0.25) - 1 * (int)((PI - asin(k*b)) / (2 * PI) + 0.25)%2;
		}
	}
	else {
		if (a > 0) {
			*u = ((2*PI + asin(k*b)) / (2 * PI) + 0.25) -1 * (int)((2 * PI + asin(k*b)) / (2 * PI) + 0.25)%2;
		}
		else {
			*u = ((PI - asin(k*b)) / (2 * PI) + 0.25)-1*(int)((PI - asin(k*b)) / (2 * PI) + 0.25)%2;
		}
	}
}
void drawWire() {
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glColor3f(0, 0, 0);
	for (int i = 0; i <= 1417; i++) {
		glBegin(GL_LINE_LOOP);
		glVertex3f(varray[polyarray[i][0]].x, varray[polyarray[i][0]].y, varray[polyarray[i][0]].z);
		glVertex3f(varray[polyarray[i][1]].x, varray[polyarray[i][1]].y, varray[polyarray[i][1]].z);
		glVertex3f(varray[polyarray[i][2]].x, varray[polyarray[i][2]].y, varray[polyarray[i][2]].z);
		glEnd();
	}
}
void drawHSR() {
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.f, 1.f);
	glColor3f(1, 1, 1);
	for (int i = 0; i <= 1417; i++) {
		float u;
		
		glBegin(GL_POLYGON);
		if (textureFlag) {
			calU(&u, varray[polyarray[i][0]].x, varray[polyarray[i][0]].z);
			glTexCoord2f(u, (varray[polyarray[i][0]].y + 3.256359) / 7.260807);
		}
		glVertex3f(varray[polyarray[i][0]].x, varray[polyarray[i][0]].y, varray[polyarray[i][0]].z);
		if (textureFlag) {
			calU(&u, varray[polyarray[i][1]].x, varray[polyarray[i][1]].z);
			glTexCoord2f(u, (varray[polyarray[i][1]].y + 3.256359) / 7.260807);
		}
		glVertex3f(varray[polyarray[i][1]].x, varray[polyarray[i][1]].y, varray[polyarray[i][1]].z);
		if (textureFlag) {
			calU(&u, varray[polyarray[i][2]].x, varray[polyarray[i][2]].z);
			glTexCoord2f(u, (varray[polyarray[i][2]].y+ 3.256359) / 7.260807);
		}
		glVertex3f(varray[polyarray[i][2]].x, varray[polyarray[i][2]].y, varray[polyarray[i][2]].z);
		glEnd();
	}


	glColor3f(0, 0, 0);
	for (int i = 0; i <= 1417; i++) {
		glBegin(GL_LINE_LOOP);
		glVertex3f(varray[polyarray[i][0]].x, varray[polyarray[i][0]].y, varray[polyarray[i][0]].z);
		glVertex3f(varray[polyarray[i][1]].x, varray[polyarray[i][1]].y, varray[polyarray[i][1]].z);
		glVertex3f(varray[polyarray[i][2]].x, varray[polyarray[i][2]].y, varray[polyarray[i][2]].z);
		glEnd();
	}
}
void drawFlat() {
	
	float nVector[3];
	float center[3];

	float x1, y1, z1, x2, y2, z2, x3, y3, z3;
	float a, b, c, d, e, f;
	float p, q, r;
	float colorR, colorG, colorB;

	glShadeModel(GL_FLAT);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	for (int i = 0; i <= 1417; i++) {
		x1 = varray[polyarray[i][0]].x;
		y1 = varray[polyarray[i][0]].y;
		z1 = varray[polyarray[i][0]].z;
		x2 = varray[polyarray[i][1]].x;
		y2 = varray[polyarray[i][1]].y;
		z2 = varray[polyarray[i][1]].z;
		x3 = varray[polyarray[i][2]].x;
		y3 = varray[polyarray[i][2]].y;
		z3 = varray[polyarray[i][2]].z;
		calRotatePos(&x1, &y1, &z1);
		calRotatePos(&x2, &y2, &z2);
		calRotatePos(&x3, &y3, &z3);
		a = x2 - x1;
		b = y2 - y1;
		c = z2 - z1;
		d = x3 - x1;
		e = y3 - y1;
		f = z3 - z1;
		p = b*f - e*c;
		q = c*d - a*f;
		r = a*e - b*d;
		nVector[0] = p / sqrt(p*p + q*q + r*r);
		nVector[1] = q / sqrt(p*p + q*q + r*r);
		nVector[2] = r / sqrt(p*p + q*q + r*r);

		center[0] = (x1 + x2 + x3) / 3;
		center[1] = (y1 + y2 + y3) / 3;
		center[2] = (z1 + z2 + z3) / 3;

		calColor(center[0], center[1], center[2], nVector, &colorR, &colorG, &colorB);
		glBegin(GL_POLYGON);
		glColor3f(colorR, colorG, colorB);
		float u;
		if (textureFlag) {
			calU(&u, varray[polyarray[i][0]].x, varray[polyarray[i][0]].z);
			glTexCoord2f(u , (varray[polyarray[i][0]].y+ 3.256359) / 7.260807);
		}
		glVertex3f(varray[polyarray[i][0]].x, varray[polyarray[i][0]].y, varray[polyarray[i][0]].z);
		
		if (textureFlag) {
			calU(&u, varray[polyarray[i][1]].x, varray[polyarray[i][1]].z);
			glTexCoord2f(u, (varray[polyarray[i][1]].y+ 3.256359) / 7.260807);
		}
		glVertex3f(varray[polyarray[i][1]].x, varray[polyarray[i][1]].y, varray[polyarray[i][1]].z);

		if (textureFlag) {
			calU(&u, varray[polyarray[i][2]].x, varray[polyarray[i][2]].z);
			glTexCoord2f(u, (varray[polyarray[i][2]].y+ 3.256359) / 7.260807);
		}
		glVertex3f(varray[polyarray[i][2]].x, varray[polyarray[i][2]].y, varray[polyarray[i][2]].z);
		glEnd();
	}
}
void drawSmooth() {
	float nVector[3];
	
	float x1, y1, z1, x2, y2, z2, x3, y3, z3;
	float a, b, c, d, e, f;
	float p, q, r;

	glShadeModel(GL_SMOOTH);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glColor3f(0, 0, 0);
	for (int i = 0; i <= 1417; i++) {
		x1 = varray[polyarray[i][0]].x;
		y1 = varray[polyarray[i][0]].y;
		z1 = varray[polyarray[i][0]].z;
		x2 = varray[polyarray[i][1]].x;
		y2 = varray[polyarray[i][1]].y;
		z2 = varray[polyarray[i][1]].z;
		x3 = varray[polyarray[i][2]].x;
		y3 = varray[polyarray[i][2]].y;
		z3 = varray[polyarray[i][2]].z;
		calRotatePos(&x1, &y1, &z1);
		calRotatePos(&x2, &y2, &z2);
		calRotatePos(&x3, &y3, &z3);
		a = x2 - x1;
		b = y2 - y1;
		c = z2 - z1;
		d = x3 - x1;
		e = y3 - y1;
		f = z3 - z1;
		p = b*f - e*c;
		q = c*d - a*f;
		r = a*e - b*d;
		nVector[0] = p / sqrt(p*p + q*q + r*r);
		nVector[1] = q / sqrt(p*p + q*q + r*r);
		nVector[2] = r / sqrt(p*p + q*q + r*r);
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				varray[polyarray[i][j]].normal[k] += nVector[k];
			}
		}
	}

	for (int i = 0; i <= 710; i++) {
		float normalizer = sqrt(varray[i].normal[0] * varray[i].normal[0] + varray[i].normal[1] * varray[i].normal[1] + varray[i].normal[2] * varray[i].normal[2]);
		varray[i].normal[0] = varray[i].normal[0] / normalizer;
		varray[i].normal[1] = varray[i].normal[1] / normalizer;
		varray[i].normal[2] = varray[i].normal[2] / normalizer;
	}
	for (int i = 0; i <= 1417; i++) {
		float vVector[3];
		float lVector[3];
		float hVector[3];
		float vx, vy, vz;
		float lx, ly, lz;
		float hx, hy, hz;
		float colorR, colorG, colorB;
		x1 = varray[polyarray[i][0]].x;
		y1 = varray[polyarray[i][0]].y;
		z1 = varray[polyarray[i][0]].z;
		x2 = varray[polyarray[i][1]].x;
		y2 = varray[polyarray[i][1]].y;
		z2 = varray[polyarray[i][1]].z;
		x3 = varray[polyarray[i][2]].x;
		y3 = varray[polyarray[i][2]].y;
		z3 = varray[polyarray[i][2]].z;
		calRotatePos(&x1, &y1, &z1);
		calRotatePos(&x2, &y2, &z2);
		calRotatePos(&x3, &y3, &z3);
		
		float u;
		glBegin(GL_POLYGON);

		calColor(x1, y1, z1, varray[polyarray[i][0]].normal, &colorR, &colorG, &colorB);
		glColor3f(colorR, colorG, colorB);
		if (textureFlag) {
			calU(&u, varray[polyarray[i][0]].x, varray[polyarray[i][0]].z);
			glTexCoord2f(u, (varray[polyarray[i][0]].y + 3.256359) / 7.260807);
		}
		glVertex3f(varray[polyarray[i][0]].x, varray[polyarray[i][0]].y, varray[polyarray[i][0]].z);

		calColor(x2, y2, z2, varray[polyarray[i][1]].normal, &colorR, &colorG, &colorB);
		glColor3f(colorR, colorG, colorB);
		if (textureFlag) {
			calU(&u, varray[polyarray[i][1]].x, varray[polyarray[i][1]].z);
			glTexCoord2f(u, (varray[polyarray[i][1]].y + 3.256359) / 7.260807);
		}
		glVertex3f(varray[polyarray[i][1]].x, varray[polyarray[i][1]].y, varray[polyarray[i][1]].z);

		calColor(x3, y3, z3, varray[polyarray[i][2]].normal, &colorR, &colorG, &colorB);
		glColor3f(colorR, colorG, colorB);
		if (textureFlag) {
			calU(&u, varray[polyarray[i][2]].x, varray[polyarray[i][2]].z);
			glTexCoord2f(u, (varray[polyarray[i][2]].y + 3.256359) / 7.260807);
		}
		glVertex3f(varray[polyarray[i][2]].x, varray[polyarray[i][2]].y, varray[polyarray[i][2]].z);

		glEnd();
	}
}
void myReshape(int w, int h) {
	glViewport((w - h) / 2, 0, h, h);	//500 500
	frameW = w; frameH = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	switch (viewMode) {
	case 0: {
		spinDx = 0; spinDy = 0;
		spinStartx = 0; spinStarty = 0;
		spinX = 0; spinY = 0;
		viewMode = -1;
		randerMode = 0;
	}
	case -1:
	case 1: glOrtho(-5, 5, -5, 5, 0.001, 1000.0);  break;
	case 2: {
		double theta = 2 * atan(5 / (double)20);
		gluPerspective(theta / PI * 180, 1, 0.0001, 1000.0);
		break;
	}
	default: break;
	}

}
void display(void) {
	glEnable(GL_DEPTH_TEST);
	if (textureFlag)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glColor3f(1.0, 1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);		//모델뷰로 변경한다.
	glLoadIdentity();				//변경 후 초기화 한번 해준다.
	gluLookAt(0.0, 0.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glRotatef(spinDx, 0, 1, 0);
	glRotatef(spinDy, 1, 0, 0);
	switch (randerMode)
	{
	case 0: drawWire(); break;
	case 1: drawHSR(); break;
	case 2: drawFlat(); break;
	case 3: drawSmooth(); break;
	default:
		break;
	}



	glutSwapBuffers();
}
void mouse(int button, int state, int x, int y) {
	switch (button) {
	case GLUT_LEFT_BUTTON:
		switch (state) {
		case GLUT_DOWN:
			spinStartx = x;
			spinStarty = y;
			glutPostRedisplay();
			break;
		case GLUT_UP:
			spinX = spinDx;
			spinY = spinDy;
		default:
			break;
		}
		break;

	default:
		break;
	}
}
void motion(int x, int y)
{
	spinDx = spinX + x - spinStartx;
	spinDy = spinY + y - spinStarty;
	glutPostRedisplay();
}
void keyfunc(unsigned char key, int x, int y)
{
	switch (key) {
	case 'w':randerMode = 0; glutPostRedisplay();
		break;
	case 'h':randerMode = 1; glutPostRedisplay();
		break;
	case 'f':randerMode = 2; glutPostRedisplay();
		break;
	case 's':randerMode = 3; glutPostRedisplay();
		break;
	case 't': {
		if (textureFlag) 
			textureFlag = false;
		else 
			textureFlag = true;
		glutPostRedisplay();
	} break;
	case 'q':
		exit(0);
		break;
	default:
		break;
	}
}
void menufunc(int value)
{

	switch (value) {		//우클릭했을때 나오는 메뉴다. 매번 pers와 ortho가 달라지므로, 매번 myreshape해줘야 한다.
	case 0: viewMode = 0; myReshape(frameW, frameH); break;
	case 1: viewMode = 1; myReshape(frameW, frameH); break;
	case 2: viewMode = 2; myReshape(frameW, frameH); break;

	default:break;

	}
	display();
}
void main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("컴그_과제5_주수강,임솔미");

	glutReshapeFunc(myReshape);
	glutDisplayFunc(display);
	init();
	openObj("Head_Man.obj", varray, polyarray);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyfunc);

	glutCreateMenu(menufunc);
	glutAddMenuEntry("Reset", 0);
	glutAddMenuEntry("Ortho", 1);
	glutAddMenuEntry("Persp", 2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glutSetMenu(0);

	glutMainLoop();
}