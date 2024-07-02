#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
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

struct Vector2 {
    float u, v;
};

struct Vector3 {
    float x, y, z;
};

std::ostream& operator<<(std::ostream& os, const Point3D& point) {
    os << "(" << point.x << ", " << point.y << ", " << point.z << ")";
    return os;
}

struct Color {
    float r, g, b;
};

struct Material {
    Color diffuse;
    Color ambient;
    Color specular;
    Color emissive;
    float shininess;
};

struct Light {
    std::string type;
    Vector3 position; 
    Vector3 direction;
    float cutoff;
    Color ambient;
    Color diffuse;
    Color specular;
};

struct Model {
    std::string name;
    std::vector<Point3D> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> texCoords;
    GLuint vboId;
    GLuint vaoId;
    Material material; 
    GLuint textureId;
};

struct Translate {
    bool active;
    bool align;
    float time;
    std::vector<Point3D> controlPoints;

    Translate() : active(true), align(false), time(0) {}
};

struct Rotate {
    bool active;
    float angle;
    Point3D axis;
    float time;

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
    std::vector<Light> lights;
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

void setupCamera(const Camera& camera) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(camera.projection.fov, static_cast<float>(worldConfig.window.width) / static_cast<float>(worldConfig.window.height), camera.projection.near, camera.projection.far);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camera.position.x, camera.position.y, camera.position.z,
        camera.lookAt.x, camera.lookAt.y, camera.lookAt.z,
        camera.up.x, camera.up.y, camera.up.z);
}

void initializeLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    float globalAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    for (size_t i = 0; i < worldConfig.lights.size(); i++) {
        Light& light = worldConfig.lights[i];
        int lightId = GL_LIGHT0 + i;
        glEnable(lightId);

        GLfloat ambient[] = { light.ambient.r, light.ambient.g, light.ambient.b, 1.0f };
        GLfloat diffuse[] = { light.diffuse.r, light.diffuse.g, light.diffuse.b, 1.0f };
        GLfloat specular[] = { light.specular.r, light.specular.g, light.specular.b, 1.0f };

        glLightfv(lightId, GL_AMBIENT, ambient);
        glLightfv(lightId, GL_DIFFUSE, diffuse);
        glLightfv(lightId, GL_SPECULAR, specular);

        if (light.type == "spot" || light.type == "directional") {
            GLfloat position[] = { light.position.x, light.position.y, light.position.z, 1.0f };
            glLightfv(lightId, GL_POSITION, position);

            if (light.type == "spot") {
                GLfloat direction[] = { light.direction.x, light.direction.y, light.direction.z, 0.0f };
                glLightfv(lightId, GL_SPOT_DIRECTION, direction);
                glLightf(lightId, GL_SPOT_CUTOFF, light.cutoff);
            }
        }
    }
}

void applyMaterial(const Material& material) {
    GLfloat ambient[] = { material.ambient.r, material.ambient.g, material.ambient.b, 1.0 };
    GLfloat diffuse[] = { material.diffuse.r, material.diffuse.g, material.diffuse.b, 1.0 };
    GLfloat specular[] = { material.specular.r, material.specular.g, material.specular.b, 1.0 };
    GLfloat emission[] = { material.emissive.r, material.emissive.g, material.emissive.b, 1.0 };
    GLfloat shininess[] = { material.shininess };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess[0]);
}

GLuint loadTexture(const std::string& filename) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, (nrChannels == 4 ? GL_RGBA : GL_RGB), width, height, 0, (nrChannels == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    std::cout << "Texture loaded successfully: " << filename << " (ID=" << textureID << ")" << std::endl;
    return textureID;
}

void renderModel(const Model& model) {
    glPushMatrix();
    glBindVertexArray(model.vaoId);

    glEnable(GL_LIGHTING);
    applyMaterial(model.material);

    if (model.textureId > 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, model.textureId);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, model.vboId);
    glVertexPointer(3, GL_FLOAT, 0, (void*)0);
    glNormalPointer(GL_FLOAT, 0, (void*)(model.vertices.size() * sizeof(Point3D)));
    glTexCoordPointer(2, GL_FLOAT, 0, (void*)((model.vertices.size() + model.normals.size()) * sizeof(Point3D)));

    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());
    glEnable(GL_CULL_FACE);

    if (model.textureId > 0) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindVertexArray(0);
    glPopMatrix();
}


