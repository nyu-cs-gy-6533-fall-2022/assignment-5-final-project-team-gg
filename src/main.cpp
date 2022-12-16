// This example is heavily based on the tutorial at https://open.gl
#include <algorithm>
// OpenGL Helpers to reduce the clutter
#include "Helpers.h"
#include "object.h"
#include "light.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#else
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#endif

// OpenGL Mathematics Library
#include <glm/glm.hpp> // glm::vec3
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#define _USE_MATH_DEFINES
#include <math.h>

#include "object.h";

// VertexBufferObject wrapper
BufferObject VBO;
// VertexBufferObject wrapper
BufferObject NBO;
// VertexBufferObject wrapper
BufferObject IndexBuffer;

BufferObject UTBO;

// Contains the vertex positions
std::vector<glm::vec3> V;
// Contains the vertex normal
std::vector<glm::vec3> VN;
// Contains the vertex index
std::vector<glm::ivec3> T;
//
std::vector<glm::vec2> TC;
// data for tbo
std::vector<float> tbo;
std::vector<float> tbo2;
std::vector<float> tbo3;
// Last position of the mouse on click
double xpos, ypos;

// camera setup and matrix calculations
glm::vec3 cameraPos;
glm::vec3 cameraTarget;
glm::vec3 cameraDirection;
glm::vec3 cameraUp;
glm::vec3 cameraRight;
glm::mat4 viewMatrix;
glm::mat4 projMatrix;
glm::mat4 modelMatrix;

float camRadius = 5.0f;
bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
//0: static, 1: cursor
bool mode = 0;

// PPM Reader code from http://josiahmanson.com/prose/optimize_ppm/

struct RGB {
    unsigned char r, g, b;
};

struct ImageRGB {
    int w, h;
    std::vector<RGB> data;
};

void eat_comment(std::ifstream& f) {
    char linebuf[1024];
    char ppp;
    while (ppp = f.peek(), ppp == '\n' || ppp == '\r')
        f.get();
    if (ppp == '#')
        f.getline(linebuf, 1023);
}

bool loadPPM(ImageRGB& img, const std::string& name) {
    std::ifstream f(name.c_str(), std::ios::binary);
    if (f.fail()) {
        std::cout << "Could not open file: " << name << std::endl;
        return false;
    }

    // get type of file
    eat_comment(f);
    int mode = 0;
    std::string s;
    f >> s;
    if (s == "P3")
        mode = 3;
    else if (s == "P6")
        mode = 6;

    // get w
    eat_comment(f);
    f >> img.w;

    // get h
    eat_comment(f);
    f >> img.h;

    // get bits
    eat_comment(f);
    int bits = 0;
    f >> bits;

    // error checking
    if (mode != 3 && mode != 6) {
        std::cout << "Unsupported magic number" << std::endl;
        f.close();
        return false;
    }
    if (img.w < 1) {
        std::cout << "Unsupported width: " << img.w << std::endl;
        f.close();
        return false;
    }
    if (img.h < 1) {
        std::cout << "Unsupported height: " << img.h << std::endl;
        f.close();
        return false;
    }
    if (bits < 1 || bits > 255) {
        std::cout << "Unsupported number of bits: " << bits << std::    endl;
        f.close();
        return false;
    }

    // load image data
    img.data.resize(img.w * img.h);

    if (mode == 6) {
        f.get();
        f.read((char*)&img.data[0], img.data.size() * 3);
    }
    else if (mode == 3) {
        for (int i = 0; i < img.data.size(); i++) {
            int v;
            f >> v;
            img.data[i].r = v;
            f >> v;
            img.data[i].g = v;
            f >> v;
            img.data[i].b = v;
        }
    }

    // close file
    f.close();
    return true;
}

