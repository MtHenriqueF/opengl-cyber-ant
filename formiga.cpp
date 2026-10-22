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
#include <string.h>
#include <math.h>
#include <GL/glut.h>
#include "RgbImage.h"

using namespace std;

// Caminho relativo para textura padrao e texturas especializadas de _temp_textures
const char* filenameTexMetal1   = "metalTexture1.bmp";
const char* filenameCarbonFiber = "_temp_textures/carbon_fiber1.bmp";
const char* filenameCorrugated  = "_temp_textures/corrugated_iron.bmp";
const char* filenameGrayMetal   = "_temp_textures/gray_metal.bmp";
const char* filenamePerforated  = "_temp_textures/perforated_metal.bmp";

// Identificadores de textura multi-materiais (Fase 5)
GLuint _texCarbonFiber   = 0; // 1. Carapaca Principal (Cabeca, Torax e Abdomen/Gaster)
GLuint _texCorrugated    = 0; // 2. Cintura (Peciolo e Pos-peciolo) e Articulacoes das Pernas
GLuint _texGrayMetal     = 0; // 3. Segmentos das Pernas e Antenas / Mandibulas / Ferrao
GLuint _texPerforated    = 0; // 4. Olhos Compostos

// Identificadores herdados de garra.cpp e quadricas GLU
GLuint _textureIdMetal1   = 0;
GLuint _textureIdSphere   = 0;
GLuint _textureIdCylinder = 0;
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

// Articulacoes das 6 Patas (Fase 3: Coxa -> Femur -> Tibia)
float legManualCoxa  = 0.0f;  // Articulacao horizontal Yaw das patas (-20 a +20 graus)
float legManualFemur = 0.0f;  // Articulacao vertical Pitch do femur (-20 a +20 graus)
float legManualTibia = 0.0f;  // Articulacao vertical Pitch da tibia (-20 a +20 graus)

// Parametros da Marcha Tripod Gait (Fase 4: Coordenadas e Cinematica 3D)
bool isWalking             = false;   // Estado da caminhada automatica (Espaco liga/desliga)
float walkCycle            = 0.0f;    // Variavel temporal do ciclo senoidal da marcha
const float WALK_SPEED     = 0.09f;   // Frequencia angular do passo (~60 FPS)
const float WALK_AMP_COXA  = 16.0f;   // Amplitude horizontal Yaw de avanco/recuo da coxa (graus)
const float WALK_AMP_FEMUR = 14.0f;   // Amplitude vertical Pitch de elevacao do joelho (graus)
const float WALK_AMP_TIBIA = 6.0f;    // Amplitude de flexao compensatoria da tibia no ar (graus)

// Rastreamento do estado do mouse
int mouseLastX = 0;
int mouseLastY = 0;
int mouseActiveButton = -1;

// Carrega uma imagem BMP 24-bits via RgbImage e registra como textura OpenGL com filtragem mipmap
GLuint loadTexture(const char* filename) {
    GLuint textureId = 0;
    RgbImage theTexMap(filename);

    // Tenta fallback com prefixo _temp_textures/ caso seja passado caminho relativo direto
    if (theTexMap.GetErrorCode() != 0 || theTexMap.GetNumCols() <= 0) {
        char altPath[256];
        snprintf(altPath, sizeof(altPath), "_temp_textures/%s", filename);
        theTexMap.LoadBmpFile(altPath);
    }

    if (theTexMap.GetErrorCode() != 0 || theTexMap.GetNumCols() <= 0 || theTexMap.GetNumRows() <= 0) {
        return 0;
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    gluBuild2DMipmaps(GL_TEXTURE_2D,
        GL_RGB,
        theTexMap.GetNumCols(),
        theTexMap.GetNumRows(),
        GL_RGB,
        GL_UNSIGNED_BYTE,
        theTexMap.ImageData());

    return textureId;
}

// Aplica a textura especifica e a cor base com suporte a modulacao e iluminacao
void applyTexture(GLuint texId, float r, float g, float b) {
    if (textureOn && texId != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texId);
        _textureIdCylinder = texId;
        _textureIdSphere   = texId;
    } else {
        glDisable(GL_TEXTURE_2D);
    }
    glColor3f(r, g, b);
}

