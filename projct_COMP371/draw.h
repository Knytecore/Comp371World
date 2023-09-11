// Methods to load models and textures.

#ifndef draw_h
#define draw_h

#include "Model.h"
#include "shader.h"

unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string> faces);

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4) {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int chunkborderVAO, chunkborderVBO;
void RenderBorder(int width, int height, Shader shader) {
    if (chunkborderVAO == 0){
        glm::vec3 chunkBorder[] = {
            glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(-0.5f,-0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(-0.5f, 0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(-0.5f, 0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3( 0.5f, 0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3( 0.5f,-0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3( 0.5f, 0.5f,-0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3( 0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(-0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3( 0.5f,-0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3( 0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3( 0.5f, 0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3( 0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3( 0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3( 0.5f,-0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3( 0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3( 0.5f,-0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(-0.5f, 0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3( 0.5f, 0.5f,-0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3( 0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
        };
        glGenVertexArrays(1, &chunkborderVAO);
        glGenBuffers(1, &chunkborderVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, chunkborderVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(chunkBorder), chunkBorder, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(chunkborderVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    glBindVertexArray(chunkborderVAO);
    shader.use();
    shader.setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(1.0f*width, 100.0f, 1.0f*height)));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -0.5f, 0.0f, 0.0f, // bottom-left
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -0.5f, 2.5f, 2.5f, // top-right
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -0.5f, 2.5f, 0.0f, // bottom-right
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -0.5f, 2.5f, 2.5f, // top-right
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -0.5f, 0.0f, 0.0f, // bottom-left
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -0.5f, 0.0f, 2.5f, // top-left
            // front face
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  0.5f, 0.0f, 0.0f, // bottom-left
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  0.5f, 2.5f, 0.0f, // bottom-right
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  0.5f, 2.5f, 2.5f, // top-right
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  0.5f, 2.5f, 2.5f, // top-right
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  0.5f, 0.0f, 2.5f, // top-left
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  0.5f, 0.0f, 0.0f, // bottom-left
            // left face
            -0.5f,  0.5f,  0.5f, -0.5f,  0.0f,  0.0f, 2.5f, 0.0f, // top-right
            -0.5f,  0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 2.5f, 2.5f, // top-left
            -0.5f, -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 0.0f, 2.5f, // bottom-left
            -0.5f, -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 0.0f, 2.5f, // bottom-left
            -0.5f, -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -0.5f,  0.5f,  0.5f, -0.5f,  0.0f,  0.0f, 2.5f, 0.0f, // top-right
            // right face
             0.5f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 2.5f, 0.0f, // top-left
             0.5f, -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 0.0f, 2.5f, // bottom-right
             0.5f,  0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 2.5f, 2.5f, // top-right
             0.5f, -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 0.0f, 2.5f, // bottom-right
             0.5f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 2.5f, 0.0f, // top-left
             0.5f, -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -0.5f, -0.5f, -0.5f,  0.0f, -0.5f,  0.0f, 0.0f, 2.5f, // top-right
             0.5f, -0.5f, -0.5f,  0.0f, -0.5f,  0.0f, 2.5f, 2.5f, // top-left
             0.5f, -0.5f,  0.5f,  0.0f, -0.5f,  0.0f, 2.5f, 0.0f, // bottom-left
             0.5f, -0.5f,  0.5f,  0.0f, -0.5f,  0.0f, 2.5f, 0.0f, // bottom-left
            -0.5f, -0.5f,  0.5f,  0.0f, -0.5f,  0.0f, 0.0f, 0.0f, // bottom-right
            -0.5f, -0.5f, -0.5f,  0.0f, -0.5f,  0.0f, 0.0f, 2.5f, // top-right
            // top face
            -0.5f,  0.5f, -0.5f,  0.0f,  0.5f,  0.0f, 0.0f, 2.5f, // top-left
             0.5f,  0.5f , 0.5f,  0.0f,  0.5f,  0.0f, 2.5f, 0.0f, // bottom-right
             0.5f,  0.5f, -0.5f,  0.0f,  0.5f,  0.0f, 2.5f, 2.5f, // top-right
             0.5f,  0.5f,  0.5f,  0.0f,  0.5f,  0.0f, 2.5f, 0.0f, // bottom-right
            -0.5f,  0.5f, -0.5f,  0.0f,  0.5f,  0.0f, 0.0f, 2.5f, // top-left
            -0.5f,  0.5f,  0.5f,  0.0f,  0.5f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 24);
    glBindVertexArray(0);
}

#endif
