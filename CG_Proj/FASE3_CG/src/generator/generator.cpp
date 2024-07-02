#define _USE_MATH_DEFINES 
#include <math.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <sstream>
#include <iterator>
#include <algorithm>


struct Vector3 {
    float x, y, z;
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {} // Construtor padrão inicializado com valores padrão
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
};


struct Patches {
    std::vector<std::vector<int>> indices;
    std::vector<Vector3> controlPoints;
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

    file << vertices.size() << "\n";
    for (const auto& v : vertices) {
        file << v.x << "," << v.y << "," << v.z << "\n";
    }

    file.close();
    std::cout << "Written " << vertices.size() << " vertices to " << filename << std::endl;
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

// PATCHES

std::vector<std::vector<std::vector<float>>> readPatchesFile(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    std::vector<std::vector<std::vector<float>>> result;
    if (file) {
        char buffer[2048];
        // Obtenção do número de patches
        if (!fgets(buffer, 2047, file)) return result;
        int numPatches = atoi(buffer);
        std::vector<std::vector<int>> indicesPerPatch; // vector de vectores, cada vector tem tamanho 16

        // Obtenção dos índices de cada patch
        for (int i = 0; i < numPatches; i++) {
            if (!fgets(buffer, 2047, file)) return result;
            std::vector<int> indices;
            for (char* token = strtok(buffer, ","); token; token = strtok(NULL, ",")) {
                indices.push_back(atoi(token));
            }
            indicesPerPatch.push_back(indices);
        }

        // Obtenção do número de pontos de controlo
        if (!fgets(buffer, 2047, file)) return result;
        int numControlPoints = atoi(buffer);

        // Obtenção dos pontos de controlo
        std::vector< std::vector<float>> controlPoints;
        for (int i = 0; i < numControlPoints; i++) {
            if (!fgets(buffer, 2047, file)) return result;
            std::vector<float> point;
            for (char* token = strtok(buffer, ","); token; token = strtok(NULL, ",")) {
                point.push_back(atof(token));
            }
            controlPoints.push_back(point);
        }

        // Construção dos patches
        for (std::vector<int> indices : indicesPerPatch) {
            std::vector< std::vector<float>> patch; // um patch é um conjunto de pontos, um ponto é um vector<float> de tamanho 3. Um patch terá 16 pontos.
            for (int indice : indices) {
                std::vector<float> point;
                point.push_back(controlPoints[indice][0]);
                point.push_back(controlPoints[indice][1]);
                point.push_back(controlPoints[indice][2]);
                patch.push_back(point);
            }
            result.push_back(patch);
        }
        fclose(file);
    }

    return result;
}

void multiplyMatrices(int la, int ca, const float* A, // matriz A, dimensões la x ca
    int lb, int cb, const float* B, // matriz B, dimensões lb x cb
    float* R, int* lr = NULL, int* cr = NULL) { // matriz R, dimensões ca x lb
    if (ca == lb) {
        if (lr) *lr = ca; // Se for NULL, então é porque não se pretende guardar as dimensões da matriz resultado
        if (cr) *cr = lb; // Se for NULL, então é porque não se pretende guardar as dimensões da matriz resultado
        for (int i = 0; i < la; i++) {
            for (int j = 0; j < cb; j++) {
                R[i * cb + j] = 0;
                for (int k = 0; k < ca; k++) {
                    R[i * cb + j] += A[i * ca + k] * B[k * cb + j];
                }
            }
        }
    }
}

void surfacePoint(float u, float v, std::vector<std::vector<float>> patch, float* res) {
    float M[16] = { -1.0f,  3.0f, -3.0f, 1.0f,
                   3.0f, -6.0f,  3.0f, 0.0f,
                  -3.0f,  3.0f,  0.0f, 0.0f,
                   1.0f,  0.0f,  0.0f, 0.0f }; // 4x4
    float U[4] = { u * u * u,u * u,u,1.0f }, V[4] = { v * v * v,v * v,v,1.0f }; // U: 1x4; V: 4x1
    float UM[4]; multiplyMatrices(1, 4, U, 4, 4, M, UM); // UM: 1x4
    float MV[4]; multiplyMatrices(4, 4, M, 4, 1, V, MV); // MV: 4x1
    float P[3][16] = { {patch[0][0],patch[1][0],patch[2][0],patch[3][0],
                       patch[4][0],patch[5][0],patch[6][0],patch[7][0],
                       patch[8][0],patch[9][0],patch[10][0],patch[11][0],
                       patch[12][0],patch[13][0],patch[14][0],patch[15][0]},
                      {patch[0][1],patch[1][1],patch[2][1],patch[3][1],
                       patch[4][1],patch[5][1],patch[6][1],patch[7][1],
                       patch[8][1],patch[9][1],patch[10][1],patch[11][1],
                       patch[12][1],patch[13][1],patch[14][1],patch[15][1]},
                      {patch[0][2],patch[1][2],patch[2][2],patch[3][2],
                       patch[4][2],patch[5][2],patch[6][2],patch[7][2],
                       patch[8][2],patch[9][2],patch[10][2],patch[11][2],
                       patch[12][2],patch[13][2],patch[14][2],patch[15][2]} }; // Três matrizes 4x4, uma para cada componente X, Y e Z
    for (int i = 0; i < 3; i++) {
        float UMP[4]; // UMP: 1x4
        multiplyMatrices(1, 4, UM, 4, 4, P[i], UMP);
        multiplyMatrices(1, 4, UMP, 4, 1, MV, &res[i]);
    }

}

void buildPatches(const char* ficheiroPath, int tessellation, const char* ficheiro_output) {
    FILE* file = fopen(ficheiro_output, "w");

    float u = 0.0f, v = 0.0f, delta = 1.0f / tessellation;
    float A[3], B[3], C[3], D[3];

    std::vector< std::vector< std::vector<float>>> patches = readPatchesFile(ficheiroPath);

    std::vector<Vector3> vertices;

    for (std::vector<std::vector<float>> patch : patches) { // um patch tem 16 pontos
        for (int i = 0; i < tessellation; i++, u += delta) {
            for (int j = 0; j < tessellation; j++, v += delta) {

                // Cálculo dos pontos
                Vector3 pointA, pointB, pointC, pointD;
                surfacePoint(u, v, patch, A);
                surfacePoint(u, v + delta, patch, B);
                surfacePoint(u + delta, v, patch, C);
                surfacePoint(u + delta, v + delta, patch, D);

                // Adicionar pontos aos vetores de vértices
                vertices.push_back(Vector3(C[0], C[1], C[2]));
                vertices.push_back(Vector3(A[0], A[1], A[2]));
                vertices.push_back(Vector3(B[0], B[1], B[2]));

                vertices.push_back(Vector3(B[0], B[1], B[2]));
                vertices.push_back(Vector3(D[0], D[1], D[2]));
                vertices.push_back(Vector3(C[0], C[1], C[2]));
            }
            v = 0.0f;
        }
        u = v = 0.0f;
    }

    std::vector<Vector3> vertexVectors;
    for (const auto& vertex : vertices) {
        vertexVectors.push_back(vertex);
    }
    writeToFile(ficheiro_output, vertexVectors);

    fclose(file);
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " [sphere|box|cone|plane|patch] <parameters> filename\n";
        return 1;
    }

    std::string shapeType = argv[1];
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
    else if (shapeType == "patch" && argc == 5) {
        const char* inputPatchFile = argv[2];
        int level = std::stoi(argv[3]);
        const char* filename = argv[4];
        buildPatches(inputPatchFile, level, filename);
    }
    else {
        std::cerr << "Invalid parameters or command\n";
        return 1;
    }

    return 0;
}