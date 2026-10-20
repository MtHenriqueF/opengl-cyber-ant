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

// Articulacoes calibradas da Cabeca, Apendices e Abdomen (Fase 2)
float headPitch     = 0.0f;   // Inclinacao vertical Pitch da cabeca (-20 a +20 graus)
float headYaw       = 0.0f;   // Rotacao horizontal Yaw da cabeca (-35 a +35 graus)
float mandibleAngle = 12.0f;  // Abertura das mandibulas (0 fechada/alinhada a 28 graus aberta)
float antennaAngle  = 10.0f;  // Articulacao das antenas (-10 a +25 graus)
float abdomenPitch  = 0.0f;   // Inclinacao vertical do abdomen/gaster (-25 a +25 graus)
float abdomenYaw    = 0.0f;   // Movimento lateral do abdomen/gaster (-25 a +25 graus)

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
    headPitch     = 0.0f;
    headYaw       = 0.0f;
    mandibleAngle = 12.0f;
    antennaAngle  = 10.0f;
    abdomenPitch  = 0.0f;
    abdomenYaw    = 0.0f;
    glutPostRedisplay();
}

// Desenha uma mandibula articulada (pinca horizontal anatomica como a foto de referencia)
void drawMandible(bool isRight) {
    glPushMatrix();
        float side = isRight ? 1.0f : -1.0f;
        // Posicionamento ancorado no focinho/bochecha frontal da cabeca (sem flutuar)
        glTranslatef(side * 0.26f, 1.18f, -0.18f);

        // 1. Articulacao horizontal no plano do chao: abre (j) e fecha (J)
        glRotatef(side * -mandibleAngle, 0.0f, 0.0f, 1.0f);

        // 2. Projeta as mandibulas PARA FRENTE (horizontal)
        // -90 graus em X projeta o cilindro na direcao +Y (frente da formiga)
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(4.0f, 1.0f, 0.0f, 0.0f); // Leve declive suave
        glRotatef(side * 10.0f, 0.0f, 1.0f, 0.0f); // Leve convergencia natural para o centro

        // Haste principal robusta saindo de dentro da cabeca
        drawCylinder(0.085f, 0.50f);
        glTranslatef(0.0f, 0.0f, 0.50f);
        drawSphere(0.085f);

        // Ponta/dente em cone curvado PARA DENTRO (-75 graus) formando a pinca
        glRotatef(side * -75.0f, 0.0f, 1.0f, 0.0f);
        drawCone(0.085f, 0.34f);
    glPopMatrix();
}

// Desenha uma antena bi-articulada proporcional e projetada para frente
void drawAntenna(bool isRight) {
    glPushMatrix();
        float side = isRight ? 1.0f : -1.0f;
        // Posicionamento na fronte superior da cabeca
        glTranslatef(side * 0.20f, 1.20f, 0.35f);

        // Articulacao interativa (u / U)
        glRotatef(antennaAngle, 1.0f, 0.0f, 0.0f);

        // Orientacao anatomica: abre para FORA (-32 graus em Z) e projeta para FRENTE (-62 em X)
        glRotatef(side * -32.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-62.0f, 1.0f, 0.0f, 0.0f);

        // Haste 1: Escapo (base que aponta para frente/cima/fora)
        drawCylinder(0.032f, 0.48f);
        glTranslatef(0.0f, 0.0f, 0.48f);

        // Junta articulada esferica (cotovelo da antena)
        drawSphere(0.042f);

        // Haste 2: Flagelo (curvado para frente e para baixo)
        glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(side * -15.0f, 0.0f, 1.0f, 0.0f);
        drawCone(0.032f, 0.52f);
    glPopMatrix();
}

