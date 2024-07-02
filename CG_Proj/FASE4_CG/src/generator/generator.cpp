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
#include <iomanip>

struct Vector3 {
    float x, y, z;

    Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}

    Vector3 normalized() const {
        float length = std::sqrt(x * x + y * y + z * z);
        return Vector3(x / length, y / length, z / length);
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    Vector3 cross(const Vector3& other) const {
        return Vector3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
    }
};

struct Vector2 {
    float u, v;

    Vector2(float u = 0.0f, float v = 0.0f) : u(u), v(v) {}
};

Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
    return Vector3(a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t);
}

std::string formatNumber(float num) {
    if (std::floor(num) == num) {
        return std::to_string(int(num));
    }
    else {
        std::stringstream stream;
        stream << std::fixed << std::setprecision(4) << num;
        return stream.str();
    }
}

void writeToFile(const std::string& filename, const std::vector<Vector3>& vertices, const std::vector<Vector3>& normals, const std::vector<Vector2>& texCoords) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Não foi possível abrir o arquivo para escrita: " << filename << std::endl;
        return;
    }

    file << vertices.size() << "\n";
    for (size_t i = 0; i < vertices.size(); ++i) {
        file << formatNumber(vertices[i].x) << "," << formatNumber(vertices[i].y) << "," << formatNumber(vertices[i].z) << " ; "
            << formatNumber(normals[i].x) << "," << formatNumber(normals[i].y) << "," << formatNumber(normals[i].z) << " ; "
            << formatNumber(texCoords[i].u) << "," << formatNumber(1.0f - texCoords[i].v) << "\n";
    }

    file.close();
    std::cout << "Written " << vertices.size() << " vertex to archive " << filename << std::endl;
}

void generatePlane(float length, int divisions, const std::string& filename) {
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> texCoords;

    float half_length = length / 2.0f;
    float step = length / divisions;

    Vector3 normal(0.0f, 1.0f, 0.0f);

    for (int i = 0; i < divisions; ++i) {
        for (int j = 0; j < divisions; ++j) {
            float x0 = i * step - half_length;
            float x1 = (i + 1) * step - half_length;
            float z0 = j * step - half_length;
            float z1 = (j + 1) * step - half_length;

            float u0 = float(i) / divisions;
            float u1 = float(i + 1) / divisions;
            float v0 = float(j) / divisions;
            float v1 = float(j + 1) / divisions;

            vertices.push_back(Vector3(x0, 0.0f, z0));
            normals.push_back(normal);
            texCoords.push_back(Vector2(u0, v0));

            vertices.push_back(Vector3(x0, 0.0f, z1));
            normals.push_back(normal);
            texCoords.push_back(Vector2(u0, v1));

            vertices.push_back(Vector3(x1, 0.0f, z0));
            normals.push_back(normal);
            texCoords.push_back(Vector2(u1, v0));

            vertices.push_back(Vector3(x0, 0.0f, z1));
            normals.push_back(normal);
            texCoords.push_back(Vector2(u0, v1));

            vertices.push_back(Vector3(x1, 0.0f, z1));
            normals.push_back(normal);
            texCoords.push_back(Vector2(u1, v1));

            vertices.push_back(Vector3(x1, 0.0f, z0));
            normals.push_back(normal);
            texCoords.push_back(Vector2(u1, v0));
        }
    }

    writeToFile(filename, vertices, normals, texCoords);
}

