//
// LICENSE:
//
// Copyright (c) 2018 Andrei Stefan Bejgu
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//


#include "../yocto/yocto_gl.h"

using namespace ygl;
using namespace std;

#include <algorithm>
#include <iostream>
#include <stack>


// Generates random numbers in interval
// Method took from: [STACK OVERFLOW = https://stackoverflow.com/questions/7560114/random-number-c-in-some-range]
// Author: Nawaz
int random(int min, int max) //range : [min, max)
{
    static bool first = true;
    if (first) {
        srand(time(NULL)); //seeding for the first time only!
        first = false;
    }
    return min + rand() % ((max + 1) - min);
}

//Rappresents a collection of models of some type
struct model {
    //all possible models
    map<int, vector<string>> possibilities;

    map<int, vec3f> height;


    //the height of actual model

    model(int type, string filename, vec3f height) {
        vector<string> possibile = vector<string>();
        possibile.push_back(filename);
        this->possibilities.insert(pair<int, vector<string>>(type, possibile));
        this->height[type] = height;
    }

    //adds another model to the actual collection
    void addPossibility(int type, string filename) {
        vector<string> possibile = vector<string>();
        if (possibilities.find(type) != possibilities.end()) {
            possibile = possibilities.at(type);
        } else
            this->possibilities.insert(pair<int, vector<string>>(type, possibile));

        if (height.find(type) == height.end()) {
            height[type] = height[0];
        }
        possibile.push_back(filename);
        possibilities[type] = possibile;
    }

    void setHeight(int type, vec3f height) {
        this->height[type] = height;
    }


    vec3f getHeight(int type) {
        return height[type];
    }

    //Get random model from all possibles of some type
    string getRandomPossibility(int type) {

        vector<string> possibile = possibilities.at(type == -1 ? 0 : type);
        //cout << possibile.size() << "\n";
        return possibile.at(random(0, possibile.size() - 1));
    }

    //Get random model from all possibles
    string getRandomPossibility() {

        return getRandomPossibility(-1);
    }
};

//loads the models filenames
map<string, model *> loadModels() {
    string prefixBuilding = "in/modularBuildings_";
    string prefixRoads = "in/roadTile_";

    string suffix = ".obj";

    auto voidOrigin = vec3f{0, 0, 0};

    auto baseHeight = vec3f{0, 0.83f, 0};

    auto blockHeight = vec3f{0, 0.2f, 0};

    auto curtainHeight = vec3f{0, 0.2f, 0};

    auto floorHeight = vec3f{0, 0.62f, 0};

    map<string, model *> modelsMap;

    auto *base = new model{0, prefixBuilding + "026a" + suffix, baseHeight};
    base->addPossibility(0, prefixBuilding + "025a" + suffix);
    base->addPossibility(0, prefixBuilding + "025b" + suffix);
    base->addPossibility(1, prefixBuilding + "038" + suffix);
    base->addPossibility(1, prefixBuilding + "038a" + suffix);
    base->addPossibility(1, prefixBuilding + "038b" + suffix);
    base->addPossibility(2, prefixBuilding + "022" + suffix);

    base->setHeight(1, floorHeight);
    base->setHeight(2, floorHeight);

    modelsMap["base"] = base;

    auto *block = new model{0, prefixBuilding + "005" + suffix, blockHeight};
    block->addPossibility(1, prefixBuilding + "017" + suffix);


    modelsMap["block"] = block;


    auto *curtain = new model{0, prefixBuilding + "010" + suffix, curtainHeight};
    curtain->addPossibility(0, prefixBuilding + "012" + suffix);
    modelsMap["curtain"] = curtain;

    auto *floor = new model{0, prefixBuilding + "021" + suffix, floorHeight};
    floor->addPossibility(0, prefixBuilding + "034" + suffix);
    floor->addPossibility(0, prefixBuilding + "041" + suffix);
    floor->addPossibility(0, prefixBuilding + "033" + suffix);
    floor->addPossibility(0, prefixBuilding + "035" + suffix);
    floor->addPossibility(0, prefixBuilding + "047" + suffix);
    floor->addPossibility(0, prefixBuilding + "048" + suffix);
    floor->addPossibility(0, prefixBuilding + "030" + suffix);
    floor->addPossibility(0, prefixBuilding + "029" + suffix);
    //floor->addPossibility(0, prefixBuilding + "042" + suffix);
    floor->addPossibility(1, prefixBuilding + "039" + suffix);
    floor->addPossibility(1, prefixBuilding + "049" + suffix);
    floor->addPossibility(1, prefixBuilding + "050" + suffix);

    floor->addPossibility(2, prefixBuilding + "037" + suffix);

    modelsMap["floor"] = floor;

    auto *roof = new model{0, prefixBuilding + "063" + suffix, voidOrigin};
    roof->addPossibility(0, prefixBuilding + "064" + suffix);
    roof->addPossibility(0, prefixBuilding + "032" + suffix);
    roof->addPossibility(0, prefixBuilding + "051" + suffix);
    roof->addPossibility(0, prefixBuilding + "011" + suffix);
    roof->addPossibility(1, prefixBuilding + "045" + suffix);
    roof->addPossibility(2, prefixBuilding + "0082" + suffix);
    roof->addPossibility(2, prefixBuilding + "009" + suffix);
    roof->addPossibility(2, prefixBuilding + "016" + suffix);

    modelsMap["roof"] = roof;

    auto *road = new model{0, prefixRoads + "162" + suffix, voidOrigin};
    road->addPossibility(0, prefixRoads + "025" + suffix);
    modelsMap["road"] = road;

    auto *crossing = new model{0, prefixRoads + "141" + suffix, voidOrigin};
    crossing->addPossibility(0, prefixRoads + "1501" + suffix);
    crossing->addPossibility(0, prefixRoads + "1502" + suffix);
    crossing->addPossibility(0, prefixRoads + "150" + suffix);
    crossing->addPossibility(0, prefixRoads + "1501" + suffix);
    crossing->addPossibility(0, prefixRoads + "1502" + suffix);
    crossing->addPossibility(0, prefixRoads + "141" + suffix);
    crossing->addPossibility(0, prefixRoads + "038" + suffix);
    crossing->addPossibility(0, prefixRoads + "0382" + suffix);
    modelsMap["crossing"] = crossing;

    modelsMap["ground"] = new model{0, prefixRoads + "163" + suffix, voidOrigin};

    auto *tree = new model{0, prefixRoads + "019" + suffix, voidOrigin};
    tree->addPossibility(0, prefixRoads + "020" + suffix);
    modelsMap["tree"] = tree;

    return modelsMap;
};

