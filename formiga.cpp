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

// Constantes padrao de camera para reset (Fase 1)
const float DEFAULT_EYE_DISTANCE = 20.0f;
const float DEFAULT_VIEW_ANGLE_X = 45.0f;
const float DEFAULT_VIEW_ANGLE_Z = 25.0f;
const float DEFAULT_TARGET_X = 0.0f;
const float DEFAULT_TARGET_Y = 0.0f;
const float DEFAULT_TARGET_Z = 0.0f;

// Parametros ativos de camera e centro de visao (Pan e Orbita)
float eyeDistance = DEFAULT_EYE_DISTANCE;
float viewAngleX  = DEFAULT_VIEW_ANGLE_X;
float viewAngleZ  = DEFAULT_VIEW_ANGLE_Z;
float targetX     = DEFAULT_TARGET_X;
float targetY     = DEFAULT_TARGET_Y;
float targetZ     = DEFAULT_TARGET_Z;
float eyeX, eyeY, eyeZ;

// Rastreamento do estado do mouse
int mouseLastX = 0;
int mouseLastY = 0;
int mouseActiveButton = -1;

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

// Reseta a camera e enquadramento para os valores originais
void resetCamera(void) {
    eyeDistance = DEFAULT_EYE_DISTANCE;
    viewAngleX  = DEFAULT_VIEW_ANGLE_X;
    viewAngleZ  = DEFAULT_VIEW_ANGLE_Z;
    targetX     = DEFAULT_TARGET_X;
    targetY     = DEFAULT_TARGET_Y;
    targetZ     = DEFAULT_TARGET_Z;
    glutPostRedisplay();
}

void handleKeypress(unsigned char key, int /*x*/, int /*y*/) {
    float radX = viewAngleX * (float)PI / 180.0f;
    switch (key) {
    case 27: // Tecla ESC
        exit(0);
    // Reset de camera
    case 'r':
    case 'R':
        resetCamera();
        break;
    // Zoom no teclado (+ / -)
    case '+':
    case '=':
        if (eyeDistance > 3.0f) eyeDistance -= 1.0f;
        glutPostRedisplay();
        break;
    case '-':
    case '_':
        if (eyeDistance < 60.0f) eyeDistance += 1.0f;
        glutPostRedisplay();
        break;
    // Pan no teclado (I, K, J, L)
    case 'i':
    case 'I':
        targetZ += 0.5f;
        glutPostRedisplay();
        break;
    case 'k':
    case 'K':
        targetZ -= 0.5f;
        glutPostRedisplay();
        break;
    case 'j':
    case 'J':
        targetX += (float)(-sin(radX) * 0.5f);
        targetY += (float)( cos(radX) * 0.5f);
        glutPostRedisplay();
        break;
    case 'l':
    case 'L':
        targetX -= (float)(-sin(radX) * 0.5f);
        targetY -= (float)( cos(radX) * 0.5f);
        glutPostRedisplay();
        break;
    // Compatibilidade com garra.cpp (w, s, a, d)
    case 'w':
        if (viewAngleZ < 85.0f) viewAngleZ += 3.0f;
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
    case GLUT_KEY_UP:    // Eleva a visao
        if (viewAngleZ < 85.0f) viewAngleZ += 3.0f;
        glutPostRedisplay();
        break;
    case GLUT_KEY_DOWN:  // Abaixa a visao
        if (viewAngleZ > -85.0f) viewAngleZ -= 3.0f;
        glutPostRedisplay();
        break;
    }
}

// Captura cliques do mouse e roda de scroll (FreeGLUT: button 3=up, 4=down)
void handleMouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        if (button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) {
            mouseActiveButton = button;
            mouseLastX = x;
            mouseLastY = y;
        } else if (button == 3) { // Scroll Up (Zoom In)
            if (eyeDistance > 3.0f) eyeDistance -= 1.0f;
            glutPostRedisplay();
        } else if (button == 4) { // Scroll Down (Zoom Out)
            if (eyeDistance < 60.0f) eyeDistance += 1.0f;
            glutPostRedisplay();
        }
    } else if (state == GLUT_UP) {
        if (button == mouseActiveButton) {
            mouseActiveButton = -1;
        }
    }
}

// Movimento do mouse arrastado (Botao Esquerdo = Orbita, Botao Direito = Pan)
void handleMouseMotion(int x, int y) {
    int deltaX = x - mouseLastX;
    int deltaY = y - mouseLastY;
    mouseLastX = x;
    mouseLastY = y;

    if (mouseActiveButton == GLUT_LEFT_BUTTON) {
        // Orbita esferica interativa
        viewAngleX += (float)deltaX * 0.4f;
        viewAngleZ += (float)deltaY * 0.4f;
        if (viewAngleZ > 85.0f)  viewAngleZ = 85.0f;
        if (viewAngleZ < -85.0f) viewAngleZ = -85.0f;
        glutPostRedisplay();
    } else if (mouseActiveButton == GLUT_RIGHT_BUTTON) {
        // Pan no plano de visualizacao relativo a camera (direcao invertida a pedido do usuario)
        float radX = viewAngleX * (float)PI / 180.0f;
        float radZ = viewAngleZ * (float)PI / 180.0f;
        float panFactor = eyeDistance * 0.003f;

        // Direcao invertida para arrasto mais intuitivo
        targetX -= (float)(-sin(radX) * deltaX) * panFactor;
        targetY -= (float)( cos(radX) * deltaX) * panFactor;

        targetZ -= (float)(deltaY) * panFactor;
        targetX -= (float)(cos(radX) * sin(radZ) * deltaY) * panFactor * 0.5f;
        targetY -= (float)(sin(radX) * sin(radZ) * deltaY) * panFactor * 0.5f;

        glutPostRedisplay();
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

    eyeX = targetX + eyeDistance * (float)(cos(viewAngleZ * PI / 180.0) * cos(viewAngleX * PI / 180.0));
    eyeY = targetY + eyeDistance * (float)(cos(viewAngleZ * PI / 180.0) * sin(viewAngleX * PI / 180.0));
    eyeZ = targetZ + eyeDistance * (float)(sin(viewAngleZ * PI / 180.0));

    gluLookAt(eyeX, eyeY, eyeZ, targetX, targetY, targetZ, 0.0, 0.0, 1.0);

    // Grade de chao para referencia espacial
    drawReferenceGrid();

    // Raiz do modelo (Placeholder do Torax para validacao)
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
    glutMouseFunc(handleMouse);
    glutMotionFunc(handleMouseMotion);
    glutReshapeFunc(handleResize);

    glutMainLoop();
    return 0;
}
