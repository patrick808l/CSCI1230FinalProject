#ifndef LSYSTEM_H
#define LSYSTEM_H

#include <glm/glm.hpp>
#include <utils/sceneparser.h>
#include <unordered_map>
#include <string>
#include <vector>

class LSystem
{
public:
    LSystem();

    LSystem(const std::string& seed,
            const std::unordered_map<char, std::string>& rules,
            float step,
            float turnAngleRad);

    std::string generate(int n);
    std::vector<RenderShapeData> interpret(std::string genStr, glm::vec3 startPos, std::string sceneFilePath);

private:
    std::string m_seed;
    std::unordered_map<char, std::string> m_rules;
    float m_step;
    float m_turnAngle;

    struct Turtle {
        glm::vec3 pos;
        glm::vec3 moveX = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 moveY = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 moveZ = glm::vec3(0.0f, 0.0f, 1.0f);
    };

    Turtle rotateByAxis(const Turtle& myTurtle, glm::vec3 axis, float turnAngleRad);
    void addLeaf(std::vector<RenderShapeData>& branches, const glm::vec3 newPos, const Turtle& myTurtle,
                 const SceneMaterial& leafMat);
};

#endif // LSYSTEM_H