// Inicializa recursos de renderizacao, iluminacao, materiais e texturas
void initRendering(void) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.06f, 0.07f, 0.09f, 1.0f);

    // Modo de ambiente de textura: GL_MODULATE para interacao entre iluminacao e imagem BMP
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Configuracao de Materiais Metalicos Especulares (aspecto cibernetico rigido e equilibrado)
    GLfloat matSpecular[]  = { 0.70f, 0.70f, 0.75f, 1.0f }; // Reflectancia especular metalica equilibrada
    GLfloat matShininess[] = { 78.0f };                   // Brilho especular elegante (na faixa de 70 a 90)
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShininess);

    // Vincula glColor3f as propriedades ambiente e difusa do material
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Visualizador local: calcula os vetores de reflexao especular exatos para cada vertice em relacao a camera
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    // Separacao da cor especular: adiciona o brilho metalico brilhante sobre a textura BMP
    #ifdef GL_LIGHT_MODEL_COLOR_CONTROL
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
    #endif

    // Configuracao de Iluminacao Suavizada de 2 Pontos (Key Light e Fill Light)
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // Luz 0: Principal (Key Light) - Incidencia moderada, evitando estourar as texturas
    GLfloat light0Ambient[]  = { 0.18f, 0.18f, 0.20f, 1.0f };
    GLfloat light0Diffuse[]  = { 0.80f, 0.80f, 0.85f, 1.0f };
    GLfloat light0Specular[] = { 0.75f, 0.75f, 0.80f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light0Ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0Diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);

    // Luz 1: Preenchimento Suave (Fill Light) - Clareia sombras sem ofuscar
    GLfloat light1Diffuse[]  = { 0.30f, 0.30f, 0.35f, 1.0f };
    GLfloat light1Specular[] = { 0.30f, 0.30f, 0.35f, 1.0f };
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light1Diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1Specular);

    quadSphere = gluNewQuadric();
    gluQuadricNormals(quadSphere, GLU_SMOOTH);

    quadCylinder = gluNewQuadric();
    gluQuadricNormals(quadCylinder, GLU_SMOOTH);

    // Carregamento das texturas multi-materiais da pasta _temp_textures
    _texCarbonFiber = loadTexture(filenameCarbonFiber);
    _texCorrugated  = loadTexture(filenameCorrugated);
    _texGrayMetal   = loadTexture(filenameGrayMetal);
    _texPerforated  = loadTexture(filenamePerforated);

    // Fallback caso alguma textura nao seja encontrada
    _textureIdMetal1 = loadTexture(filenameTexMetal1);
    if (!_texCarbonFiber) _texCarbonFiber = _textureIdMetal1;
    if (!_texCorrugated)  _texCorrugated  = _texCarbonFiber;
    if (!_texGrayMetal)   _texGrayMetal   = _texCarbonFiber;
    if (!_texPerforated)  _texPerforated  = _texCarbonFiber;

    _textureIdCylinder = _texCarbonFiber;
    _textureIdSphere   = _texCarbonFiber;
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

// Grade de referencia no plano de apoio das patas (Z = -1.65f) para orientacao espacial
void drawReferenceGrid(void) {
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.25f, 0.30f, 0.35f);
    glBegin(GL_LINES);
    for (int i = -15; i <= 15; i += 2) {
        glVertex3f((float)i, -15.0f, -1.65f);
        glVertex3f((float)i,  15.0f, -1.65f);
        glVertex3f(-15.0f, (float)i, -1.65f);
        glVertex3f( 15.0f, (float)i, -1.65f);
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
    legManualCoxa  = 0.0f;
    legManualFemur = 0.0f;
    legManualTibia = 0.0f;
    isWalking      = false;
    walkCycle      = 0.0f;
    glutPostRedisplay();
}