bool loadOFFFile(std::string filename, std::vector<glm::vec3>& vertex, std::vector<glm::ivec3>& indices, glm::vec3& min, glm::vec3& max)
{
    min.x = FLT_MAX;
    max.x = FLT_MIN;
    min.y = FLT_MAX;
    max.y = FLT_MIN;
    min.z = FLT_MAX;
    max.z = FLT_MIN;
    try {
        std::ifstream ofs(filename, std::ios::in | std::ios_base::binary);
        if (ofs.fail()) return false;
        std::string line, tmpStr;
        // First line(optional) : the letters OFF to mark the file type.
        // Second line : the number of vertices, number of faces, and number of edges, in order (the latter can be ignored by writing 0 instead).
        int numVert = 0;
        int numFace = 0;
        int numEdge = 0;
        // first line must be OFF
        getline(ofs, line);
        if (line.rfind("OFF", 0) == 0)
            getline(ofs, line);
        std::stringstream tmpStream(line);
        getline(tmpStream, tmpStr, ' ');
        numVert = std::stoi(tmpStr);
        getline(tmpStream, tmpStr, ' ');
        numFace = std::stoi(tmpStr);
        getline(tmpStream, tmpStr, ' ');
        numEdge = std::stoi(tmpStr);

        // read all vertices and get min/max values
        V.resize(numVert);
        for (int i = 0; i < numVert; i++) {
            getline(ofs, line);
            tmpStream.clear();
            tmpStream.str(line);
            getline(tmpStream, tmpStr, ' ');
            V[i].x = std::stof(tmpStr);
            min.x = std::fminf(V[i].x, min.x);
            max.x = std::fmaxf(V[i].x, max.x);
            getline(tmpStream, tmpStr, ' ');
            V[i].y = std::stof(tmpStr);
            min.y = std::fminf(V[i].y, min.y);
            max.y = std::fmaxf(V[i].y, max.y);
            getline(tmpStream, tmpStr, ' ');
            V[i].z = std::stof(tmpStr);
            min.z = std::fminf(V[i].z, min.z);
            max.z = std::fmaxf(V[i].z, max.z);
        }

        // read all faces (triangles)
        T.resize(numFace);
        for (int i = 0; i < numFace; i++) {
            getline(ofs, line);
            tmpStream.clear();
            tmpStream.str(line);
            getline(tmpStream, tmpStr, ' ');
            if (std::stoi(tmpStr) != 3) return false;
            getline(tmpStream, tmpStr, ' ');
            T[i].x = std::stoi(tmpStr);
            getline(tmpStream, tmpStr, ' ');
            T[i].y = std::stoi(tmpStr);
            getline(tmpStream, tmpStr, ' ');
            T[i].z = std::stoi(tmpStr);
        }

        ofs.close();
    }
    catch (const std::exception& e) {
        // return false if an exception occurred
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

unsigned int sphere(float sphereRadius, int sectorCount, int stackCount,
    std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
    std::vector<glm::ivec3>& indices, std::vector<glm::vec2>& textCoords) {
    // init variables
    vertices.resize(0);
    normals.resize(0);
    indices.resize(0);
    textCoords.resize(0);
    // temp variables
    glm::vec3 sphereVertexPos;
    glm::vec2 textureCoordinate;
    float xy;
    float sectorStep = 2.0f * M_PI / float(sectorCount);
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    // compute vertices and normals
    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = M_PI / 2.0f - i * stackStep;
        xy = sphereRadius * cosf(stackAngle);
        sphereVertexPos.z = sphereRadius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;

            // vertex position
            sphereVertexPos.x = xy * sinf(sectorAngle);
            sphereVertexPos.y = xy * cosf(sectorAngle);
            vertices.push_back(sphereVertexPos);

            // normalized vertex normal
            normals.push_back(sphereVertexPos / sphereRadius);

            // calculate texture coordinate
            textureCoordinate.x = float(j) / sectorCount;
            textureCoordinate.y = float(i) / stackCount;
            textCoords.push_back(textureCoordinate);
        }
    }

    // compute triangle indices
    unsigned int k1, k2;
    unsigned int maxElementIndice = 0;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0) {
                indices.push_back(glm::ivec3(k1, k2, k1 + 1));
            }
            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1)) {
                indices.push_back(glm::ivec3(k1 + 1, k2, k2 + 1));
            }
            maxElementIndice = std::max(maxElementIndice, std::max(k1, k2));
        }
    }
    return maxElementIndice + 1;

}


unsigned int torus(float outerRadius, float innerRadius, int sectorCount, int stackCount,
    std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
    std::vector<glm::ivec3>& indices, std::vector<glm::vec2>& textCoords) {
    // init variables
    vertices.resize(0);
    normals.resize(0);
    indices.resize(0);
    textCoords.resize(0);
    // temp variables
    glm::vec3 torusVertexPos;
    glm::vec2 textureCoordinate;

    float sectorStep = 2.0f * M_PI / float(sectorCount);
    float stackStep = 2.0f * M_PI / float(stackCount);
    float sectorAngle, stackAngle;

    // compute vertices and normals
    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = M_PI / 2.0f - i * stackStep;
        torusVertexPos.y = innerRadius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;

            // vertex position
            torusVertexPos.x = sinf(sectorAngle) * (innerRadius * cosf(stackAngle) + outerRadius);
            torusVertexPos.z = cosf(sectorAngle) * (innerRadius * cosf(stackAngle) + outerRadius);
            vertices.push_back(torusVertexPos);

            // normalized vertex normal
            normals.push_back(glm::normalize(torusVertexPos - glm::vec3(outerRadius * sinf(sectorAngle), 0.0f, outerRadius * cosf(sectorAngle))));

            // calculate texture coordinate
            textureCoordinate.x = float(j) / sectorCount;
            textureCoordinate.y = float(i) / stackCount;
            textCoords.push_back(textureCoordinate);
        }
    }

    // compute triangle indices
    unsigned int k1, k2;
    unsigned int maxElementIndice = 0;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1

            indices.push_back(glm::ivec3(k1, k2, k1 + 1));

            // k1+1 => k2 => k2+1

            indices.push_back(glm::ivec3(k1 + 1, k2, k2 + 1));

            maxElementIndice = std::max(maxElementIndice, std::max(k1, k2));

        }
    }
    return maxElementIndice + 1;

}