//adds an object to the scene
frame3f add_obj(scene *scn, string filename, string name, frame3f frame) {
    auto *obj1 = load_obj(filename); //Load Object_scene from filename
    map<string, material *> materialMap;
    map<string, obj_object *> objectsMap;
    for (auto *mat : obj1->materials) {
        auto matn = new material{mat->name};
        matn->kd = mat->kd;
        matn->ke = mat->ke;
        scn->materials.push_back(matn);
        materialMap[mat->name] = matn;
    }

    for (auto *obj : obj1->objects) {

        int count = 0;
        for (auto shpe : get_mesh(obj1, *obj, false)->shapes) {

            auto shp = new shape{name + to_string(count++)};

            //Creating new shape from the loaded one
            shp->mat = materialMap[shpe.matname];
            shp->triangles = shpe.triangles;
            shp->points = shpe.points;
            shp->lines = shpe.lines;
            shp->pos = shpe.pos;
            shp->norm = shpe.norm;
            shp->texcoord = shpe.texcoord;
            shp->quads = shpe.tetras;
            shp->color = shpe.color;

            scn->shapes.push_back(shp);
            scn->instances.push_back(new instance{shp->name, frame, shp});

        }
    }
    return frame;


}

//Creates a frame in position (xi,yi) and rotates it with the desired angle moving the object to make it be same origin as the GLOBAL system of this generator
frame3f rotateFrame(float angle, int xi, int yi) {
    auto newAngle = angle * pif / 180.0f;
    frame3f frame = rotation_frame3f({0, 1, 0}, newAngle);

    frame.o = vec3f{0, 0, 1} * yi + vec3f{1, 0, 0} * xi;

    if (angle == 90.0f)
        frame.o += vec3f{1, 0, 0} * 1.0f;
    else if (angle == 180.0f) {
        frame.o += vec3f{0, 0, 1} * -1.0f;
        frame.o += vec3f{1, 0, 0} * 1.0f;
    } else if (angle == 270.0f)
        frame.o += vec3f{0, 0, 1} * -1.0f;
    return frame;
}


