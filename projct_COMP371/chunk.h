#ifndef chunk_h
#define chunk_h

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <glm/glm.hpp>
#include "draw.h"
#include "Model.h"

#include "OpenSimplexNoise.h"

#define MAXSIZE_X 128
#define MAXSIZE_Y 128

class Chunk {
public:
    unsigned int terrainVAO = 0;
    float density = 0.4;
    
    // Verify if, for all items in the chunk, the given position is inside
    // any of them. Return false if this is the case, otherwise default to
    // true.
    bool isValid(glm::vec3 pos) {
        for (int i = 0; i < largePOS.size(); ++i) {
            pos.y = largePOS[i].y;
            if (glm::distance(pos, largePOS[i]) < 1.0f) {
                std::cout << "collided" << std::endl;
                return false;
            }
        }
        for (int i = 0; i < smallPOS.size(); ++i) {
            pos.y = smallPOS[i].y;
            if (glm::distance(pos, smallPOS[i]) < 1.0f) {
                std::cout << "collided" << std::endl;
                return false;
            }
        }
        return true;
    }
    
    // Wew. Generate a Chunk.
    Chunk(int w, int h, int x, int y, vector<Model> m, vector<Model> s, OpenSimplexNoise::Noise* n) {
        // Width and height must be 1 more. Prevents gap from chunks.
        width = w+1;
        height = h+1;
        // We need to know the offset so that the simplex map can be probed properly.
        // an offset of x in a grid is an offset of width*offsetX + x in the real world.
        offsetX = x;
        offsetY = y;
        largeOBJ = m;
        smallOBJ = s;
        simpleNoise = n;
        // This holds the mesh for the terrain.
        terrainVAO = 0;
        
        // Debugging
        std::cout << "initializing chunk with dimensions " <<w<<"x"<<h<<" at position ("<<x<<"," <<y<<")" <<std::endl;
        
        // Create the chunk's heightmap. Values are taken as a linear function of
        // the simplex noise map.
        heightMap = new double[(MAXSIZE_X + 1)*(MAXSIZE_Y + 1)];
        for (int i = 0; i < width; ++i)
            for (int j = 0; j < height; ++j) {
                setHeight(i, j, eval(i,j));
            }
        
        // Depending on density, will append items to be drawn on the map.
        for (int i = 0; i < width*height*density*density; ++i) {
            float xpos = 1.0*rand()/RAND_MAX*width;
            float ypos = 1.0*rand()/RAND_MAX*height;
            float  val = 1.0*rand()/RAND_MAX;
            float  lim = simpleNoise->eval(xpos, ypos);
            if (val >= 2*(lim+1))
                largePOS.push_back(glm::vec3(xpos, lim, ypos));
            xpos = 1.0*rand()/RAND_MAX*width;
            ypos = 1.0*rand()/RAND_MAX*height;
            val = 1.0*rand()/RAND_MAX;
            lim = simpleNoise->eval(xpos, ypos);
            if (val >= 2*(lim+1))
                smallPOS.push_back(glm::vec3(xpos, lim, ypos));
        }
        // Debug msgs are nice. i like debug messages.
        std::cout << "Planted " << largePOS.size() << " trees." << std::endl;
        std::cout << "Placed " << smallPOS.size() << " things." << std::endl;
    }
    // Debug msgs are mean. I hate debug messages.
    void print() {
        std::cout << "Current CHUNK: (" << offsetX << "," <<offsetY << ")" << std::endl;
    }
    // Get the height from x,y value
    double getHeight(int x, int y) {
        return heightMap[y*width + x];
    }
    // This uses linear interpolation using 4 points rather than the triangle
    // This will not work if the terrain is too steep. In which case,
    // use 3 points and determine the plane coordinate. Google it.
    //
    // We use linear interpolation because it is faster.
    double interpolateHeight(float x, float y) {
        double x1 = glm::floor(x);
        double y1 = glm::floor(y);
        double x2 = glm::ceil(x);
        double y2 = glm::ceil(y);
        
        double f1 = (x2 - x) * eval(x1, y1) + (x - x1)*eval(x2,y1);
        double f2 = (x2 - x) * eval(x1, y2) + (x - x1)*eval(x2,y2);
        return (y2 - y)*f1 + (y - y1)*f2;
    }
    
