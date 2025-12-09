#include "lsystem.h"

#include <glm/gtx/transform.hpp>
#include <stack>
#include <filesystem>

LSystem::LSystem() {}

LSystem::LSystem(const std::string& seed,
                 const std::unordered_map<char, std::string>& rules,
                 float step,
                 float turnAngleRad) :
    m_seed(seed), m_rules(rules), m_step(step), m_turnAngle(turnAngleRad) {}

// Generate L-System string with n loop
std::string LSystem::generate(int n){
    std::string genStr = m_seed;

    for(int i = 0; i < n; i++){
        std::string newStr;
        newStr.reserve(genStr.size() * 2);

        for(char c: genStr){
            auto iter = m_rules.find(c);
            if(iter != m_rules.end()){
                newStr += iter->second;
            }
            else{
                newStr += c;
            }
        }

        genStr = newStr;
    }

    return genStr;
}

// Interpret input string and turn it into a vector of RenderShapeData
std::vector<RenderShapeData> LSystem::interpret(std::string genStr, glm::vec3 startPos, std::string sceneFilePath){
    LSystem::Turtle myTurtle;
    myTurtle.pos = startPos;

    std::stack<LSystem::Turtle> turStack;
    std::vector<RenderShapeData> branches;

    std::filesystem::path basepath = std::filesystem::path(sceneFilePath).parent_path().parent_path();

    std::string leafTexturePath = (basepath / "textures" / "Stylized_Leaves_002_basecolor.png").string();
    std::string leafNormPath = (basepath / "textures" / "Stylized_Leaves_002_normal.png").string();

    std::string woodTexturePath = (basepath / "textures" / "Bark_06_basecolor.png").string();
    std::string woodNormPath = (basepath / "textures" / "Bark_06_normal.png").string();

    SceneMaterial branchMat = {};
    branchMat.cDiffuse = glm::vec4(0.41f, 0.31f, 0.f, 1.f);
    branchMat.cSpecular = glm::vec4(0.4f, 0.3f, 0.1f, 1.f);
    branchMat.shininess = 3.f;
    branchMat.blend = 1.f;

    SceneFileMap branchTextMap;
    branchTextMap.isUsed = true;
    branchTextMap.repeatU = 2.f;
    branchTextMap.repeatV = 2.f;
    branchTextMap.filename = woodTexturePath;
    branchMat.textureMap = branchTextMap;

    SceneFileMap branchNormMap;
    branchNormMap.isUsed = true;
    branchNormMap.repeatU = 2.f;
    branchNormMap.repeatV = 2.f;
    branchNormMap.filename = woodNormPath;
    branchMat.normalMap = branchNormMap;

    SceneMaterial leafMat = {};
    leafMat.cAmbient  = glm::vec4(0.01f, 0.3f, 0.15f, 1.f);
    leafMat.cDiffuse  = glm::vec4(0.06f, 0.4f, 0.2f, 1.f);
    leafMat.cSpecular = glm::vec4(0.05f, 0.5f, 0.2f, 1.f);
    leafMat.shininess = 4.f;
    leafMat.blend = 1.f;

    SceneFileMap leafTextMap;
    leafTextMap.isUsed = true;
    leafTextMap.repeatU = 3.f;
    leafTextMap.repeatV = 3.f;
    leafTextMap.filename = leafTexturePath;
    leafMat.textureMap = leafTextMap;

    SceneFileMap leafNormMap;
    leafNormMap.isUsed = true;
    leafNormMap.repeatU = 3.f;
    leafNormMap.repeatV = 3.f;
    leafNormMap.filename = leafNormPath;
    leafMat.normalMap = leafNormMap;

    size_t i = 0;
    for(char c: genStr){


        switch(c){
        case 'F':{ // move forward
            glm::vec3 oldPos = myTurtle.pos;
            glm::vec3 newPos = myTurtle.pos + m_step * myTurtle.moveY;
            glm::vec3 mid = 0.5f * (oldPos + newPos);
            float length = m_step;

            float radius = 0.2f; //0.05
            glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3(radius, length, radius));

            glm::mat4 R(1.f);
            R[0] = glm::vec4(myTurtle.moveX, 0.f);
            R[1] = glm::vec4(myTurtle.moveY, 0.f);
            R[2] = glm::vec4(myTurtle.moveZ, 0.f);

            glm::mat4 T = glm::translate(glm::mat4(1.f), mid);

            ScenePrimitive primitive;
            primitive.type = PrimitiveType::PRIMITIVE_CYLINDER;
            primitive.material = branchMat;
            RenderShapeData branchShape = RenderShapeData(primitive, T * R * S);
            branches.push_back(branchShape);

            myTurtle.pos = newPos;

            // check if current branch is the end
            bool isTip = false;
            if (i + 1 >= genStr.size()) {
                isTip = true;
            } else if (genStr[i + 1] == ']') {
                isTip = true;
            }
            // add leaf to the end of a branch
            if (isTip) {
                addLeaf(branches, newPos, myTurtle, leafMat);
            }

            break;
        }

        case '+': { // turn right by the degree
            myTurtle = rotateByAxis(myTurtle, myTurtle.moveX, m_turnAngle);
            break;
        }
        case '-': { // turn left by the degree
            myTurtle = rotateByAxis(myTurtle, myTurtle.moveX, -m_turnAngle);
            break;
        }

        case '&': { // turn up by the degree
            myTurtle = rotateByAxis(myTurtle, myTurtle.moveZ, m_turnAngle);
            break;
        }
        case '^': { // turn down by the degree
            myTurtle = rotateByAxis(myTurtle, myTurtle.moveZ, -m_turnAngle);
            break;
        }

        case '[': { // start of a branch
            turStack.push(myTurtle);
            break;
        }
        case ']': { // end of a branch
            if (!turStack.empty()) {
                myTurtle = turStack.top();
                turStack.pop();
            }
            break;
        }

        default:
            break;
        }

        i++;
    }

    return branches;
}