//Recursively creates a building adding for each floor a model and adding the roof when maxFloorNumber is reached
//and can create different type of buildings putting together only the pieces that fit for that type
int recursiveCreateBuilding(scene *scn, int type, string prefix, int maxFloors, int actualFloor, frame3f frame,
                            vec3f height, map<string, model *> modelsMap, int roofType, map<int, float> rotations,
                            int roofRotation, int xi, int yi, bool noBase, int floorType) {


    if (maxFloors == actualFloor) {

        auto *m = modelsMap["roof"];

        string filename;
        if (roofType == -2) return 0;
        else if (roofType != -1) {
            frame3f newf = rotateFrame(rotations[roofRotation], xi, yi);

            newf.o.y = frame.o.y;
            frame = newf;
            filename = m->possibilities.at(2).at(roofType);
        } else {
            filename = m->getRandomPossibility(type);
        }
        add_obj(scn, filename, prefix + to_string(actualFloor),
                {frame.x, frame.y, frame.z, frame.o + m->getHeight(type) + height});
        return 0;
    }
    frame3f newFrame;

    string filename;
    model *model;

    string modelFilename;
    vec3f newHeight;
    bool newH = false;
    if (actualFloor == 0 && noBase) {
        model = modelsMap["block"];
        modelFilename = model->getRandomPossibility(0);

    } else if (actualFloor == 0 || noBase && actualFloor == 1) {
        model = modelsMap["base"];
        if (noBase) {

            if (floorType == -1)
                modelFilename = modelsMap["floor"]->getRandomPossibility(0);
            else
                modelFilename = modelsMap["floor"]->possibilities.at(0).at(floorType);

            newH = true;
            newHeight = model->getHeight(2);

        } else {
            vector<string> possib = model->possibilities.at(0);
            modelFilename = possib.at(random(maxFloors > 2 ? 0 : 1, possib.size() - 1));
        }

    } else if (actualFloor == 3 && type == 1) {
        model = modelsMap["floor"];
        modelFilename = model->getRandomPossibility(2);


    } else if ((actualFloor % 2 == 0 || actualFloor == 1 && !noBase) && type == 1) {
        model = modelsMap["block"];
        modelFilename = actualFloor == 2 ? model->getRandomPossibility(0) : model->getRandomPossibility(type);

    } else if (type == 0 && ((actualFloor == 1 || noBase && actualFloor == 2 ||
                              (actualFloor > 1 &&
                               (noBase && actualFloor % 2 == 0 || (!noBase && actualFloor % 2 == 1)))))) {

        if (random(0, 100) > 80 && type == 0 && (actualFloor == 1 || noBase && actualFloor == 2)) {
            model = modelsMap["curtain"];
            modelFilename = model->getRandomPossibility();
        } else {
            model = modelsMap["block"];
            modelFilename = model->getRandomPossibility(type);
        }

    } else {
        model = modelsMap["floor"];
        if (floorType == -1)
            modelFilename = modelsMap["floor"]->getRandomPossibility(type);
        else
            modelFilename = modelsMap["floor"]->possibilities.at(type).at(floorType);
    }

    newFrame = add_obj(scn, modelFilename, prefix + to_string(actualFloor) + to_string(type),
                       {frame.x, frame.y, frame.z, frame.o + height});
    if (!newH) height = model->getHeight(type);
    else height = newHeight;

    recursiveCreateBuilding(scn, type, prefix, maxFloors, ++actualFloor, newFrame, height, modelsMap, roofType,
                            rotations, roofRotation, xi, yi, noBase, floorType);

}

//METHOD OVERLOADING
int recursiveCreateBuilding(scene *scn, int type, string prefix, int maxFloors, int actualFloor, frame3f frame,
                            vec3f height, map<string, model *> modelsMap, int floorType) {
    recursiveCreateBuilding(scn, type, prefix, maxFloors, actualFloor, frame, height, modelsMap, -1, {}, -1,
                            -1, -1, false, floorType);

}

//Utility method used to check the chance
bool forkStreet(int forkChance) {
    return random(0, 100) < forkChance;
}