unsigned int truncatedCone(float topRadius, float baseRadius, int sectorCount, float height,
    std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
    std::vector<glm::ivec3>& indices, std::vector<glm::vec2>& textCoords) {
    //
    float sectorStep = 2.0f * M_PI / float(sectorCount);
    float sectorAngle;
    std::vector<float> unitCircleVertices;
    glm::vec3 cylinderVertexPos;
    glm::vec2 textureCoordinate;
    float coneAngle = atanf((baseRadius - topRadius) / height);
    unsigned int maxElementIndice = 0;

    // init variables
    vertices.resize(0);
    normals.resize(0);
    indices.resize(0);
    textCoords.resize(0);

    // get unit circle vectors on XY-plane
    for (int i = 0; i <= sectorCount; ++i){
        sectorAngle = i * sectorStep;
        unitCircleVertices.push_back(sin(sectorAngle)); // x
        unitCircleVertices.push_back(0); // y
        unitCircleVertices.push_back(cos(sectorAngle));                // z
    }

    // put side vertices to arrays
    for (int i = 0; i < 2; ++i)
    {
        float h = -height / 2.0f + i * height;           // z value; -h/2 to h/2
        float t = 1.0f - i;                              // vertical tex coord; 1 to 0

        for (int j = 0, k = 0; j <= sectorCount; ++j, k += 3)
        {
            sectorAngle = j * sectorStep;

            float ux = unitCircleVertices[k];
            float uy = unitCircleVertices[k + 1];
            float uz = unitCircleVertices[k + 2];
            // position vector
            float sphereRadius = (i == 0) ? baseRadius : topRadius;
            cylinderVertexPos.x = ux * sphereRadius;
            cylinderVertexPos.y = h;
            cylinderVertexPos.z = uz * sphereRadius;
            vertices.push_back(cylinderVertexPos);

            // normal vector
            normals.push_back(glm::normalize(glm::vec3(
                cosf(coneAngle) * sinf(sectorAngle),
                sinf(coneAngle),
                cosf(coneAngle) * cosf(sectorAngle)
            )));

            // calculate texture coordinate
            textureCoordinate.x = (float)j / sectorCount;
            textureCoordinate.y = t;
            textCoords.push_back(textureCoordinate);
        }
    }

    int baseCenterIndex = (int)vertices.size();
    int topCenterIndex = baseCenterIndex + sectorCount + 1; // include center vertex

    // put base and top vertices to arrays
    for (int i = 0; i < 2; ++i)
    {
        float h = -height / 2.0f + i * height;           // z value; -h/2 to h/2
        float ny = -1 + i * 2;                           // z value of normal; -1 to 1

        // center point
        vertices.push_back(glm::vec3(0.0f, h, 0.0f));
        normals.push_back(glm::vec3(0.0f, ny, 0.0f));
        textCoords.push_back(glm::vec2(0.5f, 0.5f));

        for (int j = 0, k = 0; j < sectorCount; ++j, k += 3)
        {
            float ux = unitCircleVertices[k];
            float uz = unitCircleVertices[k + 2];
            // position vector
            float sphereRadius = (i == 0) ? baseRadius : topRadius;
            cylinderVertexPos.x = ux * sphereRadius;
            cylinderVertexPos.y = h;
            cylinderVertexPos.z = uz * sphereRadius;
            vertices.push_back(cylinderVertexPos);

            // normal vector
            normals.push_back(glm::vec3(0.0f, ny, 0.0f));

            // texture coordinate
            textureCoordinate.x = -ux * 0.5f + 0.5f;
            textureCoordinate.y = -uz * 0.5f + 0.5f;
            textCoords.push_back(textureCoordinate);

        }
    }

    unsigned int k1 = 0;                         // 1st vertex index at base
    unsigned int k2 = sectorCount + 1;           // 1st vertex index at top

    // indices for the side surface
    for (int i = 0; i < sectorCount; ++i, ++k1, ++k2)
    {
        // 2 triangles per sector
        // k1 => k1+1 => k2
        indices.push_back(glm::ivec3(k1, k2, k1 + 1));

        // k2 => k1+1 => k2+1
        indices.push_back(glm::ivec3(k1 + 1, k2, k2 + 1));
        maxElementIndice = std::max(maxElementIndice, std::max(k1, k2));
    }

    for (unsigned int i = 0, k = baseCenterIndex + 1; i < sectorCount; ++i, ++k)
    {
        if (i < sectorCount - 1)
        {
            indices.push_back(glm::ivec3(baseCenterIndex, k + 1, k));
        }
        else // last triangle
        {
            indices.push_back(glm::ivec3(baseCenterIndex, baseCenterIndex + 1, k));
        }
        maxElementIndice = std::max(maxElementIndice, k);
    }

    // indices for the top surface
    if (topRadius != 0) {
        for (unsigned int i = 0, k = topCenterIndex + 1; i < sectorCount; ++i, ++k)
        {
            if (i < sectorCount - 1)
            {
                indices.push_back(glm::ivec3(topCenterIndex, k, k + 1));
            }
            else // last triangle
            {
                indices.push_back(glm::ivec3(topCenterIndex, k, topCenterIndex + 1));
            }
            maxElementIndice = std::max(maxElementIndice, k);
        }
    }
    return maxElementIndice + 1;
}

unsigned int cone(float radius, int sectorCount, float height,
    std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
    std::vector<glm::ivec3>& indices, std::vector<glm::vec2>& textCoords) {
    return truncatedCone(0.0f, radius, sectorCount, height, vertices, normals, indices, textCoords);
}