// Desenha uma mandibula articulada (pinca horizontal anatomica como a foto de referencia)
void drawMandible(bool isRight) {
    // 3. Mandibulas: gray_metal.bmp | Tom claro/cromado
    applyTexture(_texGrayMetal, 0.9f, 0.9f, 0.95f);

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
    // 3. Antenas: gray_metal.bmp | Tom claro/cromado
    applyTexture(_texGrayMetal, 0.9f, 0.9f, 0.95f);

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

        // 1. Carapaca Principal: Cranio e junta do pescoco (Fibra de Carbono escura)
        applyTexture(_texCarbonFiber, 0.2f, 0.2f, 0.22f);

        // Junta esferica do pescoco
        drawSphere(0.35f);

        // Cranio principal da cabeca cibernetica
        glPushMatrix();
            glTranslatef(0.0f, 0.75f, 0.0f);
            glScalef(0.85f, 1.05f, 0.75f);
            drawSphere(0.70f);
        glPopMatrix();

        // 4. Olhos: Textura perforated_metal.bmp (tom contrastante escuro)
        applyTexture(_texPerforated, 0.25f, 0.25f, 0.30f);

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

        // Mandibulas em pinca articuladas e salientes (gray_metal.bmp)
        drawMandible(true);
        drawMandible(false);

        // Antenas articuladas proporcionais (gray_metal.bmp)
        drawAntenna(true);
        drawAntenna(false);
    glPopMatrix();
}

// Desenha o torax central segmentado (Protorax, Mesotorax e Metatorax - raiz da hierarquia)
void drawThorax(void) {
    // 1. Carapaca Principal: Torax com textura carbon_fiber1.bmp e tom escuro
    applyTexture(_texCarbonFiber, 0.2f, 0.2f, 0.22f);

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

        // 2. Cintura (Peciolo e Pos-peciolo): corrugated_iron.bmp | Tom prata/cinza medio
        applyTexture(_texCorrugated, 0.8f, 0.8f, 0.85f);

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

        // 1. Carapaca Principal: Gaster (Abdomen volumoso): carbon_fiber1.bmp | Tom escuro
        applyTexture(_texCarbonFiber, 0.2f, 0.2f, 0.22f);

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

        // 3. Ferrao do Gaster: gray_metal.bmp | Tom claro/cromado
        applyTexture(_texGrayMetal, 0.9f, 0.9f, 0.95f);

        glPushMatrix();
            glTranslatef(0.0f, -1.55f, -0.08f);
            drawSphere(0.12f); // Base do ferrao
            // +95 graus em X aponta o cone do eixo +Z para -Y (para tras e levemente para baixo)
            glRotatef(95.0f, 1.0f, 0.0f, 0.0f);
            drawCone(0.10f, 0.55f);
        glPopMatrix();
    glPopMatrix();
}

