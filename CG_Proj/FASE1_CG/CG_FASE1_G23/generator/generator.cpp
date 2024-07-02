#define _USE_MATH_DEFINES 
#include <math.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct Vector3 {
    float x, y, z;
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
};

Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
    return Vector3(a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t);
}

void writeToFile(const std::string& filename, const std::vector<Vector3>& vertices) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Não foi possível abrir o arquivo para escrita: " << filename << std::endl;
        return;
    }

    file << vertices.size() << std::endl; // Escreve o número total de pontos na primeira linha
    for (const auto& v : vertices) {
        file << v.x << "," << v.y << "," << v.z << std::endl;
    }
}

void generatePlane(float length, int divisions, const std::string& filename) {
    std::vector<Vector3> vertices;
    float half_length = length / 2.0f; // Metade do comprimento do plano
    float step = length / divisions;
    for (int i = 0; i < divisions; ++i) {
        for (int j = 0; j < divisions; ++j) {
            // Coordenadas dos vértices ajustadas para centralizar o plano em torno de (0, 0, 0)
            vertices.push_back(Vector3(i * step - half_length, j * step - half_length, 0));
            vertices.push_back(Vector3((i + 1) * step - half_length, j * step - half_length, 0));
            vertices.push_back(Vector3(i * step - half_length, (j + 1) * step - half_length, 0));

            vertices.push_back(Vector3((i + 1) * step - half_length, j * step - half_length, 0));
            vertices.push_back(Vector3((i + 1) * step - half_length, (j + 1) * step - half_length, 0));
            vertices.push_back(Vector3(i * step - half_length, (j + 1) * step - half_length, 0));
        }
    }
    writeToFile(filename, vertices);
}

void generateBox(float length, int divisions, const std::string& filename) {
    std::vector<Vector3> vertices;
    float half_length = length / 2.0f; // Metade do comprimento da caixa para deixar centrado
    float step = length / divisions;
    for (int face = 0; face < 6; ++face) {
        for (int i = 0; i < divisions; ++i) {
            for (int j = 0; j < divisions; ++j) {
                Vector3 v0(0.0f, 0.0f, 0.0f), v1(0.0f, 0.0f, 0.0f), v2(0.0f, 0.0f, 0.0f), v3(0.0f, 0.0f, 0.0f);
                switch (face) {
                case 0: // Face de cima
                    v0 = Vector3(i * step - half_length, j * step - half_length, half_length);
                    v1 = Vector3((i + 1) * step - half_length, j * step - half_length, half_length);
                    v2 = Vector3((i + 1) * step - half_length, (j + 1) * step - half_length, half_length);
                    v3 = Vector3(i * step - half_length, (j + 1) * step - half_length, half_length);
                    break;
                case 1: // Face de baixo
                    v0 = Vector3(i * step - half_length, j * step - half_length, -half_length);
                    v1 = Vector3((i + 1) * step - half_length, j * step - half_length, -half_length);
                    v2 = Vector3((i + 1) * step - half_length, (j + 1) * step - half_length, -half_length);
                    v3 = Vector3(i * step - half_length, (j + 1) * step - half_length, -half_length);
                    break;
                case 2: // Face da frente
                    v0 = Vector3(i * step - half_length, -half_length, j * step - half_length);
                    v1 = Vector3((i + 1) * step - half_length, -half_length, j * step - half_length);
                    v2 = Vector3((i + 1) * step - half_length, -half_length, (j + 1) * step - half_length);
                    v3 = Vector3(i * step - half_length, -half_length, (j + 1) * step - half_length);
                    break;
                case 3: // Face de trás
                    v0 = Vector3(i * step - half_length, half_length, j * step - half_length);
                    v1 = Vector3((i + 1) * step - half_length, half_length, j * step - half_length);
                    v2 = Vector3((i + 1) * step - half_length, half_length, (j + 1) * step - half_length);
                    v3 = Vector3(i * step - half_length, half_length, (j + 1) * step - half_length);
                    break;
                case 4: // Face da esquerda
                    v0 = Vector3(-half_length, i * step - half_length, j * step - half_length);
                    v1 = Vector3(-half_length, (i + 1) * step - half_length, j * step - half_length);
                    v2 = Vector3(-half_length, (i + 1) * step - half_length, (j + 1) * step - half_length);
                    v3 = Vector3(-half_length, i * step - half_length, (j + 1) * step - half_length);
                    break;
                case 5: // Face da direita
                    v0 = Vector3(half_length, i * step - half_length, j * step - half_length);
                    v1 = Vector3(half_length, (i + 1) * step - half_length, j * step - half_length);
                    v2 = Vector3(half_length, (i + 1) * step - half_length, (j + 1) * step - half_length);
                    v3 = Vector3(half_length, i * step - half_length, (j + 1) * step - half_length);
                    break;
                }

                // Adiciona dois triângulos para formar um quadrado em cada parte da divisão como vemos na imagem
                vertices.push_back(v0);
                vertices.push_back(v1);
                vertices.push_back(v2);

                vertices.push_back(v2);
                vertices.push_back(v3);
                vertices.push_back(v0);
            }
        }
    }
    writeToFile(filename, vertices);
}