unsigned int cylinder(float radius, int sectorCount, float height,
    std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
    std::vector<glm::ivec3>& indices, std::vector<glm::vec2>& textCoords) {
    return truncatedCone(radius, radius, sectorCount, height, vertices, normals, indices, textCoords);
}
unsigned int capsule(float topRadius, float baseRadius, int sectorCount, int stackCount, float height,
    std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
    std::vector<glm::ivec3>& indices, std::vector<glm::vec2>& textCoords) {
    //
    float xy;
    float sectorStep = 2.0f * M_PI / float(sectorCount);
    float stackStep = M_PI / float(stackCount);
    float sectorAngle, stackAngle;
    std::vector<float> unitCircleVertices;
    glm::vec3 capsuleVertexPos;
    glm::vec2 textureCoordinate;

    float cylinderHeight = height - topRadius - baseRadius;
    glm::vec3 topCenter(0.0f, cylinderHeight / 2.0f, 0.0f);
    glm::vec3 baseCenter(0.0f, -cylinderHeight / 2.0f, 0.0f);

    float coneAngle = atanf((baseRadius - topRadius) / cylinderHeight);

    // init variables
    vertices.resize(0);
    normals.resize(0);
    indices.resize(0);
    textCoords.resize(0);

    // get unit circle vectors on XZ-plane
    for (int i = 0; i <= sectorCount; ++i) {
        sectorAngle = i * sectorStep;
        unitCircleVertices.push_back(sinf(sectorAngle));                 // x
        unitCircleVertices.push_back(0);                                // y
        unitCircleVertices.push_back(cosf(sectorAngle));                 // z
    }


    // vertex, normal, texture coordinate
    for (int x = 0; x < 3; ++x) {
        switch (x) {
        case 0:     // top semisphere
            for (int i = 0; i <= (stackCount / 2); ++i) {
                stackAngle = M_PI / 2.0f - i * stackStep;
                xy = topRadius * cosf(stackAngle);
                capsuleVertexPos.y = topRadius * sinf(stackAngle) + (cylinderHeight / 2);
                for (int j = 0; j <= sectorCount; ++j) {
                    sectorAngle = j * sectorStep;

                    // vertex position
                    capsuleVertexPos.x = xy * sinf(sectorAngle);
                    capsuleVertexPos.z = xy * cosf(sectorAngle);
                    vertices.push_back(capsuleVertexPos);

                    // normalized vertex normal
                    normals.push_back(glm::normalize(capsuleVertexPos - topCenter));

                    // calculate texture coordinate
                    textureCoordinate.x = float(j) / sectorCount;
                    textureCoordinate.y = float(i) * (topRadius / height) / (stackCount/2);
                    textCoords.push_back(textureCoordinate);
                }
            }
            break;
        case 1:     // cylinder
            for (int i = 1; i >= 0; --i) {
                float h = -cylinderHeight / 2.0f + i * cylinderHeight;           // z value; h/2 to -h/2
                float t = 1.0f - i;                              // vertical tex coord; 1 to 0
                for (int j = 0, k = 0; j <= sectorCount; ++j, k += 3) {
                    sectorAngle = j * sectorStep;

                    float ux = unitCircleVertices[k];
                    float uy = unitCircleVertices[k + 1];
                    float uz = unitCircleVertices[k + 2];
                    // position vector
                    float capsuleRadius = (i == 0) ? baseRadius : topRadius;
                    capsuleVertexPos.x = ux * capsuleRadius;
                    capsuleVertexPos.y = h;
                    capsuleVertexPos.z = uz * capsuleRadius;
                    vertices.push_back(capsuleVertexPos);

                    // normal vector
                    normals.push_back(glm::normalize(glm::vec3(
                        cosf(coneAngle) * sinf(sectorAngle),
                        sinf(coneAngle),
                        cosf(coneAngle) * cosf(sectorAngle)
                    )));

                    // calculate texture coordinate
                    textureCoordinate.x = (float)j / sectorCount;
                    textureCoordinate.y = (i == 1) ? (topRadius / height) : ((height - baseRadius) / height);
                    textCoords.push_back(textureCoordinate);
                }
            }
            break;
        case 2:     // bottom semisphere
            for (int i = (stackCount / 2); i <= stackCount; ++i) {
                stackAngle = M_PI / 2.0f - i * stackStep;
                xy = baseRadius * cosf(stackAngle);
                capsuleVertexPos.y = (baseRadius * sinf(stackAngle)) - (cylinderHeight / 2);
                for (int j = 0; j <= sectorCount; ++j) {
                    sectorAngle = j * sectorStep;

                    // vertex position
                    capsuleVertexPos.x = xy * sinf(sectorAngle);
                    capsuleVertexPos.z = xy * cosf(sectorAngle);
                    vertices.push_back(capsuleVertexPos);

                    // normalized vertex normal
                    normals.push_back(glm::normalize(capsuleVertexPos - baseCenter));

                    // calculate texture coordinate
                    textureCoordinate.x = float(j) / sectorCount;
                    textureCoordinate.y = (float(i - (stackCount / 2)) * (baseRadius / height) / (stackCount / 2)) + ((height - baseRadius) / height);
                    textCoords.push_back(textureCoordinate);
                }
            }
            break;
        }
    }

    // indices
        // compute triangle indices
    unsigned int k1, k2;
    unsigned int maxElementIndice;
    for (int i = 0; i < (stackCount + 3); ++i) {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0) {
                indices.push_back(glm::ivec3(k1, k2, k1 + 1));
            }
            // k1+1 => k2 => k2+1
            if (i != (stackCount + 2)) {
                indices.push_back(glm::ivec3(k1 + 1, k2, k2 + 1));
            }
            maxElementIndice = std::max(maxElementIndice, std::max(k1, k2));
        }
    }
    return maxElementIndice + 1;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Get the position of the mouse in the window
        glfwGetCursorPos(window, &xpos, &ypos);
        std::cout << xpos << " " << ypos << std::endl;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        mode = (mode == 0) ? 1 : 0;
    }
}