void generateBox(float length, int divisions, const std::string& filename) {
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> texCoords;

    float half_length = length / 2.0f;
    float step = length / divisions;

    std::vector<Vector3> faceNormals = {
        Vector3(0, 1, 0),
        Vector3(0, -1, 0),
        Vector3(0, 0, 1),
        Vector3(0, 0, -1),
        Vector3(-1, 0, 0),
        Vector3(1, 0, 0)
    };

    for (int face = 0; face < 6; ++face) {
        for (int i = 0; i < divisions; ++i) {
            for (int j = 0; j < divisions; ++j) {
                Vector3 v0, v1, v2, v3;
                float u1f = i / static_cast<float>(divisions);
                float v1f = j / static_cast<float>(divisions);
                float u2f = (i + 1) / static_cast<float>(divisions);
                float v2f = (j + 1) / static_cast<float>(divisions);

                switch (face) {
                case 0:
                    v0 = Vector3(-half_length + i * step, half_length, -half_length + j * step);
                    v1 = Vector3(-half_length + (i + 1) * step, half_length, -half_length + j * step);
                    v2 = Vector3(-half_length + (i + 1) * step, half_length, -half_length + (j + 1) * step);
                    v3 = Vector3(-half_length + i * step, half_length, -half_length + (j + 1) * step);
                    break;
                case 1:
                    v0 = Vector3(-half_length + i * step, -half_length, -half_length + j * step);
                    v1 = Vector3(-half_length + (i + 1) * step, -half_length, -half_length + j * step);
                    v2 = Vector3(-half_length + (i + 1) * step, -half_length, -half_length + (j + 1) * step);
                    v3 = Vector3(-half_length + i * step, -half_length, -half_length + (j + 1) * step);
                    v1f = 1 - v1f;
                    v2f = 1 - v2f;
                    break;
                case 2:
                    v0 = Vector3(-half_length + i * step, -half_length + j * step, half_length);
                    v1 = Vector3(-half_length + (i + 1) * step, -half_length + j * step, half_length);
                    v2 = Vector3(-half_length + (i + 1) * step, -half_length + (j + 1) * step, half_length);
                    v3 = Vector3(-half_length + i * step, -half_length + (j + 1) * step, half_length);
                    break;
                case 3:
                    v0 = Vector3(-half_length + i * step, -half_length + j * step, -half_length);
                    v1 = Vector3(-half_length + (i + 1) * step, -half_length + j * step, -half_length);
                    v2 = Vector3(-half_length + (i + 1) * step, -half_length + (j + 1) * step, -half_length);
                    v3 = Vector3(-half_length + i * step, -half_length + (j + 1) * step, -half_length);
                    break;
                case 4:
                    v0 = Vector3(-half_length, -half_length + j * step, -half_length + i * step);
                    v1 = Vector3(-half_length, -half_length + (j + 1) * step, -half_length + i * step);
                    v2 = Vector3(-half_length, -half_length + (j + 1) * step, -half_length + (i + 1) * step);
                    v3 = Vector3(-half_length, -half_length + j * step, -half_length + (i + 1) * step);
                    break;
                case 5: 
                    v0 = Vector3(half_length, -half_length + j * step, -half_length + i * step);
                    v1 = Vector3(half_length, -half_length + (j + 1) * step, -half_length + i * step);
                    v2 = Vector3(half_length, -half_length + (j + 1) * step, -half_length + (i + 1) * step);
                    v3 = Vector3(half_length, -half_length + j * step, -half_length + (i + 1) * step);
                    break;
                }

                vertices.push_back(v0);
                vertices.push_back(v1);
                vertices.push_back(v2);
                vertices.push_back(v2);
                vertices.push_back(v3);
                vertices.push_back(v0);

                normals.push_back(faceNormals[face]);
                normals.push_back(faceNormals[face]);
                normals.push_back(faceNormals[face]);
                normals.push_back(faceNormals[face]);
                normals.push_back(faceNormals[face]);
                normals.push_back(faceNormals[face]);

                texCoords.push_back(Vector2(u1f, v1f));
                texCoords.push_back(Vector2(u2f, v1f));
                texCoords.push_back(Vector2(u2f, v2f));
                texCoords.push_back(Vector2(u2f, v2f));
                texCoords.push_back(Vector2(u1f, v2f));
                texCoords.push_back(Vector2(u1f, v1f));
            }
        }
    }

    writeToFile(filename, vertices, normals, texCoords);
}

