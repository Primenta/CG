#include "tinyxml2.h"
#include <fstream>
#ifdef __APPLE__ 
#include <GLUT/glut.h>
#include <GL/gl.h>
#include <GLEW/glew.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

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
    GLuint vboId;
    GLuint vaoId; // ID do VAO (Vertex Array Object)
};

struct Translate {
    bool active;
    bool align;
    float time;
    std::vector<Point3D> controlPoints;  // pontos de controlo para a Catmull

    Translate() : active(true), align(false), time(0) {}
};

struct Rotate {
    bool active;
    float angle;
    Point3D axis;
    float time;  // duração da rotação em segundos

    Rotate() : active(true), angle(0), axis{ 0, 0, 0 }, time(0) {}
};

struct Transform {
    Point3D scale;
    Translate translate;
    Rotate rotate;
    bool hasScale = false;
};

struct Group {
    Transform transform;
    std::vector<Model> models;
    std::vector<Group> children;
};

struct World {
    Window window;
    Camera camera;
    std::vector<Group> groups;
};

World worldConfig;

float rotationX = 0.0f;
float rotationY = 0.0f;
float rotationZ = 0.0f;

float alfa = 0.0f, beta = 0.5f, radius = 100.0f;
float camX, camY, camZ;

void spherical2Cartesian() {

    camX = radius * cos(beta) * sin(alfa);
    camY = radius * sin(beta);
    camZ = radius * cos(beta) * cos(alfa);
}

float catmullRom(float t, float p0, float p1, float p2, float p3) {
    return 0.5 * ((2 * p1) + (-p0 + p2) * t + (2 * p0 - 5 * p1 + 4 * p2 - p3) * pow(t, 2) + (-p0 + 3 * p1 - 3 * p2 + p3) * pow(t, 3));
}

void interpolateCatmullRom(const std::vector<Point3D>& points, float t, Point3D& pos) {
    int numPoints = points.size();
    float totalT = t * numPoints;
    int p = int(totalT);
    float localT = totalT - p;

    int p0 = p - 1;
    int p1 = p;
    int p2 = p + 1;
    int p3 = p + 2;

    p0 = (p0 < 0) ? numPoints - 1 : p0;
    p1 = p1 % numPoints;
    p2 = p2 % numPoints;
    p3 = (p3 >= numPoints) ? p3 % numPoints : p3;

    pos.x = catmullRom(localT, points[p0].x, points[p1].x, points[p2].x, points[p3].x);
    pos.y = catmullRom(localT, points[p0].y, points[p1].y, points[p2].y, points[p3].y);
    pos.z = catmullRom(localT, points[p0].z, points[p1].z, points[p2].z, points[p3].z);
}

void drawCatmullRomCurve(const std::vector<Point3D>& points) {
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_STRIP);
    for (float t = 0; t <= 1.0; t += 0.01) {
        Point3D pos;
        interpolateCatmullRom(points, t, pos);
        glVertex3f(pos.x, pos.y, pos.z);
    }
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
}

void renderModel(const Model& model) {
    glBindVertexArray(model.vaoId);

    // Define o modo de renderização para wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Desenha os triângulos como wireframe
    glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);
}