// Desenha uma pata articulada anatomica (Coxa -> Femur -> Tibia -> Garra)
void drawLeg(int side, int legIdx, float coxaYaw, float femurPitch, float tibiaPitch) {
    glPushMatrix();
        float s = (float)side; // +1.0f para Direita, -1.0f para Esquerda

        // Parametros anatomicos dos 3 pares de patas (Dianteira, Media, Traseira)
        float attachX   = 0.0f;
        float attachY   = 0.0f;
        float attachZ   = -0.18f;
        float baseYaw   = 0.0f;
        float femurElev = 0.0f;
        float femurLen  = 0.0f;
        float tibiaElev = 0.0f;
        float tibiaLen  = 0.0f;

        if (legIdx == 0) {
            // Pata Dianteira (Protorax: aponta para frente e para fora)
            attachX   = s * 0.54f;
            attachY   = 0.80f;
            baseYaw   = 38.0f;
            femurElev = 48.0f;
            femurLen  = 1.60f;
            tibiaElev = -65.0f;
            tibiaLen  = 2.87f;
        } else if (legIdx == 1) {
            // Pata Media (Mesotorax: aponta lateralmente/levemente para tras)
            attachX   = s * 0.46f;
            attachY   = -0.05f;
            baseYaw   = -10.0f;
            femurElev = 52.0f;
            femurLen  = 1.80f;
            tibiaElev = -65.0f;
            tibiaLen  = 3.12f;
        } else {
            // Pata Traseira (Metatorax: a mais longa, aponta bem para tras)
            attachX   = s * 0.48f;
            attachY   = -0.85f;
            baseYaw   = -55.0f;
            femurElev = 42.0f;
            femurLen  = 2.15f;
            tibiaElev = -58.0f;
            tibiaLen  = 3.36f;
        }

        // 1. Referencial da Base / Quadril (ponto de encaixe lateral no Torax)
        glTranslatef(attachX, attachY, attachZ);
        glRotatef(s * (baseYaw + coxaYaw), 0.0f, 0.0f, 1.0f);

        // 2. Articulacao do Quadril: corrugated_iron.bmp | Tom prata/cinza medio
        applyTexture(_texCorrugated, 0.8f, 0.8f, 0.85f);
        drawSphere(0.10f);

        // --- ELO 1 DO BRACO MECANICO: REFERENCIAL DA COXA ---
        // 3. Segmento da Coxa: gray_metal.bmp | Tom claro/cromado
        applyTexture(_texGrayMetal, 0.9f, 0.9f, 0.95f);
        float coxaElev = -10.0f;
        float coxaLen  = 0.35f;
        glRotatef(s * (90.0f - coxaElev), 0.0f, 1.0f, 0.0f);
        drawCylinder(0.085f, coxaLen);

        // --- ELO 2 DO BRACO MECANICO: REFERENCIAL DO FEMUR (TROCANTER) ---
        // Translada puramente ao longo do eixo +Z local da coxa (padrao garra.cpp)
        glTranslatef(0.0f, 0.0f, coxaLen);

        // 2. Articulacao do Trocanter: corrugated_iron.bmp | Tom prata/cinza medio
        applyTexture(_texCorrugated, 0.8f, 0.8f, 0.85f);
        drawSphere(0.095f);

        // 3. Segmento do Femur: gray_metal.bmp | Tom claro/cromado
        applyTexture(_texGrayMetal, 0.9f, 0.9f, 0.95f);
        glRotatef(s * ((coxaElev - femurElev) - femurPitch), 0.0f, 1.0f, 0.0f);
        drawCylinder(0.075f, femurLen);

        // --- ELO 3 DO BRACO MECANICO: REFERENCIAL DA TIBIA (JOELHO) ---
        // Translada puramente ao longo do eixo +Z local do femur ate o joelho
        glTranslatef(0.0f, 0.0f, femurLen);

        // 2. Articulacao do Joelho: corrugated_iron.bmp | Tom prata/cinza medio
        applyTexture(_texCorrugated, 0.8f, 0.8f, 0.85f);
        drawSphere(0.085f);

        // 3. Segmento da Tibia e Garra: gray_metal.bmp | Tom claro/cromado
        applyTexture(_texGrayMetal, 0.9f, 0.9f, 0.95f);
        glRotatef(s * ((femurElev - tibiaElev) - tibiaPitch), 0.0f, 1.0f, 0.0f);

        float mainTibia = tibiaLen * 0.72f;
        float clawLen   = tibiaLen * 0.28f;
        drawCylinder(0.055f, mainTibia);

        // --- ELO 4 DO BRACO MECANICO: REFERENCIAL DA GARRA (TARSO) ---
        // Translada puramente ao longo do eixo +Z local da tibia ate a ponta da garra
        glTranslatef(0.0f, 0.0f, mainTibia);
        drawCone(0.055f, clawLen);
    glPopMatrix();
}

