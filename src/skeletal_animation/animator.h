#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include "animation.h"
#include "bone.h"

class Animator
{
public:
    Animator() {}

    Animation* CurrentAnimation;

    void init(Animation* animation) {
        m_CurrentTime = 0.0;
        CurrentAnimation = animation;

        m_FinalBoneMatrices.reserve(100);

        for (int i = 0; i < 100; i++) {
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
        }

        m_newAnimationQueued = false;
    }

    void init(Animation* animation, float durationModifier) {
        init(animation);
        m_durationModifier = durationModifier;
    }

    void UpdateAnimation(float dt) {
        // std::cout << "animator.UpdateAnimation dt=" << dt << std::endl;
        m_DeltaTime = dt;
        if (CurrentAnimation) {
            float prevTime = m_CurrentTime;
            m_CurrentTime += CurrentAnimation->GetTicksPerSecond() * dt;
            // std::cout << "ticks per second = " << m_CurrentAnimation->GetTicksPerSecond() << ", dt = " << dt << ", duration = " << m_CurrentAnimation->GetDuration() << std::endl;
            // m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
            m_CurrentTime = fmod(m_CurrentTime, CurrentAnimation->GetDuration() * m_durationModifier);

            // switch to new animation if one is queued and this animation recently finished
            if (m_newAnimationQueued && prevTime >= m_CurrentTime) {
                CurrentAnimation = m_QueuedAnimation;
                m_CurrentTime = 0.0;
                m_newAnimationQueued = false;
                m_QueuedAnimation = nullptr;
            }

            CalculateBoneTransform(&CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
        }
    }

    /**
     * @brief queue a new animation to be played when the current one finishes
     * @param pAnimation is only queued if it is different from the current and queued animations
     */
    void QueueAnimation(Animation* pAnimation) {
        if (pAnimation != m_QueuedAnimation && pAnimation != CurrentAnimation) {
            m_QueuedAnimation = pAnimation;
            m_newAnimationQueued = true;
        }
    }

    /**
     * @brief force a new animation to take effect immediately,
     * unless it is already the current animation
     * @param pAnimation
     */
    void ForceAnimation(Animation* pAnimation) {
        if (pAnimation != CurrentAnimation) {
            CurrentAnimation = pAnimation;
            m_CurrentTime = 0.0f;
        }
    }

    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform) {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        Bone* Bone = CurrentAnimation->FindBone(nodeName);

        if (Bone)
        {
            Bone->Update(m_CurrentTime);
            nodeTransform = Bone->GetLocalTransform();
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        auto boneInfoMap = CurrentAnimation->GetBoneIDMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            int index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;
            m_FinalBoneMatrices[index] = globalTransformation * offset;
        }

        for (int i = 0; i < node->childrenCount; i++)
            CalculateBoneTransform(&node->children[i], globalTransformation);
    }

    std::vector<glm::mat4> GetFinalBoneMatrices() {
        return m_FinalBoneMatrices;
    }

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    bool m_newAnimationQueued = false;
    Animation* m_QueuedAnimation = nullptr;

    float m_CurrentTime;
    float m_DeltaTime;
    float m_durationModifier = 1.f;
};