static void cursor_position_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    float sensitivity = 0.5f; 
    glm::vec3 front;

    if (firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
    float xz = cos(glm::radians(pitch));
    front.x = cos(glm::radians(yaw)) * xz;
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * xz;
    cameraDirection = glm::normalize(front);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    float watchDistance = 5.0f;
    // temp variables
    glm::mat3 rot;
    // Update the position of the first vertex if the keys 1,2, or 3 are pressed
    switch (key)
    {
    case GLFW_KEY_A:
        rot = glm::rotate(glm::mat4(1.0f), glm::radians(-5.0f), cameraUp);
        cameraPos = rot * cameraPos;
        cameraDirection = glm::normalize(cameraPos - cameraTarget);
        cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));
        break;
    case GLFW_KEY_D:
        rot = glm::rotate(glm::mat4(1.0f), glm::radians(5.0f), cameraUp);
        cameraPos = rot * cameraPos;
        cameraDirection = glm::normalize(cameraPos - cameraTarget);
        cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));
        break;
    case GLFW_KEY_W:
        rot = glm::rotate(glm::mat4(1.0f), glm::radians(-5.0f), cameraRight);
        cameraPos = rot * cameraPos;
        cameraDirection = glm::normalize(cameraPos - cameraTarget);
        cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
        break;
    case GLFW_KEY_S:
        rot = glm::rotate(glm::mat4(1.0f), glm::radians(5.0f), cameraRight);
        cameraPos = rot * cameraPos;
        cameraDirection = glm::normalize(cameraPos - cameraTarget);
        cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
        break;
    case GLFW_KEY_UP:
        cameraPos += cameraDirection * 0.25f;
        break;
    case GLFW_KEY_DOWN:
        cameraPos -= cameraDirection * 0.25f;
        break;
    case GLFW_KEY_LEFT:
        cameraPos -= glm::normalize(glm::cross(cameraDirection, cameraUp)) * 0.25f;
        break;
    case GLFW_KEY_RIGHT:
        cameraPos += glm::normalize(glm::cross(cameraDirection, cameraUp)) * 0.25f;
        break;
    case GLFW_KEY_1:
        modelMatrix = glm::scale( modelMatrix, glm::vec3(1.1) );
        break;
    case GLFW_KEY_2:
        modelMatrix = glm::scale( modelMatrix, glm::vec3(0.9) );
        break;

    case GLFW_KEY_R:
        cameraPos = glm::vec3(0.0f, 0.0f, camRadius);
        cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        cameraDirection = glm::normalize(cameraPos - cameraTarget);
        cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));
        modelMatrix = glm::mat4(1.0);
        break;
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;
    default:
        break;
    }

}

void TBO_prepare(std::vector<float>& tbo, std::vector<glm::vec3>& vertex, std::vector<glm::vec3>& normal, 
                    std::vector<glm::ivec3>& tria, glm::vec3 col, bool is_reflecing, bool is_light){
    std::cout<<tria.size()<<std::endl;
    for(int i = 0; i < tria.size(); ++i){
        // positions
        tbo.push_back(vertex[tria[i].x].x);
        tbo.push_back(vertex[tria[i].x].y);
        tbo.push_back(vertex[tria[i].x].z);
        tbo.push_back(vertex[tria[i].y].x);
        tbo.push_back(vertex[tria[i].y].y);
        tbo.push_back(vertex[tria[i].y].z);
        tbo.push_back(vertex[tria[i].z].x);
        tbo.push_back(vertex[tria[i].z].y);
        tbo.push_back(vertex[tria[i].z].z);
        // normal
        tbo.push_back(normal[tria[i].x].x);
        tbo.push_back(normal[tria[i].x].y);
        tbo.push_back(normal[tria[i].x].z);
        tbo.push_back(normal[tria[i].y].x);
        tbo.push_back(normal[tria[i].y].y);
        tbo.push_back(normal[tria[i].y].z);
        tbo.push_back(normal[tria[i].z].x);
        tbo.push_back(normal[tria[i].z].y);
        tbo.push_back(normal[tria[i].z].z);
        // color
        tbo.push_back(col.x);
        tbo.push_back(col.y);
        tbo.push_back(col.z);
        // is_reflecing
        if(is_reflecing) tbo.push_back(5);
        else tbo.push_back(0);
        // is_light
        if(is_light) tbo.push_back(5);
        else tbo.push_back(0);
        tbo.push_back(0);
        
    }
}


void TBOlight_prepare(std::vector<float>& tbo, int id, 
    std::vector<glm::vec3>& vertex,
    glm::vec3 direction,
    float Ia, float Ii) {

    std::cout << Ia << Ii << std::endl;
    tbo.push_back((float)id);
    tbo.push_back(Ia);
    tbo.push_back(Ii);
    for (int i = 0; i < 4; ++i) {
        // positions
        tbo.push_back(vertex[i].x);
        tbo.push_back(vertex[i].y);
        tbo.push_back(vertex[i].z);
    }

    tbo.push_back(direction.x);
    tbo.push_back(direction.y);
    tbo.push_back(direction.z);

}

void TBOtex_prepare(std::vector<float>& tbo, std::vector<glm::vec2> tc){

    for (glm::vec2& i : tc) {
        tbo.push_back(i.x);
        tbo.push_back(i.y);
        tbo.push_back(0.0);
    }
}