LSystem::Turtle LSystem::rotateByAxis(const Turtle& myTurtle, glm::vec3 axis, float turnAngleRad){
    glm::mat4 R = glm::rotate(glm::mat4(1.f), turnAngleRad, glm::normalize(axis));

    Turtle newTurtle = myTurtle;
    newTurtle.moveX = glm::normalize(glm::vec3(R * glm::vec4(myTurtle.moveX, 0.f)));
    newTurtle.moveY = glm::normalize(glm::vec3(R * glm::vec4(myTurtle.moveY, 0.f)));
    newTurtle.moveZ = glm::normalize(glm::vec3(R * glm::vec4(myTurtle.moveZ, 0.f)));

    return newTurtle;
}

void LSystem::addLeaf(std::vector<RenderShapeData>& branches,
                      const glm::vec3 newPos, const LSystem::Turtle& myTurtle,
                      const SceneMaterial& leafMat){

    glm::vec3 leafCenter = newPos;
    float leafLength = 0.35f;
    float leafWidth = 0.75f;
    float leafThickness = 0.57f;

    glm::mat4 Sleaf = glm::scale(glm::mat4(1.f),
                                 glm::vec3(leafWidth, leafLength, leafThickness));

    glm::mat4 Rleaf(1.f);
    Rleaf[0] = glm::vec4(myTurtle.moveX, 0.f);
    Rleaf[1] = glm::vec4(myTurtle.moveY, 0.f);
    Rleaf[2] = glm::vec4(myTurtle.moveZ, 0.f);

    glm::mat4 Tleaf = glm::translate(glm::mat4(1.f), leafCenter);

    ScenePrimitive primitive;
    primitive.type = PrimitiveType::PRIMITIVE_SPHERE;
    primitive.material = leafMat;
    RenderShapeData leafShape = RenderShapeData(primitive, Tleaf * Rleaf * Sleaf);

    branches.push_back(leafShape);
}