float theta(int slice, int slices) {
    return 2.0f * M_PI * slice / slices;
}

void generateSphere(float radius, int slices, int stacks, const std::string& filename) {
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> texCoords;

    for (int i = 0; i <= stacks; ++i) {
        float phi = M_PI * i / stacks;
        float sin_phi = sin(phi);
        float cos_phi = cos(phi);

        for (int j = 0; j <= slices; ++j) {
            float theta = 2 * M_PI * j / slices;
            float sin_theta = sin(theta);
            float cos_theta = cos(theta);

            float x = radius * sin_phi * cos_theta;
            float y = radius * cos_phi;
            float z = radius * sin_phi * sin_theta;

            float u = static_cast<float>(j) / slices;
            float v = static_cast<float>(i) / stacks;

            Vector3 vertex(x, y, z);
            Vector3 normal = vertex.normalized();
            Vector2 texCoord(u, v);

            vertices.push_back(vertex);
            normals.push_back(normal);
            texCoords.push_back(texCoord);
        }
    }

    std::vector<Vector3> finalVertices;
    std::vector<Vector3> finalNormals;
    std::vector<Vector2> finalTexCoords;

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = (i * (slices + 1)) + j;
            int second = first + slices + 1;

            finalVertices.push_back(vertices[first]);
            finalNormals.push_back(normals[first]);
            finalTexCoords.push_back(texCoords[first]);

            finalVertices.push_back(vertices[second]);
            finalNormals.push_back(normals[second]);
            finalTexCoords.push_back(texCoords[second]);

            finalVertices.push_back(vertices[first + 1]);
            finalNormals.push_back(normals[first + 1]);
            finalTexCoords.push_back(texCoords[first + 1]);

            finalVertices.push_back(vertices[second]);
            finalNormals.push_back(normals[second]);
            finalTexCoords.push_back(texCoords[second]);

            finalVertices.push_back(vertices[second + 1]);
            finalNormals.push_back(normals[second + 1]);
            finalTexCoords.push_back(texCoords[second + 1]);

            finalVertices.push_back(vertices[first + 1]);
            finalNormals.push_back(normals[first + 1]);
            finalTexCoords.push_back(texCoords[first + 1]);
        }
    }

    writeToFile(filename, finalVertices, finalNormals, finalTexCoords);
}