    // Render the chunk
    void render(Shader shader, glm::mat4 trans, unsigned int ft, int l) {
        floorTexture = ft;
        // If the VAO for the terrain mesh is already set up, skip this.
        if (terrainVAO == 0) {
            std::cout << "d00d" << std::endl;
            glGenVertexArrays(1, &terrainVAO);
            
            unsigned int vbo, ebo;
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &ebo);
            
            std::vector<glm::vec3> positions;
            std::vector<glm::vec2> uv;
            std::vector<glm::vec3> normals;
            std::vector<unsigned int> indices;
            
            for (int i = 0; i < height; ++i)
                for (int j = 0; j < width; ++j) {
                    float zPos = 1.0f*i;
                    float yPos = 1.0f*getHeight(j,i);
                    float xPos = 1.0f*j;
                    
                    positions.push_back(glm::vec3(xPos, yPos, zPos));
                    uv.push_back(glm::vec2(i%2, j%2));
 
                    normals.push_back(glm::vec3(0.0,1.0,0.0));
                }
            
            for (int i = height-1; i > 0; --i)
                for (int j = 0; j < width; ++j) {
                    indices.push_back(j + width * i);
                    indices.push_back(j + width * (i-1));
                }
            
            indexCount = static_cast<unsigned int>(indices.size());
            std::vector<float> data;
            for (unsigned int i = 0; i < positions.size(); ++i) {
                data.push_back(positions[i].x);
                data.push_back(positions[i].y);
                data.push_back(positions[i].z);
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
            glBindVertexArray(terrainVAO);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);
            unsigned int stride = (3 + 2 + 3) * sizeof(float);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
            
        }
        // Initialized. Now draw.
        glBindVertexArray(terrainVAO);
        
        // For all strips, draw all triangles.
        int numStrips = (height-1);
        int numTrisPerStrip = (width)*2-2;
        shader.setMat4("model", trans);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glActiveTexture(GL_TEXTURE1);
        for(unsigned strip = 0; strip < numStrips; strip++)
                {
                     glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
                                   numTrisPerStrip+2,   // number of indices to render
                                   GL_UNSIGNED_INT,     // index data type
                                   (void*)(sizeof(unsigned) * (numTrisPerStrip+2) * strip)); // offset to starting index
                }
        
        // Draw the chunk at the right space relative to the camera.
        //glm::mat4 model = glm::translate(trans, glm::vec3(width/2.0, eval(width/2.0,height/2.0)+3.0, height/2.0));
        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("model", model);
        for (int i = 0; i < largePOS.size(); ++i) {
            model = glm::scale(model, glm::vec3(simpleNoise->eval(largePOS[i].x,largePOS[i].z)));
            glm::mat4 model = glm::translate(trans, glm::vec3(largePOS[i].x, eval(largePOS[i].x, largePOS[i].z), largePOS[i].z));
            shader.setMat4("model", model);
            largeOBJ[i%largeOBJ.size()].Draw(shader);
        }
        if (l == 1) {
            for (int i = 0; i < smallPOS.size(); ++i) {
                glm::mat4 model = glm::translate(trans, glm::vec3(smallPOS[i].x, eval(smallPOS[i].x, smallPOS[i].z), smallPOS[i].z));
                shader.setMat4("model", model*glm::rotate(glm::mat4(1.0f), 7.0f*smallPOS[i].y, glm::vec3(0.0f, 1.0f, 0.0)));
                smallOBJ[i%smallOBJ.size()].Draw(shader);
            }
        }

    }
    
    // Linear function of the simple noise map. For probbing terrain height.
    double eval(int i, int j) {
        return scaleY*(simpleNoise->eval(scaleX*(offsetX*width + i - offsetX), scaleX*(offsetY*height + j -offsetY)));
    }

private:
    int width;
    int height;
    int offsetX;
    int offsetY;
    int res;
    
    unsigned int indexCount;
    unsigned int nStrips;
    unsigned int tPerStrip;
    double scaleY = 32.0;
    double scaleX = 0.0125;
    
    double* heightMap;
    
    unsigned int floorTexture;
    
    std::vector<Model> largeOBJ;
    std::vector<glm::vec3> largePOS;
    std::vector<Model> smallOBJ;
    std::vector<glm::vec3> smallPOS;
    
    
    OpenSimplexNoise::Noise* simpleNoise;
    void setHeight(int x, int y, double z) {
        heightMap[y*width + x] = z;
    }
};


#endif
