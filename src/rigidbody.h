#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "collider.h"
#include "force.h"
#include <glm/glm.hpp>
#include <vector>

#define STATE_SIZE 18

class RigidBody
{
public:
    // Constant quantities
    double mass; // mass M
    glm::mat3 Ibody; // Ibody
    glm::mat3 Ibodyinv; // I−1 body (inverse of Ibody)

    // State variables
    glm::vec3 x; // x(t) position
    glm::mat3 R; // R(t) rotation
    glm::vec3 P; // P(t) momenutm
    glm::vec3 L; // L(t) angular momentum

    // Derived quantities (auxiliary variables)
    glm::mat3 Iinv; // I−1(t)
    glm::vec3 v; // v(t) velocity
    glm::vec3 omega; // w(t) angular velocity

    // Computed quantities
    glm::vec3 force; // F(t)
    glm::vec3 torque; // tau(t)

    std::vector<Force> forces;

    RigidBody(const ScenePrimitive& data, std::optional<Collider*> collider, std::vector<Collider*>* others);
    void step(double deltaT);
    void collide();
    glm::mat4 movement_matrix();

    std::optional<Collider*> collider;
    const std::vector<Collider*>* other_colliders;
private:
    double t;
    double y0[STATE_SIZE];
    double yfinal[STATE_SIZE];

    void Array_to_State(const double *y);
    void State_to_Array(double *y);
    void Compute_Force_and_Torque(double t);
    void ddt_State_to_Array(double *ydot);
    static void dydt(RigidBody* rb, double t, const double y[], double ydot[]);
};

#endif // RIGIDBODY_H