void renderGroup(const Group& group) {

    glPushMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);

    // Aplicação da escala
    if (group.transform.hasScale) {
        glScalef(group.transform.scale.x, group.transform.scale.y, group.transform.scale.z);
    }

    // Aplicação de rotação ao grupo inteiro
    if (group.transform.rotate.active) {
        float angle = group.transform.rotate.angle; // Ângulo de rotação estático
        if (group.transform.rotate.time > 0) {
            float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // tempo em segundos
            angle = fmod(currentTime * (360.0f / group.transform.rotate.time), 360.0f); 
        }
        glRotatef(angle, group.transform.rotate.axis.x, group.transform.rotate.axis.y, group.transform.rotate.axis.z);
    }

    if (group.transform.translate.active && group.transform.rotate.active && !group.transform.translate.controlPoints.empty()) {
        // Aplica rotação
        if (group.transform.rotate.time > 0 && group.transform.rotate.time != 0) {
            float rotationDuration = group.transform.rotate.time;
            float angle = fmod((glutGet(GLUT_ELAPSED_TIME) * 360.0f / rotationDuration), 360.0f);
            glRotatef(angle, group.transform.rotate.axis.x, group.transform.rotate.axis.y, group.transform.rotate.axis.z);
        }

        else if (group.transform.rotate.time == 0 || group.transform.rotate.angle == 0) {
            glRotatef(group.transform.rotate.angle, group.transform.rotate.axis.x, group.transform.rotate.axis.y, group.transform.rotate.axis.z);
        }

        drawCatmullRomCurve(group.transform.translate.controlPoints);

        if (group.transform.translate.time > 0 && group.transform.translate.align) {

            // std::cerr << "Aplica as translaçoes " << std::endl; 
            float t = fmod(glutGet(GLUT_ELAPSED_TIME) / 1000.0f, group.transform.translate.time) / group.transform.translate.time;
            Point3D pos;
            interpolateCatmullRom(group.transform.translate.controlPoints, t, pos);
            glTranslatef(pos.x, pos.y, pos.z);
        }
        else {
            // std::cerr << "Nao aplica as translaçoes " << std::endl;
            // Translação estática
            glTranslatef(group.transform.translate.controlPoints[0].x,
                group.transform.translate.controlPoints[0].y,
                group.transform.translate.controlPoints[0].z);
        }
    }

    // else std::cerr << "Nao aplica trasnlaçao " << std::endl;

    // Renderização dos modelos do grupo
    for (const Model& model : group.models) {
        renderModel(model);
    }

    // Recursivamente processa os subgrupos
    for (const Group& child : group.children) {
        renderGroup(child);
    }

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

    drawAxes();

    // Renderiza os modelos
    for (const auto& group : worldConfig.groups) {
        renderGroup(group);
    }

    glutSwapBuffers();
}

void initializeVBO(Model& model) {
    glGenBuffers(1, &model.vboId);
    glBindBuffer(GL_ARRAY_BUFFER, model.vboId);
    glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(Point3D), &model.vertices[0], GL_STATIC_DRAW);

    glGenVertexArrays(1, &model.vaoId);
    glBindVertexArray(model.vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, model.vboId);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Funçao para ler o arquivo .3d
Model readModel(const std::string& filename) {
    std::ifstream file(filename);
    Model model;
    if (!file.is_open()) {
        std::cerr << "Não foi possível abrir o arquivo: " << filename << std::endl;
        return model;
    }

    std::string line;
    int totalPoints;
    if (std::getline(file, line)) {
        try {
            totalPoints = std::stoi(line);
        }
        catch (const std::exception& e) {
            std::cerr << "Erro ao converter o número total de pontos: " << e.what() << std::endl;
            return model;
        }
    }

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        Point3D point;
        char comma; // Armazena a vírgula que está entre os valores
        if (iss >> point.x >> comma >> point.y >> comma >> point.z) {
            model.vertices.push_back(point);
        }
        else {
            std::cerr << "Erro ao ler as coordenadas do ponto: " << line << std::endl;
        }
    }

    file.close();
    if (model.vertices.size() != totalPoints) {
        std::cerr << "Aviso: o número total de pontos lidos não corresponde ao indicador inicial." << std::endl;
    }

    // Inicializa o VBO 
    initializeVBO(model);

    return model;
}


const std::string basePath = "C:/Users/GIGABYTE/Desktop/teste/teste2/src/src/generator/build/Release/";