void generateSphere(float radius, int slices, int stacks, const std::string& filename) {
    std::vector<Vector3> vertices;
    for (int i = 0; i < stacks; ++i) {
        float phi1 = M_PI * (float)i / stacks - M_PI / 2;
        float phi2 = M_PI * (float)(i + 1) / stacks - M_PI / 2;
        for (int j = 0; j < slices; ++j) {
            float theta1 = 2 * M_PI * (float)j / slices;
            float theta2 = 2 * M_PI * (float)(j + 1) / slices;

            Vector3 v0 = Vector3(radius * cos(phi1) * cos(theta1), radius * cos(phi1) * sin(theta1), radius * sin(phi1));
            Vector3 v1 = Vector3(radius * cos(phi1) * cos(theta2), radius * cos(phi1) * sin(theta2), radius * sin(phi1));
            Vector3 v2 = Vector3(radius * cos(phi2) * cos(theta2), radius * cos(phi2) * sin(theta2), radius * sin(phi2));
            Vector3 v3 = Vector3(radius * cos(phi2) * cos(theta1), radius * cos(phi2) * sin(theta1), radius * sin(phi2));

            vertices.push_back(v0);
            vertices.push_back(v1);
            vertices.push_back(v2);

            vertices.push_back(v2);
            vertices.push_back(v3);
            vertices.push_back(v0);
        }
    }
    writeToFile(filename, vertices);
}

float theta(int slice, int slices) {
    return 2.0f * M_PI * slice / slices;
}

void generateCone(float radius, float height, int slices, int stacks, const std::string& filename) {
    std::vector<Vector3> vertices;
    float stack_height = height / stacks;
    float angle = (2 * M_PI) / slices;

    for (int i = 0; i < slices; ++i) {
        // Base
        float angle1 = i * angle;
        float angle2 = (i + 1) * angle;
        Vector3 v0(0.0f, 0.0f, 0.0f);
        Vector3 v1(radius * cos(angle1), radius * sin(angle1), 0.0f);
        Vector3 v2(radius * cos(angle2), radius * sin(angle2), 0.0f);
        vertices.push_back(v0);
        vertices.push_back(v1);
        vertices.push_back(v2);
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            float angle1 = j * angle;
            float angle2 = (j + 1) * angle;

            // Coordenadas dos vértices das stacks
            float r0 = radius * (stacks - i) / stacks;
            float r1 = radius * (stacks - (i + 1)) / stacks;
            Vector3 v0(r0 * cos(angle1), r0 * sin(angle1), i * stack_height);
            Vector3 v1(r0 * cos(angle2), r0 * sin(angle2), i * stack_height);
            Vector3 v2(r1 * cos(angle1), r1 * sin(angle1), (i + 1) * stack_height);
            Vector3 v3(r1 * cos(angle2), r1 * sin(angle2), (i + 1) * stack_height);

            // Primeiro triângulo
            vertices.push_back(v0);
            vertices.push_back(v1);
            vertices.push_back(v2);

            // Segundo triângulo
            vertices.push_back(v2);
            vertices.push_back(v1);
            vertices.push_back(v3);
        }
    }

    // Escrever no arquivo
    writeToFile(filename, vertices);
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [sphere|box|cone] <parameters> filename\n";
        return 1;
    }

    std::string shapeType = argv[1];
    std::vector<Vector3> points;
    std::string filename;

    if (shapeType == "sphere" && argc == 6) {
        double radius = std::stod(argv[2]);
        int slices = std::stoi(argv[3]);
        int stacks = std::stoi(argv[4]);
        filename = argv[5];
        generateSphere(radius, slices, stacks, filename);
    }
    else if (shapeType == "box" && argc == 5) {
        double length = std::stod(argv[2]);
        int grid = std::stoi(argv[3]);
        filename = argv[4];
        generateBox(length, grid, filename);
    }
    else if (shapeType == "plane" && argc == 5) {
        float length = std::stof(argv[2]);
        int divisions = std::stoi(argv[3]);
        filename = argv[4];
        generatePlane(length, divisions, filename);
    }
    else if (shapeType == "cone" && argc == 7) {
        double radius = std::stod(argv[2]);
        double height = std::stod(argv[3]);
        int slices = std::stoi(argv[4]);
        int stacks = std::stoi(argv[5]);
        filename = argv[6];
        generateCone(radius, height, slices, stacks, filename);
    }
    else {
        std::cerr << "Invalid parameters for " << shapeType << "\n";
        return 1;
    }

    return 0;
}