void generateCone(float radius, float height, int slices, int stacks, const std::string& filename) {
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> texCoords;

    float stackHeight = height / stacks;
    float angleStep = (2 * M_PI) / slices;

    for (int i = 0; i < slices; ++i) {
        float angle = i * angleStep;
        float nextAngle = (i + 1) * angleStep;
        Vector3 v0(0, 0, 0);
        Vector3 v1(radius * cos(angle), 0, radius * sin(angle));
        Vector3 v2(radius * cos(nextAngle), 0, radius * sin(nextAngle));

        vertices.push_back(v0);
        vertices.push_back(v1);
        vertices.push_back(v2);

        normals.push_back(Vector3(0, -1, 0));
        normals.push_back(Vector3(0, -1, 0));
        normals.push_back(Vector3(0, -1, 0));

        texCoords.push_back(Vector2(0.5, 0.5));
        texCoords.push_back(Vector2(cos(angle) * 0.5 + 0.5, sin(angle) * 0.5 + 0.5));
        texCoords.push_back(Vector2(cos(nextAngle) * 0.5 + 0.5, sin(nextAngle) * 0.5 + 0.5));
    }

    for (int j = 0; j < stacks; ++j) {
        float currentRadius = radius * (1 - static_cast<float>(j) / stacks);
        float nextRadius = radius * (1 - static_cast<float>(j + 1) / stacks);
        float currentHeight = j * stackHeight;
        float nextHeight = (j + 1) * stackHeight;

        for (int i = 0; i < slices; ++i) {
            float angle = i * angleStep;
            float nextAngle = (i + 1) * angleStep;

            Vector3 v0(currentRadius * cos(angle), currentHeight, currentRadius * sin(angle));
            Vector3 v1(nextRadius * cos(angle), nextHeight, nextRadius * sin(angle));
            Vector3 v2(currentRadius * cos(nextAngle), currentHeight, currentRadius * sin(nextAngle));
            Vector3 v3(nextRadius * cos(nextAngle), nextHeight, nextRadius * sin(nextAngle));

            vertices.push_back(v0);
            vertices.push_back(v1);
            vertices.push_back(v2);

            vertices.push_back(v1);
            vertices.push_back(v3);
            vertices.push_back(v2);

            Vector3 normal = (v1 - v0).normalized().cross((v2 - v0).normalized()).normalized();
            normals.push_back(normal);
            normals.push_back(normal);
            normals.push_back(normal);
            normals.push_back(normal);
            normals.push_back(normal);
            normals.push_back(normal);

            texCoords.push_back(Vector2(static_cast<float>(i) / slices, static_cast<float>(j) / stacks));
            texCoords.push_back(Vector2(static_cast<float>(i) / slices, static_cast<float>(j + 1) / stacks));
            texCoords.push_back(Vector2(static_cast<float>(i + 1) / slices, static_cast<float>(j) / stacks));
            texCoords.push_back(Vector2(static_cast<float>(i) / slices, static_cast<float>(j + 1) / stacks));
            texCoords.push_back(Vector2(static_cast<float>(i + 1) / slices, static_cast<float>(j + 1) / stacks));
            texCoords.push_back(Vector2(static_cast<float>(i + 1) / slices, static_cast<float>(j) / stacks));
        }
    }

    writeToFile(filename, vertices, normals, texCoords);
}

std::vector<std::vector<std::vector<float>>> readPatchesFile(const char* filePath) {
    std::ifstream file(filePath);
    std::vector<std::vector<std::vector<float>>> result;
    if (file.is_open()) {
        std::string line;
        if (!std::getline(file, line)) return result;
        int numPatches = std::stoi(line);
        std::vector<std::vector<int>> indicesPerPatch;

        for (int i = 0; i < numPatches; i++) {
            if (!std::getline(file, line)) return result;
            std::vector<int> indices;
            std::stringstream ss(line);
            std::string token;
            while (std::getline(ss, token, ',')) {
                indices.push_back(std::stoi(token));
            }
            indicesPerPatch.push_back(indices);
        }

        if (!std::getline(file, line)) return result;
        int numControlPoints = std::stoi(line);

        std::vector<std::vector<float>> controlPoints;
        for (int i = 0; i < numControlPoints; i++) {
            if (!std::getline(file, line)) return result;
            std::vector<float> point;
            std::stringstream ss(line);
            std::string token;
            while (std::getline(ss, token, ',')) {
                point.push_back(std::stof(token));
            }
            controlPoints.push_back(point);
        }

        for (const auto& indices : indicesPerPatch) {
            std::vector<std::vector<float>> patch;
            for (int indice : indices) {
                patch.push_back(controlPoints[indice]);
            }
            result.push_back(patch);
        }
    }
    else {
        std::cerr << "Erro ao abrir o arquivo: " << filePath << std::endl;
    }
    return result;
}