// Desenha os 3 pares de patas articuladas (6 patas no total) com cinematica de Tripod Gait
void drawAllLegs(void) {
    for (int legIdx = 0; legIdx < 3; legIdx++) {
        // Itera para ambos os lados: Direita (side = +1) e Esquerda (side = -1)
        for (int sIdx = 0; sIdx < 2; sIdx++) {
            int side = (sIdx == 0) ? 1 : -1;
            float walkCoxa  = 0.0f;
            float walkFemur = 0.0f;
            float walkTibia = 0.0f;

            if (isWalking) {
                // Tripod Gait (Marcha em Tripe de 6 patas):
                // Tripe A: Dianteira Esquerda (side=-1, leg=0), Media Direita (side=+1, leg=1), Traseira Esquerda (side=-1, leg=2)
                // Tripe B: Dianteira Direita (side=+1, leg=0), Media Esquerda (side=-1, leg=1), Traseira Direita (side=+1, leg=2)
                bool isTripodA = ((side == -1 && legIdx != 1) || (side == 1 && legIdx == 1));

                float phase = isTripodA ? walkCycle : (walkCycle + (float)PI);
                float sinPhase = (float)sin(phase);
                float cosPhase = (float)cos(phase);

                // 1. Articulacao Yaw da Coxa (Avanco / Recuo):
                // Durante a fase de balanco no ar (sinPhase > 0), a pata viaja para frente (-cos vai de -1 a +1).
                // Durante a fase de apoio no solo (sinPhase <= 0), a pata empurra o chao para tras (-cos vai de +1 a -1).
                walkCoxa = -WALK_AMP_COXA * cosPhase;

                // 2. Articulacao Pitch do Femur (Elevacao do Joelho):
                // No ar (sinPhase > 0): eleva o joelho para descolar a garra do solo.
                // No solo (sinPhase <= 0): garra apoia firmemente no plano de referencia Z = -1.65f.
                if (sinPhase > 0.0f) {
                    walkFemur = WALK_AMP_FEMUR * sinPhase;
                    // 3. Flexao compensatoria da tibia no ar para afastar a ponta do chao
                    walkTibia = -WALK_AMP_TIBIA * sinPhase;
                }
            }

            // Composicao cinematica: angulos dinamicos da marcha + articulacao manual
            float finalCoxa  = legManualCoxa + walkCoxa;
            float finalFemur = legManualFemur + walkFemur;
            float finalTibia = legManualTibia + walkTibia;

            drawLeg(side, legIdx, finalCoxa, finalFemur, finalTibia);
        }
    }
}

// Salva uma captura de tela do framebuffer em formato BMP 24-bits sem compressao
void saveScreenshot(const char* filename, int width, int height) {
    int rowPadding = (4 - (width * 3) % 4) % 4;
    int rowSize = width * 3 + rowPadding;
    int dataSize = rowSize * height;
    int fileSize = 54 + dataSize;

    unsigned char* pixels = (unsigned char*)malloc(width * height * 3);
    if (!pixels) return;
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    FILE* f = fopen(filename, "wb");
    if (f) {
        unsigned char fileHeader[14] = {
            'B', 'M',
            (unsigned char)(fileSize), (unsigned char)(fileSize >> 8), (unsigned char)(fileSize >> 16), (unsigned char)(fileSize >> 24),
            0, 0, 0, 0,
            54, 0, 0, 0
        };
        unsigned char infoHeader[40] = {
            40, 0, 0, 0,
            (unsigned char)(width), (unsigned char)(width >> 8), (unsigned char)(width >> 16), (unsigned char)(width >> 24),
            (unsigned char)(height), (unsigned char)(height >> 8), (unsigned char)(height >> 16), (unsigned char)(height >> 24),
            1, 0,
            24, 0,
            0, 0, 0, 0,
            (unsigned char)(dataSize), (unsigned char)(dataSize >> 8), (unsigned char)(dataSize >> 16), (unsigned char)(dataSize >> 24),
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        };
        fwrite(fileHeader, 1, 14, f);
        fwrite(infoHeader, 1, 40, f);

        unsigned char pad[3] = {0, 0, 0};
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 3;
                unsigned char r = pixels[idx];
                unsigned char g = pixels[idx + 1];
                unsigned char b = pixels[idx + 2];
                fputc(b, f);
                fputc(g, f);
                fputc(r, f);
            }
            if (rowPadding > 0) {
                fwrite(pad, 1, rowPadding, f);
            }
        }
        fclose(f);
        printf("Captura de tela salva em: %s\n", filename);
    }
    free(pixels);
}