// Desenha a cabeca articulada completa (cranio, olhos, mandibulas e antenas)
void drawHead(void) {
    glPushMatrix();
        // Conexao com o torax (pescoco anterior)
        glTranslatef(0.0f, 1.6f, 0.15f);

        // Cinematica corrigida: primeiro Yaw (giro horizontal do pescoco), depois Pitch (inclinacao vertical da cabeca)
        glRotatef(headYaw,   0.0f, 0.0f, 1.0f);
        glRotatef(headPitch, 1.0f, 0.0f, 0.0f);

        // Junta esferica do pescoco
        drawSphere(0.35f);

        // Cranio principal da cabeca cibernetica
        glPushMatrix();
            glTranslatef(0.0f, 0.75f, 0.0f);
            glScalef(0.85f, 1.05f, 0.75f);
            drawSphere(0.70f);
        glPopMatrix();

        // Olho Direito (+X)
        glPushMatrix();
            glTranslatef(0.55f, 0.85f, 0.28f);
            glScalef(1.0f, 1.2f, 0.9f);
            drawSphere(0.22f);
        glPopMatrix();

        // Olho Esquerdo (-X)
        glPushMatrix();
            glTranslatef(-0.55f, 0.85f, 0.28f);
            glScalef(1.0f, 1.2f, 0.9f);
            drawSphere(0.22f);
        glPopMatrix();

        // Mandibulas em pinca articuladas e salientes
        drawMandible(true);
        drawMandible(false);

        // Antenas articuladas proporcionais
        drawAntenna(true);
        drawAntenna(false);
    glPopMatrix();
}

// Desenha o torax central segmentado (Protorax, Mesotorax e Metatorax - raiz da hierarquia)
void drawThorax(void) {
    glPushMatrix();
        // Segmento 1: Protorax (anterior, mais robusto proximo ao pescoco)
        glPushMatrix();
            glTranslatef(0.0f, 0.85f, 0.08f);
            glScalef(0.62f, 0.82f, 0.56f);
            drawSphere(1.0f);
        glPopMatrix();

        // Segmento 2: Mesotorax (central, cintura toracica esguia)
        glPushMatrix();
            glTranslatef(0.0f, -0.05f, 0.0f);
            glScalef(0.50f, 0.72f, 0.46f);
            drawSphere(1.0f);
        glPopMatrix();

        // Segmento 3: Metatorax (posterior, conecta com o peciolo)
        glPushMatrix();
            glTranslatef(0.0f, -0.90f, -0.04f);
            glScalef(0.54f, 0.65f, 0.50f);
            drawSphere(1.0f);
        glPopMatrix();
    glPopMatrix();
}

// Desenha a cadeia posterior articulada: Peciolo + Pos-peciolo + Abdomen (Gaster) + Ferrao
void drawAbdomen(void) {
    glPushMatrix();
        // Conexao na extremidade traseira do Metatorax (-Y)
        glTranslatef(0.0f, -1.50f, -0.05f);

        // Articulacao da cintura (b/B para Pitch vertical, n/N para Yaw lateral)
        glRotatef(abdomenYaw,   0.0f, 0.0f, 1.0f);
        glRotatef(abdomenPitch, 1.0f, 0.0f, 0.0f);

        // 1. Peciolo (primeiro no da cintura)
        glPushMatrix();
            glScalef(0.28f, 0.35f, 0.28f);
            drawSphere(1.0f);
        glPopMatrix();

        // 2. Pos-peciolo (segundo no da cintura)
        glTranslatef(0.0f, -0.45f, 0.02f);
        glPushMatrix();
            glScalef(0.32f, 0.35f, 0.32f);
            drawSphere(1.0f);
        glPopMatrix();

        // 3. Gaster (Abdomen volumoso principal em formato de gota/oval)
        glTranslatef(0.0f, -1.45f, 0.05f);
        glRotatef(-8.0f, 1.0f, 0.0f, 0.0f); // Inclinacao natural do gaster

        // Bulbo principal do gaster
        glPushMatrix();
            glScalef(0.95f, 1.35f, 0.88f);
            drawSphere(1.0f);
        glPopMatrix();

        // Anel posterior afilado do gaster (formato anatomico pontiagudo)
        glPushMatrix();
            glTranslatef(0.0f, -0.90f, -0.04f);
            glScalef(0.68f, 0.78f, 0.62f);
            drawSphere(1.0f);
        glPopMatrix();

        // 4. Ferrao do Gaster (Stinger mecanico cônico na extremidade traseira)
        glPushMatrix();
            glTranslatef(0.0f, -1.55f, -0.08f);
            drawSphere(0.12f); // Base do ferrao
            // +95 graus em X aponta o cone do eixo +Z para -Y (para tras e levemente para baixo)
            glRotatef(95.0f, 1.0f, 0.0f, 0.0f);
            drawCone(0.10f, 0.55f);
        glPopMatrix();
    glPopMatrix();
}