void renderGroup(const Group& group) {
    glPushMatrix();

    if (group.transform.hasScale) {
        glScalef(group.transform.scale.x, group.transform.scale.y, group.transform.scale.z);
    }

    if (group.transform.rotate.active) {
        float angle = group.transform.rotate.angle;
        if (group.transform.rotate.time > 0) {
            float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
            angle = fmod(currentTime * (360.0f / group.transform.rotate.time), 360.0f);
        }
        glRotatef(angle, group.transform.rotate.axis.x, group.transform.rotate.axis.y, group.transform.rotate.axis.z);
    }

    if (group.transform.translate.active && !group.transform.translate.controlPoints.empty()) {
        drawCatmullRomCurve(group.transform.translate.controlPoints);

        if (group.transform.translate.time > 0 && group.transform.translate.align) {
            float t = fmod(glutGet(GLUT_ELAPSED_TIME) / 1000.0f, group.transform.translate.time) / group.transform.translate.time;
            Point3D pos;
            interpolateCatmullRom(group.transform.translate.controlPoints, t, pos);
            glTranslatef(pos.x, pos.y, pos.z);
        }
        else {
            glTranslatef(group.transform.translate.controlPoints[0].x,
                group.transform.translate.controlPoints[0].y,
                group.transform.translate.controlPoints[0].z);
        }
    }

    for (const Model& model : group.models) {
        renderModel(model);
    }

    for (const Group& child : group.children) {
        renderGroup(child);
    }

    glPopMatrix();
}

void drawAxes() {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_LINES);

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-5.0f, 0.0f, 0.0f);
    glVertex3f(5.0f, 0.0f, 0.0f);

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -5.0f, 0.0f);
    glVertex3f(0.0f, 5.0f, 0.0f);

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -5.0f);
    glVertex3f(0.0f, 0.0f, 5.0f);
    glEnd();
    glEnable(GL_LIGHTING); 
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
}

void drawNormals(const std::vector<Model>& models) {
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 1.0f, 0.0f);

    glBegin(GL_LINES);
    for (const auto& model : models) {
        for (size_t i = 0; i < model.vertices.size(); ++i) {
            Point3D vertex = model.vertices[i];
            Vector3 normal = model.normals[i];

            glVertex3f(vertex.x, vertex.y, vertex.z);
            glVertex3f(vertex.x + normal.x * 0.1f, vertex.y + normal.y * 0.1f, vertex.z + normal.z * 0.1f);
        }
    }
    glEnd();

    glEnable(GL_LIGHTING);
}

void drawGroupNormals(const Group& group) {
    drawNormals(group.models);

    for (const Group& child : group.children) {
        drawGroupNormals(child);
    }
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

    glEnable(GL_LIGHTING);
    drawAxes();

    for (const auto& group : worldConfig.groups) {
        renderGroup(group);
    }

    glutSwapBuffers();
}

void initializeVBO(Model& model) {
    glGenBuffers(1, &model.vboId);
    glBindBuffer(GL_ARRAY_BUFFER, model.vboId);

    size_t verticesSize = model.vertices.size() * sizeof(Point3D);
    size_t normalsSize = model.normals.size() * sizeof(Vector3);
    size_t texCoordsSize = model.texCoords.size() * sizeof(Vector2);

    glBufferData(GL_ARRAY_BUFFER, verticesSize + normalsSize + texCoordsSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, verticesSize, &model.vertices[0]);
    glBufferSubData(GL_ARRAY_BUFFER, verticesSize, normalsSize, &model.normals[0]);
    glBufferSubData(GL_ARRAY_BUFFER, verticesSize + normalsSize, texCoordsSize, &model.texCoords[0]);

    glGenVertexArrays(1, &model.vaoId);
    glBindVertexArray(model.vaoId);

    glBindBuffer(GL_ARRAY_BUFFER, model.vboId);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(verticesSize));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(verticesSize + normalsSize));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

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

    char comma, semicolon;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        Point3D vertex;
        Vector3 normal;
        Vector2 texCoord;

        if ((iss >> vertex.x >> comma >> vertex.y >> comma >> vertex.z >> semicolon
            >> normal.x >> comma >> normal.y >> comma >> normal.z >> semicolon
            >> texCoord.u >> comma >> texCoord.v) &&
            comma == ',' && semicolon == ';') {
            texCoord.v = 1.0f - texCoord.v;
            model.vertices.push_back(vertex);
            model.normals.push_back(normal);
            model.texCoords.push_back(texCoord);
        }
        else {
            std::cerr << "Erro ao ler a linha do arquivo: " << line << std::endl;
        }
    }

    file.close();
    if (model.vertices.size() != totalPoints) {
        std::cerr << "Aviso: o número total de pontos lidos não corresponde ao indicador inicial." << std::endl;
    }

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
            }
            else {

                for (tinyxml2::XMLElement* point = child->FirstChildElement("point"); point; point = point->NextSiblingElement("point")) {
                    Point3D p{
                        point->FloatAttribute("x"),
                        point->FloatAttribute("y"),
                        point->FloatAttribute("z")
                    };
                    transform.translate.controlPoints.push_back(p);
                }
            }
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