void parseTransform(tinyxml2::XMLElement* element, Transform& transform) {

    for (tinyxml2::XMLElement* child = element->FirstChildElement(); child; child = child->NextSiblingElement()) {
        std::string type = child->Value();

        if (type == "translate") {
            transform.translate.active = true;
            transform.translate.time = child->FloatAttribute("time", 0);
            transform.translate.align = child->BoolAttribute("align", false);

            if (child->NoChildren()) {
                Point3D p{
                    child->FloatAttribute("x", 0.0f),
                    child->FloatAttribute("y", 0.0f),
                    child->FloatAttribute("z", 0.0f)
                };
                transform.translate.controlPoints.push_back(p);
                std::cerr << "Added static control point: " << p << std::endl;
            }
            else {

                for (tinyxml2::XMLElement* point = child->FirstChildElement("point"); point; point = point->NextSiblingElement("point")) {
                    Point3D p{
                        point->FloatAttribute("x"),
                        point->FloatAttribute("y"),
                        point->FloatAttribute("z")
                    };
                    transform.translate.controlPoints.push_back(p);
                    // std::cerr << "Pontos de controlo: " << p << std::endl;
                }
            }
            // std::cerr << "Total de pontos de controlo adicionados: " << transform.translate.controlPoints.size() << std::endl;
        }

        else if (type == "rotate") {
            transform.rotate.active = true;
            transform.rotate.time = child->FloatAttribute("time", 0.0f);
            transform.rotate.angle = child->FloatAttribute("angle", 0.0f);
            transform.rotate.axis.x = child->FloatAttribute("x", 0.0f);
            transform.rotate.axis.y = child->FloatAttribute("y", 0.0f);
            transform.rotate.axis.z = child->FloatAttribute("z", 0.0f);
        }
        else if (type == "scale") {
            transform.scale.x = child->FloatAttribute("x", 1.0f);
            transform.scale.y = child->FloatAttribute("y", 1.0f);
            transform.scale.z = child->FloatAttribute("z", 1.0f);
            transform.hasScale = true;
        }
    }
}

// Função para ler os atributos da câmera
void parseCamera(tinyxml2::XMLElement* cameraElement, Camera& camera) {
    if (!cameraElement) return;

    tinyxml2::XMLElement* posElement = cameraElement->FirstChildElement("position");
    if (posElement) {
        camera.position.x = posElement->FloatAttribute("x");
        camera.position.y = posElement->FloatAttribute("y");
        camera.position.z = posElement->FloatAttribute("z");

        radius = sqrt(camera.position.x * camera.position.x + camera.position.y * camera.position.y + camera.position.z * camera.position.z);
        alfa = atan2(camera.position.z, camera.position.x);
        beta = asin(camera.position.y / radius);
    }

    tinyxml2::XMLElement* lookAtElement = cameraElement->FirstChildElement("lookAt");
    if (lookAtElement) {
        camera.lookAt.x = lookAtElement->FloatAttribute("x");
        camera.lookAt.y = lookAtElement->FloatAttribute("y");
        camera.lookAt.z = lookAtElement->FloatAttribute("z");
    }

    tinyxml2::XMLElement* upElement = cameraElement->FirstChildElement("up");
    if (upElement) {
        camera.up.x = upElement->FloatAttribute("x");
        camera.up.y = upElement->FloatAttribute("y");
        camera.up.z = upElement->FloatAttribute("z");
    }

    tinyxml2::XMLElement* projElement = cameraElement->FirstChildElement("projection");
    if (projElement) {
        camera.projection.fov = projElement->FloatAttribute("fov");
        camera.projection.near = projElement->FloatAttribute("near");
        camera.projection.far = projElement->FloatAttribute("far");
    }
}

void parseModels(tinyxml2::XMLElement* modelsElement, Group& group) {
    if (!modelsElement) return;

    for (tinyxml2::XMLElement* modelElement = modelsElement->FirstChildElement("model");
        modelElement != nullptr;
        modelElement = modelElement->NextSiblingElement("model")) {

        std::string modelFile = modelElement->Attribute("file");
        if (!modelFile.empty()) {
            Model model = readModel(basePath + modelFile);
            group.models.push_back(model);
        }
    }
}

void parseGroup(tinyxml2::XMLElement* element, Group& group) {
    // Verifica se há um elemento de transformação
    tinyxml2::XMLElement* transformElement = element->FirstChildElement("transform");
    if (transformElement) {
        parseTransform(transformElement, group.transform);
    }

    // Processa os modelos contidos diretamente neste grupo
    tinyxml2::XMLElement* modelsElement = element->FirstChildElement("models");
    if (modelsElement) {
        parseModels(modelsElement, group);
    }

    // Processa subgrupos recursivamente
    for (tinyxml2::XMLElement* childElement = element->FirstChildElement("group");
        childElement != nullptr;
        childElement = childElement->NextSiblingElement("group")) {
        Group childGroup;
        parseGroup(childElement, childGroup);
        group.children.push_back(childGroup);
    }
}

