#ifndef COLLIDER_H
#define COLLIDER_H

#include "utils/sceneparser.h"
#include <glm/glm.hpp>
#include <optional>
#include <unordered_map>

struct Collision {
    glm::vec3 pos_ws; // the point where the two shapes collide
    std::optional<glm::vec3> facedim; // aids in calculating the corners of the shape if it was a face to face collision
    glm::vec3 normal_ws; // the length of normal encodes the magnitude of the collison
};

class Collider
{
    // the ctm for the object, to help in calculating the auxilliary values
    glm::mat4 ctm;
    // position of the collider (world space)
    glm::vec3 pos;
    // original center position (object space)
    glm::vec4 org_pos;
    glm::vec3 dim; // (width, height, depth)

    // calculate collision for cube to cube
    std::optional<Collision> collide_cube_to_cube(Collider* other);

public:
    // unique id for the collider to prevent self-collisions
    int id;
    // unsafe pointer to rigid body (cannot type correctly b/c of recursive imports)
    void* body;

    /// - A value of 0 indicates no energy is lost (meaning kinetic energy is conserved by this object, collisions bounce)
    /// - A value of 1 indicates all energy is lost (meaning energy is lost, collisions stop movement)
    /// - Any other value interpolates between the two behaviors
    /// - The energy loss of a collision is also dependent on this parameter on the other collider
    double energy_loss;
    bool is_ground;

    /// directions that are "banned" because of collisions
    std::unordered_map<int, glm::vec3> banned_dirs;

    glm::vec3 filter_bans(glm::vec3 dir);
    void update_pos(glm::mat4 new_ctm);
    std::optional<Collision> collides(Collider* other);
    Collider(const ScenePrimitive& data, glm::mat4 ctm);
};

#endif // COLLIDER_H
