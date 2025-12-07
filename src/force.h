#ifndef FORCE_H
#define FORCE_H

#include <glm/glm.hpp>

enum ForceType {
    periodic, // switches
    impulse,  // only occurs for a small burst
    constant, // constant force
};

class Force
{
public:
    enum ForceType m_type;

    // For an impulse force, the start and end time of it
    double t0, t1;

    // For a periodic force, the period
    double period;

    glm::vec3 trans;
    glm::vec3 torque;

    Force();
};

#endif // FORCE_H