// Generates the roads in the matrix
// starts from actual postion (x,y) and adds street to the matrix in direction -> dir
// dir == 0 RIGHT; dir == 1 LEFT; dir == 2 UP; dir == 3 DOWN;
// the road will fork in actual position with forkChance probability if it forks this method will be called recursivelly and the chance will be HALVED
// in this way the streets will be created using the probability
//all dagerous cases will stop the road for exampple if there is near actual position(x,y) another road of the same direction in this way we will avoid a map with only roads if the forkProbability is high
int generateRoads(int *matrix, int x, int y, int n, int m, int dir, int forkChance, bool first, bool justFork) {
    if (x <= n && y <= m && x >= 0 && y >= 0 && (matrix[x * n + y] == 5)) {
        vector<int> possibleDirections;

        auto doubleForkChance = 30; //chance of double forking
        auto count = 0;
        auto forked = false;

        if (dir == 0) {
            possibleDirections.push_back(2);
            possibleDirections.push_back(3);
            auto steps = first ? m : random(y + (m - y) / 2, m);
            first = false;

            for (auto yi = y; yi < steps; yi++) {
                if (((x + 1 < n && matrix[(x + 1) * n + yi] == 0) || (x - 1 >= 0 && (matrix[(x - 1) * n + yi] == 0)))) {
                    break;
                }
                matrix[x * n + yi] = 0;
                auto forkedBool = false;
                if (forkStreet(forkChance) && !justFork && !forked && count++ > 0 && yi != steps - 1) {

                    forked = true;
                    forkedBool = true;
                    if (random(0, 100) < doubleForkChance) // fork in all directions
                    {
                        generateRoads(matrix, x + 1, yi, n, m, 2, forkChance / 2, first, true);
                        generateRoads(matrix, x - 1, yi, n, m, 3, forkChance / 2, first, true);
                    } else {
                        int newdir = possibleDirections[random(0, possibleDirections.size())];
                        if (newdir == 2)
                            generateRoads(matrix, x + 1, yi, n, m, newdir, forkChance / 2, first, true);
                        else if (newdir == 3)
                            generateRoads(matrix, x - 1, yi, n, m, newdir, forkChance / 2, first, true);
                    }
                }
                justFork = false;
                if (!forkedBool) forked = false;
            }
        } else if (dir == 1) {
            possibleDirections.push_back(2);
            possibleDirections.push_back(3);

            auto steps = random(0, y / 2);

            for (auto yi = y; yi >= steps; yi--) {
                if (((x + 1 < n && matrix[(x + 1) * n + yi] == 0) || (x - 1 >= 0 && (matrix[(x - 1) * n + yi] == 0)))) {
                    break;
                }
                matrix[x * n + yi] = 0;
                auto forkedBool = false;

                if (forkStreet(forkChance) && !justFork && !forked && count++ > 0 && yi != steps) {
                    forked = true;
                    forkedBool = true;
                    if (random(0, 100) < doubleForkChance) // fork in all directions
                    {
                        generateRoads(matrix, x + 1, yi, n, m, 2, forkChance / 2, first, true);
                        generateRoads(matrix, x - 1, yi, n, m, 3, forkChance / 2, first, true);
                    } else {
                        int newdir = possibleDirections[random(0, possibleDirections.size())];

                        if (newdir == 2)
                            generateRoads(matrix, x + 1, yi, n, m, newdir, forkChance / 2, first, true);
                        else
                            generateRoads(matrix, x - 1, yi, n, m, newdir, forkChance / 2, first, true);
                    }
                }
                justFork = false;
                if (!forkedBool) forked = false;
            }
        } else if (dir == 2) {

            possibleDirections.push_back(0);
            possibleDirections.push_back(1);

            auto steps = random(x + (n - x) / 2, n);

            for (auto xi = x; xi < steps; xi++) {
                if (((y + 1 < m && matrix[(xi) * n + y + 1] == 2) || (y - 1 >= 0 && (matrix[(xi) * n + y - 1] == 2)))) {
                    break;
                }
                matrix[xi * n + y] = 2;
                auto forkedBool = false;
                if (forkStreet(forkChance) && !justFork && !forked && count++ > 0 && xi != steps - 1) {

                    forked = true;
                    forkedBool = true;

                    if (random(0, 100) < doubleForkChance) // fork in all directions
                    {
                        generateRoads(matrix, xi, y + 1, n, m, 0, forkChance / 2, first, true);
                        generateRoads(matrix, xi, y - 1, n, m, 1, forkChance / 2, first, true);
                    } else {
                        int newdir = possibleDirections[random(0, possibleDirections.size())];
                        if (newdir == 0)
                            generateRoads(matrix, xi, y + 1, n, m, newdir, forkChance / 2, first, true);
                        else
                            generateRoads(matrix, xi, y - 1, n, m, newdir, forkChance / 2, first, true);
                    }
                }
                justFork = false;
                if (!forkedBool) forked = false;
            }
        } else {
            possibleDirections.push_back(0);
            possibleDirections.push_back(1);

            auto steps = random(0, x / 2);
            for (auto xi = x; xi >= steps; xi--) {
                if (((y + 1 < m && matrix[(xi) * n + y + 1] == 2) || (y - 1 >= 0 && (matrix[(xi) * n + y - 1] == 2)))) {
                    break;
                }
                matrix[xi * n + y] = 2;
                auto forkedBool = false;

                if (forkStreet(forkChance) && !justFork && !forked && count++ > 0 && xi != steps) {

                    forked = true;
                    forkedBool = true;
                    if (random(0, 100) < doubleForkChance) // fork in all directions
                    {
                        generateRoads(matrix, xi, y + 1, n, m, 0, forkChance / 2, first, true);
                        generateRoads(matrix, xi, y - 1, n, m, 1, forkChance / 2, first, true);
                    } else {
                        int newdir = possibleDirections[random(0, possibleDirections.size())];
                        if (newdir == 0)
                            generateRoads(matrix, xi, y + 1, n, m, newdir, forkChance / 2, first, true);
                        else
                            generateRoads(matrix, xi, y - 1, n, m, newdir, forkChance / 2, first, true);
                    }
                }
                justFork = false;
                if (!forkedBool) forked = false;


            }
        }

    } else {
        return 0;
    }
}