void parseColor(tinyxml2::XMLElement* colorElement, Model& model) {
    if (!colorElement) return;

    tinyxml2::XMLElement* diffuseElement = colorElement->FirstChildElement("diffuse");
    if (diffuseElement) {
        model.material.diffuse.r = diffuseElement->IntAttribute("R") / 255.0f;
        model.material.diffuse.g = diffuseElement->IntAttribute("G") / 255.0f;
        model.material.diffuse.b = diffuseElement->IntAttribute("B") / 255.0f;
        std::cout << "Diffuse: " << model.material.diffuse.r << ", " << model.material.diffuse.g << ", " << model.material.diffuse.b << std::endl;
    }

    tinyxml2::XMLElement* ambientElement = colorElement->FirstChildElement("ambient");
    if (ambientElement) {
        model.material.ambient.r = ambientElement->IntAttribute("R") / 255.0f;
        model.material.ambient.g = ambientElement->IntAttribute("G") / 255.0f;
        model.material.ambient.b = ambientElement->IntAttribute("B") / 255.0f;
        std::cout << "Ambient: " << model.material.ambient.r << ", " << model.material.ambient.g << ", " << model.material.ambient.b << std::endl;
    }

    tinyxml2::XMLElement* specularElement = colorElement->FirstChildElement("specular");
    if (specularElement) {
        model.material.specular.r = specularElement->IntAttribute("R") / 255.0f;
        model.material.specular.g = specularElement->IntAttribute("G") / 255.0f;
        model.material.specular.b = specularElement->IntAttribute("B") / 255.0f;
        std::cout << "Specular: " << model.material.specular.r << ", " << model.material.specular.g << ", " << model.material.specular.b << std::endl;
    }

    tinyxml2::XMLElement* emissiveElement = colorElement->FirstChildElement("emissive");
    if (emissiveElement) {
        model.material.emissive.r = emissiveElement->IntAttribute("R") / 255.0f;
        model.material.emissive.g = emissiveElement->IntAttribute("G") / 255.0f;
        model.material.emissive.b = emissiveElement->IntAttribute("B") / 255.0f;
        std::cout << "Emissive: " << model.material.emissive.r << ", " << model.material.emissive.g << ", " << model.material.emissive.b << std::endl;
    }

    tinyxml2::XMLElement* shininessElement = colorElement->FirstChildElement("shininess");
    if (shininessElement) {
        model.material.shininess = shininessElement->FloatAttribute("value");
        std::cout << "Shininess: " << model.material.shininess << std::endl;
    }
}