void handleKeypress(unsigned char key, int /*x*/, int /*y*/) {
    switch (key) {
    case 27: // Tecla ESC
        exit(0);
    // Reset de camera e articulacoes
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
    // Articulacao calibrada da Cabeca: Pitch (h/H) e Yaw (y/Y)
    case 'h': // Inclina cabeca para cima
        if (headPitch < 20.0f) headPitch += 2.5f;
        glutPostRedisplay();
        break;
    case 'H': // Inclina cabeca para baixo
        if (headPitch > -20.0f) headPitch -= 2.5f;
        glutPostRedisplay();
        break;
    case 'y': // Gira cabeca para a esquerda
        if (headYaw < 35.0f) headYaw += 3.0f;
        glutPostRedisplay();
        break;
    case 'Y': // Gira cabeca para a direita
        if (headYaw > -35.0f) headYaw -= 3.0f;
        glutPostRedisplay();
        break;
    // Mandibulas: Abre/Fecha (j/J)
    case 'j': // Abre mandibulas (para fora)
        if (mandibleAngle < 28.0f) mandibleAngle += 2.5f;
        glutPostRedisplay();
        break;
    case 'J': // Fecha mandibulas (ate tocar, sem cruzar)
        if (mandibleAngle > 0.0f) mandibleAngle -= 2.5f;
        glutPostRedisplay();
        break;
    // Antenas: Articula (u/U)
    case 'u': // Eleva/articula antenas
        if (antennaAngle < 25.0f) antennaAngle += 2.5f;
        glutPostRedisplay();
        break;
    case 'U': // Abaixa antenas
        if (antennaAngle > -10.0f) antennaAngle -= 2.5f;
        glutPostRedisplay();
        break;
    // Abdomen / Peciolo: Pitch vertical (b/B) e Yaw lateral (n/N)
    case 'b': // Eleva/arqueia o abdomen
        if (abdomenPitch < 25.0f) abdomenPitch += 2.5f;
        glutPostRedisplay();
        break;
    case 'B': // Abaixa/curva o abdomen (movimento de ferroada)
        if (abdomenPitch > -25.0f) abdomenPitch -= 2.5f;
        glutPostRedisplay();
        break;
    case 'n': // Move o abdomen para um lado
        if (abdomenYaw < 25.0f) abdomenYaw += 2.5f;
        glutPostRedisplay();
        break;
    case 'N': // Move o abdomen para o outro lado
        if (abdomenYaw > -25.0f) abdomenYaw -= 2.5f;
        glutPostRedisplay();
        break;
    // Pan vertical no teclado (I / K)
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

    // Desenho hierarquico da Formiga Robo
    glColor3f(1.0f, 1.0f, 1.0f);

    // 1. Torax central segmentado (raiz da hierarquia)
    drawThorax();

    // 2. Cabeca articulada e apendices (filha anterior do Torax)
    drawHead();

    // 3. Peciolo, Abdomen (Gaster) e Ferrao (filhos posteriores do Torax)
    drawAbdomen();

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