void handleKeypress(unsigned char key, int /*x*/, int /*y*/) {
    switch (key) {
    case 27: // Tecla ESC
        exit(0);
    // Captura de tela para documentacao (P / p)
    case 'p':
    case 'P':
        saveScreenshot("docs/screenshot_formiga.bmp", 900, 700);
        break;
    // Modo de Caminhada (Tripod Gait): Liga/Desliga marcha automatica (Espaco)
    case ' ':
        isWalking = !isWalking;
        glutPostRedisplay();
        break;
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
    // Articulacao manual das 6 Patas (Fase 3: teclas 1/2, 3/4, 5/6)
    case '1': // Articula a coxa para frente (Yaw)
        if (legManualCoxa < 20.0f) legManualCoxa += 2.0f;
        glutPostRedisplay();
        break;
    case '2': // Articula a coxa para tras (Yaw)
        if (legManualCoxa > -20.0f) legManualCoxa -= 2.0f;
        glutPostRedisplay();
        break;
    case '3': // Eleva os femures (Pitch)
        if (legManualFemur < 20.0f) legManualFemur += 2.0f;
        glutPostRedisplay();
        break;
    case '4': // Abaixa os femures (Pitch)
        if (legManualFemur > -20.0f) legManualFemur -= 2.0f;
        glutPostRedisplay();
        break;
    case '5': // Flexiona as tibias
        if (legManualTibia < 20.0f) legManualTibia += 2.0f;
        glutPostRedisplay();
        break;
    case '6': // Estende as tibias
        if (legManualTibia > -20.0f) legManualTibia -= 2.0f;
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

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    eyeX = targetX + eyeDistance * (float)(cos(viewAngleZ * PI / 180.0) * cos(viewAngleX * PI / 180.0));
    eyeY = targetY + eyeDistance * (float)(cos(viewAngleZ * PI / 180.0) * sin(viewAngleX * PI / 180.0));
    eyeZ = targetZ + eyeDistance * (float)(sin(viewAngleZ * PI / 180.0));

    gluLookAt(eyeX, eyeY, eyeZ, targetX, targetY, targetZ, 0.0, 0.0, 1.0);

    // Posicionamento espacial das 2 fontes de luz no referencial do mundo (fixas na cena)
    GLfloat light0Pos[] = { 10.0f, -8.0f, 14.0f, 1.0f }; // Luz Principal (frontal direita alta)
    GLfloat light1Pos[] = { -10.0f, 8.0f,  4.0f, 1.0f }; // Luz de Preenchimento Suave (traseira esquerda)
    glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
    glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);

    // Grade de chao (desenhada sem iluminacao e sem textura para contraste puro)
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    drawReferenceGrid();

    // Habilita Iluminacao para o robo
    glEnable(GL_LIGHTING);

    // Garante modo GL_MODULATE ativo para interacao entre iluminacao e imagem BMP
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Habilita GL_TEXTURE_2D antes de desenhar caso texturas estejam ativas
    if (textureOn) {
        glEnable(GL_TEXTURE_2D);
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    // Desenho hierarquico da Formiga Robo com materiais especializados
    // 1. Torax central segmentado (raiz da hierarquia) - Carapaca Fibra de Carbono
    drawThorax();

    // 2. Cabeca articulada e apendices (filha anterior do Torax)
    drawHead();

    // 3. Peciolo, Abdomen (Gaster) e Ferrao (filhos posteriores do Torax)
    drawAbdomen();

    // 4. Seis patas articuladas anatomicas (3 de cada lado conectadas ao Torax)
    drawAllLegs();

    // Desabilita GL_TEXTURE_2D apos o termino do desenho
    glDisable(GL_TEXTURE_2D);

    glutSwapBuffers();
}

// Callback de temporizador para animacao fluida da marcha (~60 FPS)
void updateAnimation(int value) {
    (void)value;
    if (isWalking) {
        walkCycle += WALK_SPEED;
        if (walkCycle > 2.0f * (float)PI) {
            walkCycle -= 2.0f * (float)PI;
        }
        glutPostRedisplay();
    }
    glutTimerFunc(16, updateAnimation, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(900, 700);
    glutCreateWindow("Simulador Formiga Robo 3D - Cyber Ant");

    initRendering();
    if (argc > 1 && strcmp(argv[1], "--screenshot") == 0) {
        handleResize(900, 700);
        drawScene();
        const char* path = (argc > 2) ? argv[2] : "docs/screenshot_formiga.bmp";
        saveScreenshot(path, 900, 700);
        return 0;
    }

    glutDisplayFunc(drawScene);
    glutKeyboardFunc(handleKeypress);
    glutSpecialFunc(handleSpecialKeypress);
    glutMouseFunc(handleMouse);
    glutMotionFunc(handleMouseMotion);
    glutReshapeFunc(handleResize);
    glutTimerFunc(16, updateAnimation, 0);

    glutMainLoop();
    return 0;
}