void TBO_test(std::vector<glm::vec3>& vertex, std::vector<glm::vec3>& normal, 
                    std::vector<glm::ivec3>& tria){
    vertex.push_back(glm::vec3(-10, -10, -3));
    vertex.push_back(glm::vec3(-10, 10, -3));
    vertex.push_back(glm::vec3(10, -10, -3));
    normal.push_back(glm::vec3(0, 0, 1));
    normal.push_back(glm::vec3(0, 0, 1));    
    normal.push_back(glm::vec3(0, 0, 1));
    tria.push_back(glm::vec3(vertex.size()-3,vertex.size()-2,vertex.size()-1));
    //
    vertex.push_back(glm::vec3(10, -10, -3));
    vertex.push_back(glm::vec3(-10, 10, -3));
    vertex.push_back(glm::vec3(10, 10, -3));
    normal.push_back(glm::vec3(0, 0, 1));
    normal.push_back(glm::vec3(0, 0, 1));    
    normal.push_back(glm::vec3(0, 0, 1));
    tria.push_back(glm::vec3(vertex.size()-3,vertex.size()-2,vertex.size()-1));
    //
}




int main(void)
{
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())   
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(800, 600, "Hello OpenGL", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    #ifndef __APPLE__
      glewExperimental = true;
      GLenum err = glewInit();
      if(GLEW_OK != err)
      {
        /* Problem: glewInit failed, something is seriously wrong. */
       std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
      }
      glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
      std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
    #endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    std::cout << "OpenGL version recieved: " << major << "." << minor << "." << rev << std::endl;
    std::cout << "Supported OpenGL is " << (const char*)glGetString(GL_VERSION) << std::endl;
    std::cout << "Supported GLSL is " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Initialize the VAO
    // A Vertex Array Object (or VAO) is an object that describes how the vertex
    // attributes are stored in a Vertex Buffer Object (or VBO). This means that
    // the VAO is not the actual object storing the vertex data,
    // but the descriptor of the vertex data.
    VertexArrayObject VAO;
    VAO.init();
    VAO.bind();

    // Initialize the VBO with the vertices data
    VBO.init();
    // initialize normal array buffer
    NBO.init();
    // initialize element array buffer
    IndexBuffer.init(GL_ELEMENT_ARRAY_BUFFER);

    UTBO.init();


    // initialize model matrix
    modelMatrix = glm::mat4(1.0f);

    // 1: generate sphere, 0: load OFF model
#if 1
    //////////////////////////////object//////////////////////////////////////////
    std::vector<Object*> objs;


    
    // Torus ta(1.0f, 0.5f, 5, 5);
    // ta.maxIndex = torus(1.0f, 0.5f, 5, 5, ta.vertices, ta.normals, ta.indices, ta.texCoords);
    // ta.offset(glm::vec3(0.0f, -2.0f, 0.0));
    // objs.push_back(&ta);

    Torus tb(0.5f, 0.2f, 30, 30);
    tb.maxIndex = torus(0.5f, 0.2f, 30, 30, tb.vertices, tb.normals, tb.indices, tb.texCoords);
    tb.offset(glm::vec3(0.0f, 1.0f, 0.0f));
    objs.push_back(&tb);


    // Sphere tc(0.2f, 30, 30);
    // tc.maxIndex = sphere(0.2f, 30, 30, tc.vertices, tc.normals, tc.indices, tc.texCoords);
    // std::cout << tc.maxIndex << " " << tc.vertices.size() << " " << tc.indices.size();
    // tc.offset(glm::vec3(0.0f, -0.5f, 0.0));
    // objs.push_back(&tc);

    TruncatedCone td(0.2f, 0.4f, 30, 1);
    td.maxIndex = truncatedCone(0.5f, 0.8f, 30, 0.5, td.vertices, td.normals, td.indices, td.texCoords);
    //td.offset(glm::vec3(1.0, -1.0, 0.0));
    td.color = glm::vec3(0.0, 1.0, 0.0);
    //objs.push_back(&td);

    Capsule tf(0.2, 0.8, 20, 20);
    tf.maxIndex = capsule(0.2, 0.2, 30, 30, 0.8, tf.vertices, tf.normals, tf.indices, tf.texCoords);
    tf.offset(glm::vec3(-1.0, 0.0, 0.0));
    tf.color = glm::vec3(1.0, 0.5, 0.0);
    //objs.push_back(&tf);

    Cone tg(0.2, 0.4, 30);
    tg.maxIndex = cone(0.2, 30, 0.4, tg.vertices, tg.normals, tg.indices, tg.texCoords);
    tg.color = glm::vec3(0.0, 0.5, 0.9);
    //objs.push_back(&tg);


    Cylinder th(0.2, 0.3, 30);
    th.maxIndex = cylinder(0.2, 30, 0.3, th.vertices, th.normals, th.indices, th.texCoords);
    th.color = glm::vec3(0.1, 0.1, 0.1);
    th.offset(glm::vec3(0.0, 0.0, 1.0));
    //objs.push_back(&th);

    Plane te(glm::vec3(-10.0f, 0.0f, -10.0f), glm::vec3(10.0f, 0.0f, -10.0f));
    te.offset(glm::vec3( 0.0f, 6.0f, 0.0f));
    te.reflect = false;
    te.color = glm::vec3(0.7, 0.7, 0.9);
    //objs.push_back(&te);    

    Plane te2(glm::vec3(-10.0f, 0.0f, -10.0f), glm::vec3(10.0f, 0.0f, -10.0f));
    te2.offset(glm::vec3( 0.0f, -2.0f, 0.0f));
    te2.reflect = false;
    te2.color = glm::vec3(0.7, 0.7, 0.9);
    objs.push_back(&te2);

    Plane te3(glm::vec3(-10.0f, 10.0f, 0.0f), glm::vec3(10.0f, 10.0f, 0.0f));
    te3.offset(glm::vec3( 0.0f, 0.0f, -6.0f));
    te3.reflect = false;
    te3.color = glm::vec3(0.7, 0.7, 0.9);
    objs.push_back(&te3);

    Plane te4(glm::vec3(-10.0f, 10.0f, 0.0f), glm::vec3(10.0f, 10.0f, 0.0f));
    te4.offset(glm::vec3( 0.0f, .0f, 6.0f));
    te4.reflect = false;
    te4.color = glm::vec3(0.7, 0.7, 0.9);
    objs.push_back(&te4);

    Plane te5(glm::vec3(0.0f, 10.0f, 10.0f), glm::vec3(0.0f, 10.0f, -10.0f));
    te5.offset(glm::vec3( 6.0f, 0.0f, 0.0f));
    te5.reflect = false;
    te5.color = glm::vec3(0.7, 0.7, 0.9);
    objs.push_back(&te5);

    Plane te6(glm::vec3(0.0f, 10.0f, 10.0f), glm::vec3(0.0f, 10.0f, -10.0f));
    te6.offset(glm::vec3( -6.0f, -0.0f, 0.0f));
    te6.reflect = false;
    te6.color = glm::vec3(0.7, 0.7, 0.9);
    objs.push_back(&te6);   


    int indicesMax = 0;
    V.resize(0); VN.resize(0); T.resize(0);
    for (Object* i : objs) {
        int vsize = V.size();
        V.insert(V.end(), i->vertices.begin(), i->vertices.end());
        VN.insert(VN.end(), i->normals.begin(), i->normals.end());
        i->adjustIndice(vsize);
        T.insert(T.end(), i->indices.begin(), i->indices.end());
        indicesMax += vsize;
        TC.insert(TC.end(), i->texCoords.begin(), i->texCoords.end());
        TBO_prepare(tbo, V, VN, i->indices, i->color, i->reflect, i->light);
    }
    TBOtex_prepare(tbo3, TC);


    ///////////////////////////////////////////light///////////////////////////////////////////////
    std::vector<Light*> ligs;

    PointLight la(glm::vec3(-2.0f, 4.0f, -2.0f));
    ligs.push_back(&la);

    PointLight lb(glm::vec3(2.0f, 4.0f, 2.0f));
    ligs.push_back(&lb);

    SpotLight lc(glm::vec3(1.0, 4.0, 1.0), glm::vec3(-1.0,-2.0,-1.0), 40);
    //ligs.push_back(&lc);

    DirectionalLight ld(glm::vec3(0.1, -1.0, 0.1));
    //ligs.push_back(&ld);

    Arealight le(glm::vec3(-1.2, 3, -1.3), glm::vec3(-1.4, 3.3, 1), glm::vec3(0.2, 3.2, 0.4));
    ligs.push_back(&le);

    for (Light* i : ligs) {
        TBOlight_prepare(tbo2, i->identifier, i->vertices, i->direction, i->I_a, i->I_i);
    }

    
    VBO.update(V);
    NBO.update(VN);
    IndexBuffer.update(T);
    UTBO.update(TC);

    // load PPM image file
    ImageRGB image;
    bool imageAvailable = loadPPM(image, "../data/land_shallow_topo_2048.ppm");

#else
    // load  OFF file
    glm::vec3 min, max, tmpVec;
    std::cout << "Loading OFF file...";
    loadOFFFile("../data/stanford_dragon2.off", V, T, min, max);
    //loadOFFFile("../data/bunny.off", V, T, min, max);
    std::cout << " done! " << V.size() << " vertices, " << T.size() << " triangles" << std::endl;
    VBO.update(V);
    IndexBuffer.update(T);

    // compute model matrix so that the mesh is inside a -1..1 cube
    tmpVec = max - min;
    float maxVal = glm::max(tmpVec.x, glm::max(tmpVec.y, tmpVec.z));
    tmpVec /= 2.0f;
    modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f / maxVal));
    modelMatrix *= glm::translate(glm::mat4(1.0f), -(min + tmpVec));
    
    // compute face normals
    std::cout << "Computing face normals...";
    std::vector<glm::vec3> faceN(3);
    faceN.resize(T.size());
    for (unsigned int i = 0; i < faceN.size(); i++) {
        faceN[i] = glm::normalize(glm::cross(V[T[i].y] - V[T[i].x], V[T[i].z] - V[T[i].x]));
    }
    std::cout << " done!" << std::endl;
    // compute vertex normals
    std::cout << "Computing vertex normals...";
    VN.resize(V.size());
    for (unsigned int i = 0; i < VN.size(); i++) {
        VN[i] = glm::vec3(0.0f);
    }
    for (unsigned int j = 0; j < T.size(); j++) {
        VN[T[j].x] += faceN[j];
        VN[T[j].y] += faceN[j];
        VN[T[j].z] += faceN[j];
    }
    for (unsigned int i = 0; i < VN.size(); i++) {
        VN[i] = glm::normalize(VN[i]);
    }
    std::cout << " done!" << std::endl;
    // initialize normal array buffer
    NBO.init();
    NBO.update(VN);