void multiplyMatrices(int la, int ca, const float* A, int lb, int cb, const float* B, float* R) {
    if (ca == lb) {
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

void surfacePoint(float u, float v, const std::vector<std::vector<float>>& patch, float* res) {
    float M[16] = { -1, 3, -3, 1, 3, -6, 3, 0, -3, 3, 0, 0, 1, 0, 0, 0 };
    float U[4] = { u * u * u, u * u, u, 1 };
    float V[4] = { v * v * v, v * v, v, 1 };
    float UM[4], MV[4];

    multiplyMatrices(1, 4, U, 4, 4, M, UM);
    multiplyMatrices(4, 4, M, 4, 1, V, MV);

    float P[3][16];
    for (int i = 0; i < 16; ++i) {
        P[0][i] = patch[i][0];
        P[1][i] = patch[i][1];
        P[2][i] = patch[i][2];
    }

    for (int i = 0; i < 3; ++i) {
        res[i] = 0;
        for (int j = 0; j < 4; ++j) {
            res[i] += UM[j] * P[i][j] * MV[j];
        }
    }
}

void derivativeSurfacePoint(float u, float v, const std::vector<std::vector<float>>& patch, float* dU, float* dV) {
    float M[16] = { -1.0f,  3.0f, -3.0f, 1.0f,
                     3.0f, -6.0f,  3.0f, 0.0f,
                    -3.0f,  3.0f,  0.0f, 0.0f,
                     1.0f,  0.0f,  0.0f, 0.0f };
    float Ud[4] = { 3 * u * u, 2 * u, 1, 0 };
    float Vd[4] = { 3 * v * v, 2 * v, 1, 0 };
    float U[4] = { u * u * u, u * u, u, 1 };
    float V[4] = { v * v * v, v * v, v, 1 };

    float UDM[4], VDM[4];
    multiplyMatrices(1, 4, Ud, 4, 4, M, UDM);
    multiplyMatrices(4, 4, M, 4, 1, Vd, VDM);

    float P[3][16];
    for (int i = 0; i < 16; ++i) {
        P[0][i] = patch[i][0];
        P[1][i] = patch[i][1];
        P[2][i] = patch[i][2];
    }

    for (int i = 0; i < 3; ++i) {
        dU[i] = 0;
        dV[i] = 0;
        for (int j = 0; j < 4; ++j) {
            dU[i] += UDM[j] * P[i][j];
            dV[i] += VDM[j] * P[i][j];
        }
    }
}

Vector3 computeNormal(const std::vector<std::vector<float>>& patch, float u, float v) {
    float dU[3], dV[3];
    derivativeSurfacePoint(u, v, patch, dU, dV);
    Vector3 tangentU(dU[0], dU[1], dU[2]);
    Vector3 tangentV(dV[0], dV[1], dV[2]);

    Vector3 normal = tangentU.cross(tangentV);
    if (normal.x == 0 && normal.y == 0 && normal.z == 0) {
        return Vector3(0, 1, 0);
    }
    return normal.normalized();
}

void buildPatches(const char* filePath, int tessellation, const std::string& outputFile) {
    std::vector<std::vector<std::vector<float>>> patches = readPatchesFile(filePath);
    if (patches.empty()) {
        std::cerr << "Erro: Nenhum patch encontrado." << std::endl;
        return;
    }

    std::vector<Vector3> vertices, normals;
    std::vector<Vector2> texCoords;

    float delta = 1.0f / tessellation;
    for (const auto& patch : patches) {
        for (int i = 0; i <= tessellation; ++i) {
            float u = i * delta;
            for (int j = 0; j <= tessellation; ++j) {
                float v = j * delta;
                float res[3];
                surfacePoint(u, v, patch, res);
                Vector3 point(res[0], res[1], res[2]);
                Vector3 normal = computeNormal(patch, u, v);

                vertices.push_back(point);
                normals.push_back(normal);
                texCoords.push_back(Vector2(u, 1 - v));
            }
        }
    }

    writeToFile(outputFile, vertices, normals, texCoords);
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " [sphere|box|cone|plane|patch] <parameters> filename\n";
        return 1;
    }

    std::string shapeType = argv[1];
    std::string filename;

    if (shapeType == "sphere" && argc == 6) {
        float radius = std::stof(argv[2]);
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
        const char* filePath = argv[2];
        int tessellation = std::stoi(argv[3]);
        filename = argv[4];
        buildPatches(filePath, tessellation, filename);
    }
    else {
        std::cerr << "Invalid parameters or command\n";
        return 1;
    }

    return 0;
}