void parseLights(tinyxml2::XMLElement* lightsElement, World& world) {
    if (!lightsElement) {
        std::cerr << "Lights element not found." << std::endl;
        return;
    }
    for (tinyxml2::XMLElement* lightElement = lightsElement->FirstChildElement("light"); lightElement; lightElement = lightElement->NextSiblingElement("light")) {
        Light light;
        light.type = lightElement->Attribute("type");

        if (light.type == "point") {
            light.position.x = lightElement->FloatAttribute("posx", 0.0f);
            light.position.y = lightElement->FloatAttribute("posy", 0.0f);
            light.position.z = lightElement->FloatAttribute("posz", 0.0f);
        }
        else if (light.type == "directional") {
            light.direction.x = lightElement->FloatAttribute("dirx", 0.0f);
            light.direction.y = lightElement->FloatAttribute("diry", 0.0f);
            light.direction.z = lightElement->FloatAttribute("dirz", 0.0f);
        }
        else if (light.type == "spot") {
            light.position.x = lightElement->FloatAttribute("posx", 0.0f);
            light.position.y = lightElement->FloatAttribute("posy", 0.0f);
            light.position.z = lightElement->FloatAttribute("posz", 0.0f);
            light.direction.x = lightElement->FloatAttribute("dirx", 0.0f);
            light.direction.y = lightElement->FloatAttribute("diry", 0.0f);
            light.direction.z = lightElement->FloatAttribute("dirz", 0.0f);
            light.cutoff = lightElement->FloatAttribute("cutoff", 180.0f);
        }

        light.ambient.r = lightElement->FloatAttribute("ambientR", 0.2f);
        light.ambient.g = lightElement->FloatAttribute("ambientG", 0.2f);
        light.ambient.b = lightElement->FloatAttribute("ambientB", 0.2f);
        light.diffuse.r = lightElement->FloatAttribute("diffuseR", 0.8f);
        light.diffuse.g = lightElement->FloatAttribute("diffuseG", 0.8f);
        light.diffuse.b = lightElement->FloatAttribute("diffuseB", 0.8f);
        light.specular.r = lightElement->FloatAttribute("specularR", 1.0f);
        light.specular.g = lightElement->FloatAttribute("specularG", 1.0f);
        light.specular.b = lightElement->FloatAttribute("specularB", 1.0f);

        world.lights.push_back(light);
    }
}


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
            tinyxml2::XMLElement* colorElement = modelElement->FirstChildElement("color");
            parseColor(colorElement, model);

            tinyxml2::XMLElement* textureElement = modelElement->FirstChildElement("texture");
            if (textureElement) {
                const char* textureFile = textureElement->Attribute("file");
                if (textureFile) {
                    std::string texturePath = basePath + textureFile;
                    model.textureId = loadTexture(texturePath);
                }
            }

            group.models.push_back(model);
        }
    }
}

void parseGroup(tinyxml2::XMLElement* element, Group& group) {
    tinyxml2::XMLElement* transformElement = element->FirstChildElement("transform");
    if (transformElement) {
        parseTransform(transformElement, group.transform);
    }

    tinyxml2::XMLElement* modelsElement = element->FirstChildElement("models");
    if (modelsElement) {
        parseModels(modelsElement, group);
    }

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

    tinyxml2::XMLElement* windowElement = root->FirstChildElement("window");
    if (windowElement) {
        worldConfig.window.width = windowElement->IntAttribute("width");
        worldConfig.window.height = windowElement->IntAttribute("height");
    }

    tinyxml2::XMLElement* lightsElement = root->FirstChildElement("lights");
    if (lightsElement) {
        parseLights(lightsElement, worldConfig);
    }

    tinyxml2::XMLElement* cameraElement = root->FirstChildElement("camera");
    if (cameraElement) {
        parseCamera(cameraElement, worldConfig.camera);
    }

    tinyxml2::XMLElement* groupElement = root->FirstChildElement("group");
    while (groupElement) {
        Group group;
        parseGroup(groupElement, group);
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

    spherical2Cartesian();
    worldConfig.camera.position.x = camX;
    worldConfig.camera.position.y = camY;
    worldConfig.camera.position.z = camZ;

    glutPostRedisplay();
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
    worldConfig.camera.position.x = camX;
    worldConfig.camera.position.y = camY;
    worldConfig.camera.position.z = camZ;
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

    GLenum glewInitResult = glewInit();
    if (GLEW_OK != glewInitResult) {
        std::cerr << "ERROR: " << glewGetErrorString(glewInitResult) << std::endl;
        return EXIT_FAILURE;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(processKeys);
    glutSpecialFunc(processSpecialKeys);
    glutIdleFunc(myIdleFunc);

    parseXML("C:/Users/GIGABYTE/Desktop/teste/teste2/src/src/engine/xml_parte1.xml");
    initializeModels();
    initializeLighting();

    glutMainLoop();

    return 0;
}