#endif

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    // load fragment shader file 
    std::ifstream fragShaderFile("../shader/fragment.glsl");
    std::stringstream fragCode;
    fragCode << fragShaderFile.rdbuf();
    // load vertex shader file
    std::ifstream vertShaderFile("../shader/vertex.glsl");
    std::stringstream vertCode;
    vertCode << vertShaderFile.rdbuf();
    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program.init(vertCode.str(), fragCode.str(), "outColor");
    program.bind();

    // The vertex shader wants the position of the vertices as an input.
    // The following line connects the VBO we defined above with the position "slot"
    // in the vertex shader
    

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Register the cursor move callback
    glfwSetCursorPosCallback(window, cursor_position_callback);

    // Update viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // camera setup
    cameraPos = glm::vec3(0.0f, 0.0f, camRadius);
    cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    cameraDirection = glm::normalize(cameraPos - cameraTarget);
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    cameraRight = glm::normalize(glm::cross(cameraUp, cameraDirection));

    // TBO
    // tbo: pos1, pos2, pos3, nor1, nor2, nor3, col, (is_reflecting, is_lighting, 0);
    // for test
    unsigned int TBO_tex;
    GLuint TBO;
    glGenBuffers(1, &TBO);
    glBindBuffer(GL_TEXTURE_BUFFER, TBO);
    
    //TBO_prepare(tbo, V, VN, T, glm::vec3(1,1,0));


    glBufferData(GL_TEXTURE_BUFFER, tbo.size()*sizeof(float), &tbo[0], GL_STATIC_DRAW);
    

    glGenTextures(1, &TBO_tex);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);


    glUniform1i(glGetUniformLocation(program.program_shader, "tria"), 0);
    glUniform1i(glGetUniformLocation(program.program_shader, "tbo_size"), T.size());

    //
    // tbo2: id, pos1, pos2, pos3, pos4, dir
    // for test
    unsigned int TBO_tex2;
    GLuint TBO2;
    glGenBuffers(1, &TBO2);
    glBindBuffer(GL_TEXTURE_BUFFER, TBO2);

    glBufferData(GL_TEXTURE_BUFFER, tbo2.size() * sizeof(float), &tbo2[0], GL_STATIC_DRAW);


    glGenTextures(1, &TBO_tex2);
    glBindBuffer(GL_TEXTURE_BUFFER, 1);


    glUniform1i(glGetUniformLocation(program.program_shader, "tria2"), 1);
    glUniform1i(glGetUniformLocation(program.program_shader, "tbo_size2"), ligs.size());


    // utbo
    unsigned int ut;
    glGenTextures(1, &ut);
    glBindBuffer(GL_TEXTURE_2D, 2);
    glBindTexture(GL_TEXTURE_2D, ut);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.w, image.h, 0, GL_RGB, GL_UNSIGNED_BYTE, &image.data[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    glUniform1i(program.uniform("tex"), 2);

    //
    std::cout<<tbo.size()<<std::endl;

    program.bindVertexAttribArray("position", VBO);
    program.bindVertexAttribArray("normal", NBO);
    program.bindVertexAttribArray("texCoords", UTBO);




    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Get the size of the window
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // matrix calculations
        switch (mode)
        {
        case 0:
            viewMatrix = glm::lookAt(cameraPos, cameraTarget, cameraUp);
            break;
        case 1:
            viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);
            break;
        default:
            viewMatrix = glm::lookAt(cameraPos, cameraTarget, cameraUp);
            break;
        }
        projMatrix = glm::perspective(glm::radians(35.0f), (float)width / (float)height, 0.1f, 100.0f);

        // Bind your VAO (not necessary if you have only one)
        VAO.bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_BUFFER, TBO_tex);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, TBO);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, TBO_tex2);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, TBO2);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, ut);


        // bind your element array
        IndexBuffer.bind();

        // Bind your program
        program.bind();

        // Set the uniform values
        glUniform3f(program.uniform("triangleColor"), 0.79f, 0.75f, 0.9f);
        glUniform3f(program.uniform("camPos"), cameraPos.x, cameraPos.y, cameraPos.z);
        glUniformMatrix4fv(program.uniform("modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(program.uniform("viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(program.uniform("projMatrix"), 1, GL_FALSE, glm::value_ptr(projMatrix));
        // direction towards the light
        glUniform3fv(program.uniform("lightPos"), 1, glm::value_ptr(glm::vec3(-1.0f, 2.0f, 3.0f)));
        // x: ambient; 
        glUniform3f(program.uniform("lightParams"), 0.1f, 50.0f, 0.0f);

        // Clear the framebuffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Enable depth test
        glEnable(GL_DEPTH_TEST);
        
        // Draw a triangle
        //glDrawArrays(GL_TRIANGLES, 0, V.size());
        glDrawElements(GL_TRIANGLES, T.size() * 3, GL_UNSIGNED_INT, 0);
        //glDrawElements(GL_LINES, T.size() * 3, GL_UNSIGNED_INT, 0);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    program.free();
    VAO.free();
    VBO.free();
    UTBO.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
