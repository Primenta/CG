#include "tinyxml2.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

struct Window {
    int width;
    int height;
};

struct Camera {
    struct Position {
        float x;
        float y;
        float z;
    } position;

    struct LookAt {
        float x;
        float y;
        float z;
    } lookAt;

    struct Up {
        float x;
        float y;
        float z;
    } up;

    struct Projection {
        float fov;
        float near;
        float far;
    } projection;
};

struct Point3D {
    float x, y, z;
};

std::ostream& operator<<(std::ostream& os, const Point3D& point) {
    os << "(" << point.x << ", " << point.y << ", " << point.z << ")";
    return os;
}

struct Model {
    std::string name;
    std::vector<Point3D> vertices;
};

struct Group {
    std::vector<Model> models;
};

struct World {
    Window window;
    Camera camera;
    Group group;
};

World worldConfig;

float rotationX = 0.0f;
float rotationY = 0.0f;
float rotationZ = 0.0f;

void renderModel(const Model& model) {
    // Salva o estado da matriz atual e aplicar transforma??es
    glPushMatrix();
    glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
    glRotatef(rotationZ, 0.0f, 0.0f, 1.0f);

    // Primeiro: Desenha o modelo s?lido com uma cor de fundo
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // N?o atualiza o buffer de cor
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_TRIANGLES);
    for (const Point3D& vertex : model.vertices) {
        glVertex3f(vertex.x, vertex.y, vertex.z);
    }
    glEnd();

    // Segundo: Desenha o modelo em wireframe
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Atualiza o buffer de cor
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1.0f, -1.0f); // Deslocamento para as linhas serem desenhadas por cima dos s?lidos
    glColor3f(1.0f, 1.0f, 1.0f); // Cor branca para o wireframe
    glBegin(GL_TRIANGLES);
    for (const Point3D& vertex : model.vertices) {
        glVertex3f(vertex.x, vertex.y, vertex.z);
    }
    glEnd();

    glDisable(GL_POLYGON_OFFSET_LINE);

    // Restaura o estado da matriz.
    glPopMatrix();
}

void drawAxes() {
    glBegin(GL_LINES);
    // Eixo X em vermelho
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-5.0f, 0.0f, 0.0f);
    glVertex3f(5.0f, 0.0f, 0.0f);
    // Eixo Y em verde
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -5.0f, 0.0f);
    glVertex3f(0.0f, 5.0f, 0.0f);
    // Eixo Z em azul
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -5.0f);
    glVertex3f(0.0f, 0.0f, 5.0f);
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(worldConfig.camera.projection.fov,
        (float)worldConfig.window.width / (float)worldConfig.window.height,
        worldConfig.camera.projection.near,
        worldConfig.camera.projection.far);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(worldConfig.camera.position.x, worldConfig.camera.position.y, worldConfig.camera.position.z,
        worldConfig.camera.lookAt.x, worldConfig.camera.lookAt.y, worldConfig.camera.lookAt.z,
        worldConfig.camera.up.x, worldConfig.camera.up.y, worldConfig.camera.up.z);

    drawAxes(); // Desenha os eixos

    // Renderiza os modelos
    for (const auto& model : worldConfig.group.models) {
        if (model.vertices.size() > 0) {
            renderModel(model);
        }
    }
    glutSwapBuffers();
}

void initialize() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glutPostRedisplay();
}

// Fun??o para ler o arquivo .3d
Model readModel(const std::string& filename) {
    std::ifstream file(filename);
    Model model;
    if (!file.is_open()) {
        std::cerr << "N?o foi poss?vel abrir o arquivo: " << filename << std::endl;
        return model;
    }

    std::string line;
    int totalPoints;
    if (std::getline(file, line)) {
        try {
            totalPoints = std::stoi(line);
        }
        catch (const std::exception& e) {
            std::cerr << "Erro ao converter o n?mero total de pontos: " << e.what() << std::endl;
            return model;
        }
    }
    else {
        std::cerr << "Erro ao ler o n?mero total de pontos" << std::endl;
        return model;
    }

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        Point3D point;
        char comma; // Armazena a virgula que esta entre os valores
        if (iss >> point.x >> comma >> point.y >> comma >> point.z) {
            model.vertices.push_back(point);
        }
        else {
            std::cerr << "Erro ao ler as coordenadas do ponto: " << line << std::endl;
        }
    }

    file.close();
    if (model.vertices.size() != totalPoints) {
        std::cerr << "Aviso: o n?mero total de pontos lidos n?o corresponde ao indicador inicial." << std::endl;
    }
    return model;
}