void parseXML(const std::string& filename) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError result = doc.LoadFile(filename.c_str());
    if (result != tinyxml2::XML_SUCCESS) {
        std::cerr << "Erro ao abrir o arquivo XML: " << filename << std::endl;
        return;
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("world");
    if (!root) {
        std::cerr << "Tag <world> não encontrada." << std::endl;
        return;
    }

    // Parse window
    tinyxml2::XMLElement* windowElement = root->FirstChildElement("window");
    if (windowElement) {
        worldConfig.window.width = windowElement->IntAttribute("width");
        worldConfig.window.height = windowElement->IntAttribute("height");
    }

    // Parse camera
    tinyxml2::XMLElement* cameraElement = root->FirstChildElement("camera");
    if (cameraElement) {
        parseCamera(cameraElement, worldConfig.camera);
    }

    // Parse groups
    tinyxml2::XMLElement* groupElement = root->FirstChildElement("group");
    while (groupElement) {
        Group group;
        parseGroup(groupElement, group); // Inicia o processo sem uma transformação pai
        worldConfig.groups.push_back(group);
        groupElement = groupElement->NextSiblingElement("group");
    }
}

void processKeys(unsigned char c, int xx, int yy) {
    switch (c) {
    case 'q':
        exit(0);
    case '+':
        radius -= 1.0f;
        if (radius < 1.0f)
            radius = 1.0f;
        break;
    case '-':
        radius += 1.0f;
        break;
    }

    // Recalcula as coordenadas cartesianas da câmera
    spherical2Cartesian();
    // Atualiza a posição da câmera
    worldConfig.camera.position.x = camX;
    worldConfig.camera.position.y = camY;
    worldConfig.camera.position.z = camZ;

    glutPostRedisplay(); // Redesenha a cena com a nova posição da câmera
}


void processSpecialKeys(int key, int xx, int yy) {
    switch (key) {
    case GLUT_KEY_RIGHT:
        alfa -= 0.1;
        break;
    case GLUT_KEY_LEFT:
        alfa += 0.1;
        break;
    case GLUT_KEY_UP:
        beta += 0.1f;
        if (beta > 1.5f)
            beta = 1.5f;
        break;
    case GLUT_KEY_DOWN:
        beta -= 0.1f;
        if (beta < -1.5f)
            beta = -1.5f;
        break;
    case GLUT_KEY_PAGE_DOWN:
        radius -= 1.0f;
        if (radius < 1.0f)
            radius = 1.0f;
        break;
    case GLUT_KEY_PAGE_UP:
        radius += 1.0f;
        break;
    }
    spherical2Cartesian();
    glutPostRedisplay();
    // Atualiza a posição e a direção da câmera
    worldConfig.camera.position.x = camX;
    worldConfig.camera.position.y = camY;
    worldConfig.camera.position.z = camZ;
    glutPostRedisplay(); // Redesenha a cena com a nova posição da câmera
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

void initializeModels() {
    for (auto& group : worldConfig.groups) {
        for (auto& model : group.models) {
            initializeVBO(model);
        }
    }
}

void myIdleFunc() {
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(worldConfig.window.width, worldConfig.window.height);
    glutCreateWindow("Engine Application");

    // Configuração de GLEW
    GLenum glewInitResult = glewInit();
    if (GLEW_OK != glewInitResult) {
        std::cerr << "ERROR: " << glewGetErrorString(glewInitResult) << std::endl;
        return EXIT_FAILURE;
    }

    // Remove a parte de trás das figuras
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Configurações do GLUT
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(processKeys);
    glutSpecialFunc(processSpecialKeys);

    // Configuração da função de idle
    glutIdleFunc(myIdleFunc);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    parseXML("C:/Users/GIGABYTE/Desktop/teste/teste2/src/src/engine/xml_parte1.xml");
    initializeModels();

    glutMainLoop();

    return 0;
}