//This method cheks the positions around the input position and the output is:
//0 if there are no crossroads
//1 if there is a crossroad at right
//2 if there is a crossroad at left
//3 if there are even right even left crossroads
//4 if there is frontal and right
//5 if there is frontal and left
//6 if there is a total crossroad
//7 end of road
//8 end of road inverse
int checkIncrocio(int matrix[], int x, int y, int n, int m, int dir) {
    auto incroci = 0;
    if (dir == 0) {
        if (x + 1 < n && matrix[(x + 1) * n + y] == 2)
            incroci += 2;
        if (x - 1 >= 0 && matrix[(x - 1) * n + y] == 2)
            incroci += 1;
        if (y + 1 < m && matrix[x * n + y + 1] == 0 && incroci != 0) incroci += 3;
        if (((y + 1 < m && matrix[x * n + y + 1] == 5) || y + 1 == m) && incroci == 0) incroci += 7;
        if (((y - 1 >= 0 && matrix[x * n + y - 1] == 5) || y - 1 < 0) && incroci == 0) incroci += 8;
    } else if (dir == 2) {
        if (y + 1 < m && matrix[x * n + y + 1] == 0)
            incroci += 1;
        if (y - 1 >= 0 && matrix[x * n + y - 1] == 0)
            incroci += 2;
        if (x + 1 < n && matrix[(x + 1) * n + y] == 2 && incroci != 0) incroci += 3;
        if (((x + 1 < n && matrix[(x + 1) * n + y] == 5) || x + 1 == n) && incroci == 0) incroci += 7;
        if (((x - 1 >= 0 && matrix[(x - 1) * n + y] == 5) || x - 1 < 0) && incroci == 0) incroci += 8;
    }
    return incroci;
}


//This method checks if the actual site for building needs a rotation output is:
//0 if don't need rotation
//1 if 90 degree rotation
//2 if 180 degree rotation
//3 if 270 degree rotation
//4 if this site is not near road
int checkBuildingSite(int matrix[], int x, int y, int n, int m) {
    vector<int> possibilities;
    if (x + 1 < n && (matrix[(x + 1) * n + y] == 2 || matrix[(x + 1) * n + y] == 0)) possibilities.push_back(1);
    if (x - 1 >= 0 && (matrix[(x - 1) * n + y] == 2 || matrix[(x - 1) * n + y] == 0)) return 0;
    if (y + 1 < m && (matrix[x * n + y + 1] == 2 || matrix[x * n + y + 1] == 0)) possibilities.push_back(2);
    if (y - 1 >= 0 && (matrix[x * n + y - 1] == 2 || matrix[x * n + y - 1] == 0)) return 3;

    if (possibilities.size() == 0) return 4;

    return possibilities.at(random(0, possibilities.size() - 1));


}

//this method returns only all the possibile rotations map
map<int, float> getRotationsMap() {
    map<int, float> rotationsMap;

    rotationsMap[0] = 0.0f;
    rotationsMap[1] = 90.0f;
    rotationsMap[2] = 180.0f;
    rotationsMap[3] = 270.0f;

    return rotationsMap;
};

//This method returns only all the possibile rotations
vector<float> getRotations() {
    vector<float> pushed;
    pushed.push_back(90.0f);
    pushed.push_back(180.0f);
    pushed.push_back(270.0f);
    pushed.push_back(0.0f);
    return pushed;
};