const std::string basePath = "C:/Users/marga/OneDrive/Ambiente de Trabalho/CG/src/generator/build/release/";

void parseXML(const std::string& filename) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError result = doc.LoadFile(filename.c_str());
    if (result != tinyxml2::XML_SUCCESS) {
        std::cerr << "Erro ao abrir o arquivo XML: " << filename << std::endl;
        return;
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("world");
    if (!root) {
        std::cerr << "Tag <world> n?o encontrada." << std::endl;
        return;
    }

    tinyxml2::XMLElement* windowElement = root->FirstChildElement("window");
    if (windowElement) {
        worldConfig.window.width = windowElement->IntAttribute("width");
        worldConfig.window.height = windowElement->IntAttribute("height");
    }

    tinyxml2::XMLElement* cameraElement = root->FirstChildElement("camera");
    if (cameraElement) {
        tinyxml2::XMLElement* posElement = cameraElement->FirstChildElement("position");
        if (posElement) {
            worldConfig.camera.position.x = posElement->FloatAttribute("x");
            worldConfig.camera.position.y = posElement->FloatAttribute("y");
            worldConfig.camera.position.z = posElement->FloatAttribute("z");
        }

        tinyxml2::XMLElement* lookAtElement = cameraElement->FirstChildElement("lookAt");
        if (lookAtElement) {
            worldConfig.camera.lookAt.x = lookAtElement->FloatAttribute("x");
            worldConfig.camera.lookAt.y = lookAtElement->FloatAttribute("y");
            worldConfig.camera.lookAt.z = lookAtElement->FloatAttribute("z");
        }

        tinyxml2::XMLElement* upElement = cameraElement->FirstChildElement("up");
        if (upElement) {
            worldConfig.camera.up.x = upElement->FloatAttribute("x");
            worldConfig.camera.up.y = upElement->FloatAttribute("y");
            worldConfig.camera.up.z = upElement->FloatAttribute("z");
        }

        tinyxml2::XMLElement* projElement = cameraElement->FirstChildElement("projection");
        if (projElement) {
            worldConfig.camera.projection.fov = projElement->FloatAttribute("fov");
            worldConfig.camera.projection.near = projElement->FloatAttribute("near");
            worldConfig.camera.projection.far = projElement->FloatAttribute("far");
        }
    }

    tinyxml2::XMLElement* groupElement = root->FirstChildElement("group");
    if (groupElement) {
        tinyxml2::XMLElement* modelsElement = groupElement->FirstChildElement("models");
        if (modelsElement) {
            for (tinyxml2::XMLElement* modelElement = modelsElement->FirstChildElement("model");
                modelElement != nullptr;
                modelElement = modelElement->NextSiblingElement("model")) {

                std::string modelFile = modelElement->Attribute("file");
                Model model = readModel(basePath + modelFile);
                worldConfig.group.models.push_back(model);
            }
        }
    }
}

void specialKeys(int key, int x, int y) {
    const float step = 0.1f; // Tamanho para mover o objeto
    const float rotationStep = 5.0f; // Graus de rota??o por passo

    switch (key) {
    case GLUT_KEY_LEFT:
        rotationY -= rotationStep; // Rota??o em torno do eixo Y
        break;
    case GLUT_KEY_RIGHT:
        rotationY += rotationStep; // Rota??o em torno do eixo Y
        break;
    case GLUT_KEY_UP:
        rotationX -= rotationStep; // Rota??o em torno do eixo X
        break;
    case GLUT_KEY_DOWN:
        rotationX += rotationStep; // Rota??o em torno do eixo X
        break;
    }

    glutPostRedisplay();
}

void reshape(int width, int height) {
    worldConfig.window.width = width;
    worldConfig.window.height = height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(worldConfig.camera.projection.fov,
        (float)width / (float)height,
        worldConfig.camera.projection.near,
        worldConfig.camera.projection.far);

    glViewport(0, 0, width, height);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(worldConfig.camera.position.x, worldConfig.camera.position.y, worldConfig.camera.position.z,
        worldConfig.camera.lookAt.x, worldConfig.camera.lookAt.y, worldConfig.camera.lookAt.z,
        worldConfig.camera.up.x, worldConfig.camera.up.y, worldConfig.camera.up.z);
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);

    parseXML("C:/Users/marga/OneDrive/Ambiente de Trabalho/CG/src/engine/xml_parte1.xml");

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(worldConfig.window.width, worldConfig.window.height);
    glutCreateWindow("Engine Application");

    initialize();

    glutReshapeFunc(reshape);

    glutDisplayFunc(display);
    glutSpecialFunc(specialKeys);

    glutMainLoop();

    return 0;
}




