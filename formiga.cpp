#define _CRT_SECURE_NO_WARNINGS
/*
 * ==============================================================================
 * Computacao Grafica (IME 04-10842) - Trabalho Final 2026.1
 * Projeto: Simulador de Formiga Robo 3D em OpenGL (Cyber Ant)
 * Autor: Mateus Henrique Freitas Maciel
 * Ponto de partida: garra.cpp
 * ==============================================================================
 */

#define PI 3.14159265358979323846

#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include "RgbImage.h"

using namespace std;

// Caminho relativo para textura conforme regras de portabilidade
const char* filenameTexMetal1 = "metalTexture1.bmp";

// Identificadores de textura e quadricas GLU
GLuint _textureIdMetal1;
GLuint _textureIdSphere;
GLuint _textureIdCylinder;
GLUquadric* quadSphere;
GLUquadric* quadCylinder;

// Estados globais de renderizacao
bool textureOn = true;

// Parametros de camera (baseados em garra.cpp, serao expandidos na Fase 1)
float eyeDistance = 20.0f;
float eyeX;
float eyeY;
float eyeZ;
float viewAngleX = 45.0f;
float viewAngleZ = 25.0f;

// Carrega uma imagem BMP 24-bits via RgbImage e registra como textura OpenGL
GLuint loadTexture(const char* filename) {
    GLuint textureId;
    RgbImage theTexMap(filename);

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D,
        0,
        GL_RGB,
        theTexMap.GetNumCols(),
        theTexMap.GetNumRows(),
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        theTexMap.ImageData());
    return textureId;
}

// Inicializa recursos de renderizacao, quadricas e texturas
void initRendering(void) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.08f, 0.09f, 0.12f, 1.0f);

    quadSphere = gluNewQuadric();
    gluQuadricNormals(quadSphere, GLU_SMOOTH);

    quadCylinder = gluNewQuadric();
    gluQuadricNormals(quadCylinder, GLU_SMOOTH);

    _textureIdMetal1 = loadTexture(filenameTexMetal1);
    _textureIdCylinder = _textureIdMetal1;
    _textureIdSphere = _textureIdMetal1;
}

// Primitivas basicas herdadas de garra.cpp com suporte a textura
void drawCylinder(float diameter, float length) {
    if (textureOn) {
        glBindTexture(GL_TEXTURE_2D, _textureIdCylinder);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gluQuadricTexture(quadCylinder, GL_TRUE);
    } else {
        gluQuadricTexture(quadCylinder, GL_FALSE);
    }
    gluCylinder(quadCylinder, diameter, diameter, length, 32, 16);
}

void drawCone(float diameter, float length) {
    if (textureOn) {
        glBindTexture(GL_TEXTURE_2D, _textureIdCylinder);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gluQuadricTexture(quadCylinder, GL_TRUE);
    } else {
        gluQuadricTexture(quadCylinder, GL_FALSE);
    }
    gluCylinder(quadCylinder, diameter, 0.0, length, 32, 16);
}

void drawDisk(float diameterInner, float diameterOuter) {
    if (textureOn) {
        glBindTexture(GL_TEXTURE_2D, _textureIdCylinder);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gluQuadricTexture(quadCylinder, GL_TRUE);
    } else {
        gluQuadricTexture(quadCylinder, GL_FALSE);
    }
    gluDisk(quadCylinder, diameterInner, diameterOuter, 32, 8);
}

void drawSphere(float diameter) {
    if (textureOn) {
        glBindTexture(GL_TEXTURE_2D, _textureIdSphere);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gluQuadricTexture(quadSphere, GL_TRUE);
    } else {
        gluQuadricTexture(quadSphere, GL_FALSE);
    }
    gluSphere(quadSphere, diameter, 32, 32);
}

// Grade de referencia no plano Z = -2.5 para orientacao espacial
void drawReferenceGrid(void) {
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.25f, 0.30f, 0.35f);
    glBegin(GL_LINES);
    for (int i = -15; i <= 15; i += 2) {
        glVertex3f((float)i, -15.0f, -2.5f);
        glVertex3f((float)i,  15.0f, -2.5f);
        glVertex3f(-15.0f, (float)i, -2.5f);
        glVertex3f( 15.0f, (float)i, -2.5f);
    }
    glEnd();
    if (textureOn) {
        glEnable(GL_TEXTURE_2D);
    }
}

void handleKeypress(unsigned char key, int /*x*/, int /*y*/) {
    switch (key) {
    case 27: // Tecla ESC
        exit(0);
    case 'w':
        if (viewAngleZ < 175.0f) viewAngleZ += 3.0f;
        glutPostRedisplay();
        break;
    case 'z':
    case 's':
        if (viewAngleZ > -85.0f) viewAngleZ -= 3.0f;
        glutPostRedisplay();
        break;
    case 'a':
        viewAngleX -= 3.0f;
        glutPostRedisplay();
        break;
    case 'd':
        viewAngleX += 3.0f;
        glutPostRedisplay();
        break;
    case 't':
    case 'T':
        textureOn = !textureOn;
        glutPostRedisplay();
        break;
    }
}

// Controle de camera pelas teclas de setas (Esquerda: horario, Direita: anti-horario, Cima/Baixo: elevacao)
void handleSpecialKeypress(int key, int /*x*/, int /*y*/) {
    switch (key) {
    case GLUT_KEY_LEFT:  // Gira sentido horario
        viewAngleX -= 3.0f;
        glutPostRedisplay();
        break;
    case GLUT_KEY_RIGHT: // Gira sentido anti-horario
        viewAngleX += 3.0f;
        glutPostRedisplay();
        break;
    case GLUT_KEY_UP:    // Gira para cima
        if (viewAngleZ < 175.0f) viewAngleZ += 3.0f;
        glutPostRedisplay();
        break;
    case GLUT_KEY_DOWN:  // Gira para baixo
        if (viewAngleZ > -85.0f) viewAngleZ -= 3.0f;
        glutPostRedisplay();
        break;
    }
}

void handleResize(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)w / (double)h, 0.5, 100.0);
}

void drawScene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (textureOn) {
        glEnable(GL_TEXTURE_2D);
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    eyeX = eyeDistance * (float)(cos(viewAngleZ * PI / 180.0) * cos(viewAngleX * PI / 180.0));
    eyeY = eyeDistance * (float)(cos(viewAngleZ * PI / 180.0) * sin(viewAngleX * PI / 180.0));
    eyeZ = eyeDistance * (float)(sin(viewAngleZ * PI / 180.0));

    if (cos(viewAngleZ * PI / 180.0) >= 0.0) {
        gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    } else {
        gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0);
    }

    // Grade de chao para referencia espacial
    drawReferenceGrid();

    // Raiz do modelo (Placeholder do Torax para validacao da Fase 0)
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
        drawSphere(1.5f);
    glPopMatrix();

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(900, 700);
    glutCreateWindow("Simulador Formiga Robo 3D - Cyber Ant");

    initRendering();
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(handleKeypress);
    glutSpecialFunc(handleSpecialKeypress);
    glutReshapeFunc(handleResize);

    glutMainLoop();
    return 0;
}