int createBuilding(int *matrix, int x, int y, int n, int m, int floors, int type, map<string, model *> modelsMap,
                    scene *scn, map<int, float> rotations) {
    int countX = 0, countYFinal = 0;


    if (floors >= 4) {
        for (int xi = x; xi < n; xi++) {
            if ((matrix[(xi) * n + y] == 2 || matrix[(xi) * n + y] == 0) || matrix[(xi) * n + y] == 6) break;

            if (xi == x && checkBuildingSite(matrix, xi, y, n, m) == 4) break;
            //cout << xi << " counted \n";
            int countY = 0;
            for (int yi = y; yi < m; yi++) {
                if ((matrix[xi * n + yi] == 2 || matrix[xi * n + yi] == 0) || matrix[xi * n + yi] == 6) break;
                //matrix[xi*n+yi] = 7;
                countY++;
            }
            countX++;

            if (countYFinal != 0)
                countYFinal = min(countYFinal, countY);
            else
                countYFinal = countY;
        }
    } else {
        countX = 1;
        countYFinal = 1;
    }
    if (countX > 1 && countYFinal > 1) {

        countX = random(2, min(3, countX));
        countYFinal = random(2, min(3, countYFinal));
        if(floors < 6) type = 0;
        int floorType = random(0, modelsMap["floor"]->possibilities.at(type).size() - 1);


        for (int xi = x; xi < x + countX; xi++) {
            for (int yi = y; yi < y + countYFinal; yi++) {

                frame3f frame;
                int roofType = 0;
                int roofRotation = 0;
                frame = rotateFrame(rotations[0], xi, yi);
                int tmpType = type;


                if (xi == x + countX - 1 && yi == y + countYFinal - 1) {
                    roofRotation = type == 0 ? 2 : 0;
                    frame = type == 0 ? frame : rotateFrame(rotations[1], xi, yi);
                } else if (xi == x + countX - 1 && yi == y) {
                    roofRotation = type == 0 ? 3 : 0;
                    frame = type == 0 ? frame : rotateFrame(rotations[2], xi, yi);
                } else if (yi == y + countYFinal - 1 && xi == x) {
                    roofRotation = type == 0 ? 1 : 0;
                    frame = type == 0 ? frame : rotateFrame(rotations[0], xi, yi);
                } else if (xi == x + countX - 1) {
                    roofType = type == 0 ? 1 : 0;
                    roofRotation = type == 0 ? 3 : 0;

                    frame = type == 0 ? rotateFrame(rotations[2], xi, yi) : rotateFrame(rotations[1], xi, yi);
                    tmpType = 0;

                } else if (yi == y + countYFinal - 1) {
                    roofType = type == 0 ? 1 : 0;
                    roofRotation = type == 0 ? 2 : 0;

                    frame = rotateFrame(rotations[1], xi, yi);
                    tmpType = 0;


                } else if (xi == x && yi == y) {
                    roofRotation = type == 0 ? 0 : 0;
                    frame = type == 0 ? frame : rotateFrame(rotations[3], xi, yi);

                } else if (xi == x) {
                    roofType = type == 0 ? 1 : 0;
                    roofRotation = type == 0 ? 1 : 0;
                    tmpType = 0;

                } else if (yi == y) {
                    roofType = type == 0 ? 1 : 0;
                    roofRotation = type == 0 ? 0 : 0;
                    tmpType = 0;
                    frame = rotateFrame(rotations[3], xi, yi);

                } else {
                    roofType = type == 0 ? 2 : 0;
                    roofRotation = type == 0 ? 0 : 0;
                    tmpType = 0;
                }

                matrix[xi * n + yi] = 6;

                bool noBase = !(xi == x && yi == y);
                if (xi == x && yi == y)
                    frame = type == 0 ? rotateFrame(rotations[checkBuildingSite(matrix, xi, yi, n, m)], xi, yi) : frame;

                auto tmpFloors = noBase && tmpType == 0 ? floors + 1 : floors;
                recursiveCreateBuilding(scn, tmpType, "building" + to_string(xi) + to_string(yi),
                                        tmpType == 1 ? tmpFloors + 1 : tmpFloors, 0,
                                        frame, vec3f{0, 0, 0}, modelsMap, type == 1 ? -2 : roofType, rotations,
                                        roofRotation, xi, yi,
                                        noBase, floorType);

            }
        }

    } else if( countX == 1 && countYFinal == 1 && floors <4){

        auto rotationNeeded = checkBuildingSite(matrix, x, y, n, m);
        if (rotationNeeded == 4) return 0;
        int floorType = random(0, modelsMap["floor"]->possibilities.at(type).size() - 1);

        recursiveCreateBuilding(scn, 0, "building" + to_string(x) + to_string(y), floors, 0,
                                rotateFrame(rotations[rotationNeeded], x, y), vec3f{0, 0, 0},
                                modelsMap, floorType);
    }


}

