#ifndef world_h
#define world_h

#include "chunk.h"
#include <vector>
#include <glm/glm.hpp>

// For simplified querying of the world
enum ORIENTATION {
    NORTH,
    SOUTH,
    EAST,
    WEST
};

// A world
class world {
public:
    // constructor.
    world(int pos_x, int pos_y, int width, int height, int VD,  OpenSimplexNoise::Noise* n) {
        noise = n;
        posX = pos_x;
        posY = pos_y;
        cellWidth = width;
        cellHeight = height;
        vd = VD;
        nose = n;
        loadChunks();
    }
    
    // Returns true if position is valid for the currently selected chunk.
    bool isValid(glm::vec3 pos) {
        return cChunk->isValid(pos);
    }
    
    // Player moved to another chunk. Shift current chunk accordingly.
    void updatePos(ORIENTATION dir) {
        switch (dir) {
            case NORTH:
                posY += 1;
                break;
            case SOUTH:
                posY -= 1;
                break;
            case EAST:
                posX += 1;
                break;
            case WEST:
                posX -= 1;
                break;
            default:
                break;
        }
        loadChunks();
    }
    
    // Interpolate from current chunk
    double interpolateHeight(float x, float y) {
        return cChunk->interpolateHeight(x,y);
    }
    
    // Load chunks if they exist, generate them if thye dont.
    void loadChunks() {
        dXs.clear();
        dYs.clear();
        dWorlds.clear();
        
        if (vd >= 50) {
            std::cout << "no" << std::endl;
            exit(-1);
        }
        
        bool woah[100][100];
        for (int i = 0; i < 2*vd + 1; ++i)
            for (int j = 0; j < 2*vd + 1; ++j)
                woah[i][j] = false;
        
        for (int i = 0; i < Xs.size(); ++i) {
            if (Xs[i]-posX < 2*vd+1 && Xs[i]-posX >= 0 && Ys[i]-posY < 2*vd+1 && Ys[i]-posY >= 0) {
                std::cout << "Loading chunk (" << Xs[i] <<","<<Ys[i]<<")"<<std::endl;
                dXs.push_back(Xs[i]);
                dYs.push_back(Ys[i]);
                dWorlds.push_back(worlds[i]);
                woah[Xs[i]-posX][Ys[i]-posY] = true;
            }
        }
        
        for (int i = 0; i < 2*vd + 1; ++i)
            for (int j = 0; j < 2*vd + 1; ++j)
                if (!woah[i][j]) {
                    std::cout << "Generating chunk (" << posX + i <<","<< posY + j <<")"<<std::endl;
                    Chunk temp = Chunk(cellWidth, cellHeight, posX+i, posY+j, largeAssets, smallAssets, nose);
                    Xs.push_back(i+posX);
                    Ys.push_back(j+posY);
                    worlds.push_back(temp);
                    dXs.push_back(i+posX);
                    dYs.push_back(j+posY);
                    dWorlds.push_back(temp);
                }
        
        for (int i = 0; i < Xs.size(); ++i)
            if (Xs[i] - posX == vd && Ys[i] - posY == vd)
                cChunk = &worlds[i];
        cChunk->print();
        std::cout << "Total chunks:" << worlds.size() << " chunks." << std::endl;
        std::cout << "Displayed chunks: " << dWorlds.size() << " chunks."<< std::endl;
    }
    
    // Render all loaded chunks.
    void renderChunks(Shader shader, int l) {
        glm::mat4 model = glm::mat4(1.0f);
        for (int i = 0; i < dXs.size(); ++i) {
            model = glm::translate(glm::mat4(1.0f), glm::vec3(cellWidth*(dXs[i]-posX)-vd*cellWidth, 0.0f, cellHeight*(dYs[i]-posY)-vd*cellHeight));
            dWorlds[i].render(shader, model, woodTexture, l);
        }
    }
    
private:
    int posX;
    int posY;
    int cellWidth;
    int cellHeight;
    int vd;
    Chunk* cChunk;
    OpenSimplexNoise::Noise* nose;
    vector <Model> largeAssets = {
        Model("assets/models/trees/oak/oak.obj"),
        Model("assets/models/trees/poplar/poplar.obj"),
        Model("assets/models/trees/pine/pine.obj"),
        Model("assets/models/trees/plum/plum.obj"),
        Model("assets/models/trees/maple/maple.obj"),
        Model("assets/models/trees/ash/ash.obj")
    };
    vector <Model> smallAssets = {
        Model("assets/models/small/stump/stump.obj"),
        Model("assets/models/small/rock/rock.obj")
    };
    unsigned int woodTexture = loadTexture("assets/textures/surfaces/grass.jpg");
    OpenSimplexNoise::Noise* noise;
    std::vector<Chunk> worlds;
    std::vector<Chunk> dWorlds;
    std::vector<int> Xs;
    std::vector<int> dXs;
    std::vector<int> Ys;
    std::vector<int> dYs;
    
    int abs (int x) {
        return x >= 0 ? x : - x;
    }
};

#endif