//This method will generate a city with:
// citySize: matrix size
// maxFloors: max number of floors of the  buildings of the city
// minFloors: min number of floors of the  buildings of the city
// streetSplitChance: the chance for street to fork look at method comment for more info
// buildingCreationChance: the probability to create a buiding in each possible position (near roads) higher more buildings will be built
// treeCreationChance: probability for trees to be placed in a position
// urbanization: rappresents how much the city is urbanized(has higher buildings and less trees) it starts from the center of the matrix
//               and will create urban enviroment until the urbanization percentage is ended so the borders will be more rural (lower buildings and more trees)
void generate(scene *scn, int citySize, int maxFloors, int minFloors, int streetSplitChance, int buildingCreationChance,
              int treeCreationChance, int urbanization) {
    auto x = citySize;
    auto y = citySize;
    int matrixBuildings[x * y + 1];


    maxFloors = max(maxFloors, 3);
    minFloors = max(minFloors, 1);
    minFloors = min(maxFloors, minFloors);
    for (auto xi = 0; xi < x; xi++)
        for (auto yi = 0; yi < y; yi++)
            matrixBuildings[xi * x + yi] = 5;
    cout << "[INFO] Generating roads\n";
    generateRoads(matrixBuildings, x / 2, random(0, 0), x, y, 0, streetSplitChance, true, false);
    cout << "[INFO] Roads generated, generation of buildings and trees started\n";

    auto rotations = getRotationsMap();
    auto urbanizationRate = 1.0f - urbanization / 100.0f;
    auto modelsMap = loadModels();

    for (auto xi = 0; xi < x; xi++) {
        auto attraversamentoCount = 0;
        for (auto yi = 0; yi < y; yi++) {
            auto frame = frame3f{{1, 0, 0}, {0, 1, 0}, {0, 0, 1},
                                 vec3f{0, 0, 1} * yi + vec3f{1, 0, 0} * xi};
            auto dir = matrixBuildings[xi * x + yi];
            if (dir == 6) continue;
            if (dir == 5) {

                if (random(0, 100) <= buildingCreationChance) {

                    auto floors = xi < x * urbanizationRate || xi > x - x * urbanizationRate ||
                                  yi < y * urbanizationRate || yi > y - y * urbanizationRate ? random(minFloors, minFloors <= 4 ? 4 : minFloors) : random( minFloors <= 2 ? 2 : minFloors, maxFloors);
                    auto type = floors > 2 ? random(0, 1) : 0;
                    createBuilding(matrixBuildings, xi, yi, x, y, floors, type, modelsMap, scn, rotations);

                } else if (random(0, 100) <= treeCreationChance) {
                    auto maxTrees =
                            xi < x * urbanizationRate || xi > x - x * urbanizationRate || yi < y * urbanizationRate ||
                            yi > y - y * urbanizationRate ? random(0, 2) : random(0, 1);
                    auto count = 0;
                    auto pushed = getRotations();
                    while (count < maxTrees) {
                        auto rotation = pushed.at(random(0, pushed.size() - 1));
                        pushed.erase(std::remove(pushed.begin(), pushed.end(), rotation), pushed.end());
                        add_obj(scn, modelsMap["tree"]->getRandomPossibility(),
                                "tree" + to_string(xi) + to_string(yi) + to_string(count++),
                                rotateFrame(rotation, xi, yi));
                    }
                }
                /*
                if (random(0, 100) <= buildingCreationChance) {
                    auto rotationNeeded = checkBuildingSite(matrixBuildings, xi, yi, x, y);
                    if (rotationNeeded == 4) continue;
                    else {
                        frame = rotateFrame(rotations[rotationNeeded], xi, yi);
                        auto floors = xi < x * urbanizationRate || xi > x - x * urbanizationRate ||
                                     yi < y * urbanizationRate || yi > y - y * urbanizationRate ? random(minFloors, minFloors <=2? 2 :minFloors) : random(
                                minFloors <=3 ? 3 : minFloors, maxFloors);
                        auto type = floors > 2 ? random(0, 1) : 0;
                        recursiveCreateBuilding(scn, type, "building" + to_string(xi) + to_string(yi), floors, 0,
                                                frame, vec3f{0, 0, 0}, modelsMap);
                    }
                } else if (random(0, 100) <= treeCreationChance) {
                    auto maxTrees =
                            xi < x * urbanizationRate || xi > x - x * urbanizationRate || yi < y * urbanizationRate ||
                            yi > y - y * urbanizationRate ? random(0, 2) : random(0, 1);
                    auto count = 0;
                    auto pushed = getRotations();
                    while (count < maxTrees) {
                        auto rotation = pushed.at(random(0, pushed.size() - 1));
                        pushed.erase(std::remove(pushed.begin(), pushed.end(), rotation), pushed.end());
                        add_obj(scn, modelsMap["tree" ]->getRandomPossibility(),
                                "tree" + to_string(xi) + to_string(yi) + to_string(count++),
                                rotateFrame(rotation, xi, yi));
                    }
                }
                 */
            } else {
                if (dir == 2 || dir == 3) {
                    frame = rotation_frame3f({0, 1, 0}, 90.0f * pif / 180.0f);
                    frame.o = vec3f{0, 0, 1} * yi + vec3f{1, 0, 0} * xi;
                    frame.o += vec3f{1, 0, 0} * 1.0f;
                    frame.o += vec3f{0, 0, 1} * -1.0f;
                } else
                    frame.o += vec3f{1, 0, 0} * 1.0f;
                frame.o += vec3f{0, 1, 0} * -0.2; //low the street model

                auto incrocio = checkIncrocio(matrixBuildings, xi, yi, x, y, dir);
                if (incrocio > 0) {
                    add_obj(scn, modelsMap["crossing"]->possibilities.at(0).at(incrocio),
                            "crossing" + to_string(xi) + to_string(yi), frame);

                } else //put crosswalk
                {
                    if (random(0, 100) < 10 && dir == 0 && attraversamentoCount < 2) {
                        attraversamentoCount++;
                        add_obj(scn, modelsMap["road"]->possibilities.at(0).at(1),
                                "road" + to_string(xi) + to_string(yi), frame);
                    } else
                        add_obj(scn, modelsMap["road"]->possibilities.at(0).at(0),
                                "road" + to_string(xi) + to_string(yi),
                                frame);
                }
            }
        }
    }
    cout << "[INFO] Buildings and trees generated\n";


}


//this method will init the scene
//taken from Yocto-gl library and modified the parameters
void initScene(scene *scn, bool sunset) {
    auto mat = new material{"floor"};
    mat->kd = {(1.0f / 255.0f) * 82, (1.0f / 255.0f) * 111, (1.0f / 255.0f) * 53};
    scn->materials.push_back(mat);
    auto shp = new shape{"floor"};
    float posWidth = 200;
    shp->pos = {{-posWidth, 0, -posWidth},
                {posWidth,  0, -posWidth},
                {posWidth,  0, posWidth},
                {-posWidth, 0, posWidth}};
    shp->norm = {{0, 1, 0},
                 {0, 1, 0},
                 {0, 1, 0},
                 {0, 1, 0}};
    shp->texcoord = {{-10, -10},
                     {10,  -10},
                     {10,  10},
                     {-10, 10}};
    shp->triangles = {{0, 1, 2},
                      {0, 2, 3}};
    shp->mat = mat;
    scn->shapes.push_back(shp);
    scn->instances.push_back(new instance{"floor", identity_frame3f, shp});


    auto lshp = new shape{"sun"};
    lshp->pos = {{-20.4f, sunset ? 30.0f : 128.0f, 44}};
    lshp->points = {0};
    scn->shapes.push_back(lshp);
    auto lmat = new material{"sun"};
    float nightmode = sunset ? 20.0f : 1.0f;
    lmat->ke = {sunset ? 250000 / nightmode : 100000, sunset ? 140000 / nightmode : 100000,
                sunset ? 100000 / nightmode : 100000};
    lshp->mat = lmat;
    scn->materials.push_back(lmat);
    instance *istL = new instance{"sun", identity_frame3f, lshp};
    scn->instances.push_back(
            istL);

    auto cam = new camera{"cam"};
    cam->frame = {{-0.579,  0,    0.815},
                  {0.184,   0.974, 0.131},
                  {-0.794,  0.226, -0.564},
                  {-18.867, 11.604,   -13.814}}; //lookat_frame3f({-11, 11, -2}, {0, 7, 0}, {0, 7, 0});

    //cam->frame = {{-0.618, 0, 0.786}, {0.205,0.965,0.161},{-0.759,0.261, -0.596},{-10.700, 7.999, -9.224} }; //lookat_frame3f({-11, 11, -2}, {0, 7, 0}, {0, 7, 0});

    cam->yfov = 17 * pif / 170.f;
    //cam->yfov = 20 * pif / 170.f;
    //cam->yfov = 10 * pif / 170.f;

    cam->aspect = 16.0f / 9.0f;
    cam->aperture = 0;
    cam->focus = length(vec3f{0, 4, 10} - vec3f{0, 1, 0});
    scn->cameras.push_back(cam);
}

int main(int argc, char **argv) {

    // parse command line
    auto parser =
            make_parser(argc, argv, "city_generator", "generate a city");

    auto citySize = parse_opt(parser, "--city-size", "-size", "city size", 30);

    auto minFloors = parse_opt(parser, "--min-floors", "-m", "min floors", 1);

    auto maxFloors = parse_opt(parser, "--max-floors", "-M", "max floors", 5);

    auto streetSplitChance = parse_opt(parser, "--street-split-probability", "-sc",
                                       "probability for the street to split", 30);

    auto buildingCreationChance = parse_opt(parser, "--building-creation-probability", "-bc",
                                            "probability for the building to be set in a location", 50);

    auto treeCreationChance = parse_opt(parser, "--tree-creation-probability", "-tc",
                                        "probability for a tree to be set in a location", 30);

    auto urbanization = parse_opt(parser, "--urbanization", "-u", "urbanization of the city", 75);

    auto sunset = parse_flag(parser, "--sunset-mode", "-s", "activate sunset mode");

    auto outputFile = parse_opt(parser, "--output-image", "-o", "image filename", "scene_out.obj"s);

    auto *scn = new scene();

    initScene(scn, sunset);

    generate(scn, citySize, maxFloors, minFloors, streetSplitChance, buildingCreationChance, treeCreationChance,
             urbanization);

    cout << "[INFO] Saving scene\n";

    save_scene(outputFile, scn, save_options{});

    cout << "[INFO] Scne saved\n";


    return 0;
}
