#include "rigidbody.h"
#include "glm/gtx/transform.hpp"
#include "shapes/cone.h"
#include "shapes/cube.h"
#include "shapes/cylinder.h"
#include "shapes/sphere.h"
#include <cstring>
#include <glm/glm.hpp>
#include <iostream>

RigidBody::RigidBody(const ScenePrimitive& shape, std::optional<Collider*> collider, std::vector<Collider*>* others) {
    glm::mat3 Ibody;
    switch (shape.type) {
    // use cube for animated model
    case PrimitiveType::PRIMITIVE_CUBE:
    case PrimitiveType::PRIMITIVE_ANIMATED_MODEL:
        Ibody = Cube().Ibody(shape.mass);
        break;
    case PrimitiveType::PRIMITIVE_CONE:
        Ibody = Cone().Ibody(shape.mass);
        break;
    case PrimitiveType::PRIMITIVE_CYLINDER:
        Ibody = Cylinder().Ibody(shape.mass);
        break;
    case PrimitiveType::PRIMITIVE_SPHERE:
        Ibody = Sphere().Ibody(shape.mass);
        break;
    case PrimitiveType::PRIMITIVE_MESH:
        // use sphere to represent mesh
        Ibody = Sphere().Ibody(shape.mass);
        break;
    }

    // initial conditions are zero / no movement
    this->x = glm::vec3(0, 0, 0);
    this->v = glm::vec3(0, 0, 0);
    this->R = glm::mat3(1, 0, 0,
                        0, 1, 0,
                        0, 0, 1);
    this->mass = shape.mass;
    // initial momentum is also zero
    this->P = glm::vec3(0, 0, 0);
    this->L = glm::vec3(0, 0, 0);

    // IBody must be calculated per shape!
    this->Ibody = Ibody;
    this->Ibodyinv = glm::inverse(Ibody);
    this->forces = shape.forces;
    this->t = 0;

    // save colliders
    this->collider = collider;
    this->other_colliders = others;
    if (collider.has_value()) {
        collider.value()->body = (void*) this;
    }

    // update yfinal
    State_to_Array(yfinal);

    double t0 = shape.t_offset;
    // step from 0 to t0
    if (t0 != 0) {
        this->step(t0);
    }
}

/* Copy the state information into an array */
void RigidBody::State_to_Array(double *y) {
    *y++ = x[0]; /* x component of position */
    *y++ = x[1]; /* etc. */
    *y++ = x[2];
    for(int i = 0; i < 3; i++) /* copy rotation matrix */
        for(int j = 0; j < 3; j++)
            *y++ = R[i][j];
    *y++ = P[0];
    *y++ = P[1];
    *y++ = P[2];
    *y++ = L[0];
    *y++ = L[1];
    *y++ = L[2];
}

/* Copy information from an array into the state variables */
void RigidBody::Array_to_State(const double *y) {
    x[0] = *y++;
    x[1] = *y++;
    x[2] = *y++;
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            R[i][j] = *y++;
    P[0] = *y++;
    P[1] = *y++;
    P[2] = *y++;
    L[0] = *y++;
    L[1] = *y++;
    L[2] = *y++;

    /* Compute auxiliary variables... */
    /* v(t) = P(t) / M */
    v = glm::vec3(P.x / mass, P.y / mass, P.z / mass);

    /* I−1(t) = R(t) I−1body R(t)T */
    Iinv = R * Ibodyinv * glm::transpose(R);

    /* ω(t) = I−1(t)L(t) */
    omega = Iinv * L;
}

void RigidBody::Compute_Force_and_Torque(double t) {
    // reset force and torque
    force = glm::vec3(0, 0, 0);
    torque = glm::vec3(0, 0, 0);

    // iterate over forces and calculate cumulative
    for (Force& f : this->forces) {
        if (f.m_type == ForceType::periodic) {
            int t_mapped = (int) ((t / f.period) + 0.5);
            if ((t_mapped % 2) == 0) {
                // even is pos
                force += f.trans;
                torque += f.torque;
            } else {
                // odd is neg
                force -= f.trans;
                torque -= f.torque;
            }
        } else if (f.m_type == ForceType::impulse) {
            if (t >= f.t0 && t < f.t1) {
                force += f.trans;
                torque += f.torque;
            }
        } else if (f.m_type == ForceType::constant) {
            force += f.trans;
            torque += f.torque;
        }
    }

    if (this->collider.has_value()) {
        Collider* c = this->collider.value();
        for (auto it = c->banned_dirs.begin(); it != c->banned_dirs.end(); it++) {
            glm::vec3 ban = 1.0f - glm::abs(it->second);
            force *= ban;
        }
    }
}

typedef void (*dydt_func)(RigidBody* rb, double t, const double y[], double dy[]);

// simple euler solver
void ode(double y0[], double yend[], double t0, double t1, RigidBody* rb, dydt_func dydt) {
    // tradeoff between speed and accuracy. Higher is more accurate obviously since it takes more steps
    int steps = (int) ((t1 - t0) * 10000.0) + 10; // at least 10 steps but dynamically adjusted for the timespan
    double h = (t1 - t0) / steps;

    double t = t0;
    double y[STATE_SIZE], dy[STATE_SIZE];

    memcpy(y, y0, sizeof(double) * STATE_SIZE);
    for (int k = 0; k < steps; k++) {
        dydt(rb, t, y, dy);
        for (int i = 0; i < STATE_SIZE; i++) {
            y[i] += h * dy[i];
        }
        t += h;
    }
    memcpy(yend, y, sizeof(double) * STATE_SIZE);
}

glm::mat3 Star(glm::vec3 a) {
    return glm::mat3(0, a[2], -a[1],
                     -a[2], 0, a[0],
                     a[1], -a[0], 0);
}

void RigidBody::ddt_State_to_Array(double *ydot)
{
    /* copy d/dt x(t) = v(t) into ydot */
    *ydot++ = v[0];
    *ydot++ = v[1];
    *ydot++ = v[2];

    /* Compute Rdot(t) = ω(t) * R(t) */
    glm::mat3 Rdot = Star(omega) * R;
    /* copy Rdot(t) into array */
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            *ydot++ = Rdot[i][j];

    /* d/dt P(t) = F(t) */
    *ydot++ = force[0];
    *ydot++ = force[1];
    *ydot++ = force[2];

    /* d/dt L(t) = τ(t) */
    *ydot++ = torque[0];
    *ydot++ = torque[1];
    *ydot++ = torque[2];
}

 void RigidBody::dydt(RigidBody* rb, double t, const double y[], double ydot[]) {
    /* put data in y[ ] into Bodies[ ] */
    rb->Array_to_State(y);
    rb->Compute_Force_and_Torque(t);
    rb->ddt_State_to_Array(ydot);
}

void RigidBody::step(double deltaT) {
    // skip calculation if no forces are being applied and there is no collider (object cannot move)
    if (this->forces.empty() && !this->collider.has_value()) {
        return;
    }

    // calculate the t values
    double t0 = t;
    double t1 = t + deltaT;
    t += deltaT;

    /* copy yfinal back to y0 */
    memcpy(y0, yfinal, sizeof(double) * STATE_SIZE);
    ode(y0, yfinal, t0, t1, this, dydt);
    /* copy d/dt Y(t + delta) into state variables */
    this->Array_to_State(yfinal);
}

std::pair<double, double> quadratic(double a, double b, double c) {
    if ((b*b - 4*a*c) < 0) {
        double both = (-b) / (2 * a);
        return std::make_pair(both, both);
    }
    if (a == 0) {
        std::cout << "mass cannot be zero" << std::endl;
        return std::make_pair(0, 0);
    }
    double first = (-b + sqrt(b*b - 4*a*c)) / (2 * a);
    double second = (-b - sqrt(b*b - 4*a*c)) / (2 * a);
    return std::make_pair(first, second);
}

std::pair<double, double> solve_momentum_equation(double m1, double m2, double v1i, double v2i, float absorb) {
    double Ke = (0.5 * m1 * v1i * v1i) + (0.5 * m2 * v2i * v2i);
    double p = (m1 * v1i) + (m2 * v2i);

    // calculate co-efficients for quadratic formula
    double a = (m1 * m2) + (m1 * m1);
    double b = -2.0 * p * m1;
    double cterm = (p * p) - (2.0 * (1.0 - absorb) * Ke * m2);
    std::pair<double, double> sols = quadratic(a, b, cterm);

    double v1fa = sols.first;
    double v2fa = (p - (m1*v1fa)) / m2;
    double v1fb = sols.second;
    double v2fb = (p - (m1*v1fb)) / m2;

    // pick the solution that is farther away in velocity-space
    if (abs(sols.first - v1i) > abs(sols.second - v1i)) {
        return std::make_pair(v1fa, v2fa);
    } else {
        return std::make_pair(v1fb, v2fb);
    }
}

void RigidBody::move_by(glm::vec3 pos) {
    this->x += pos;
    this->State_to_Array(this->yfinal);

    if (!this->collider.has_value()) {
        return;
    }
    this->collider.value()->update_pos(glm::translate(x));
}

void RigidBody::rot_by(float theta) {
    // rotate about y axis
    this->R = glm::rotate(theta, glm::vec3(0, -1, 0));
    this->State_to_Array(this->yfinal);
}

void RigidBody::collide() {
    if (!this->collider.has_value()) {
        return;
    }

    Collider* mycollider = this->collider.value();
    RigidBody* mybody = this;
    for (auto it = this->other_colliders->begin(); it != this->other_colliders->end(); it++) {
        Collider* ocollider = *it;
        RigidBody* obody = (RigidBody*) ocollider->body;

        // skip one set of interactions to avoid double-collisions
        if (mycollider->id <= ocollider->id) {
            continue;
        }

        auto mbcoll = ocollider->collides(mycollider);
        if (mbcoll.has_value()) {
            Collision c = mbcoll.value();
            // skip checking banned collisions
            if (mycollider->banned_dirs.contains(ocollider->id)) {
                continue;
            }
            // insert direction to be banned
            mycollider->banned_dirs[ocollider->id] = c.normal_ws;
            ocollider->banned_dirs[mycollider->id] = c.normal_ws;

            // calculation of the absorbtion parameter. It cannot be 0 because that would allow shapes to fuse...
            float absorb = 1 - ((1 - mycollider->energy_loss) * (1 - ocollider->energy_loss));
            if (!mycollider->is_ground) {
                // if we aren't ground, calculate the change in our momentum
                if (ocollider->is_ground) {
                    // "ball-to-wall"
                    // calculate new momentum (assume ground is static, absorbs energy, and delivers momentum back)
                    glm::vec3 flipped_moment = ((-2.0f + absorb) * std::min(glm::dot(c.normal_ws, mybody->P), 0.0f)) * c.normal_ws;
                    mybody->P += flipped_moment;
                } else {
                    // "ball-to-ball"

                    // calculate kinetic energies
                    double m1 = mybody->mass;
                    double m2 = obody->mass;
                    double v1i = glm::dot(c.normal_ws, mybody->v);
                    double v2i = glm::dot(c.normal_ws, obody->v);
                    auto pair = solve_momentum_equation(m1, m2, v1i, v2i, absorb);
                    double v1f = pair.first;
                    double v2f = pair.second;

                    // update momentums if the objects are not separating
                    if (!(v1i > 0 && v2i < 0)) {
                        glm::vec3 myPrevMomentum = mybody->P - (mybody->P * glm::abs(c.normal_ws));
                        glm::vec3 oPrevMomentum = obody->P - (obody->P * glm::abs(c.normal_ws));
                        mybody->P = ((float) (v1f * m1) * c.normal_ws) + myPrevMomentum;
                        obody->P = ((float) (v2f * m2) * c.normal_ws) + oPrevMomentum;
                    }
                }
            } else {
                if (!ocollider->is_ground) {
                    // "wall-to-ball"
                    // calculate new momentum (assume ground is static, absorbs energy, and delivers momentum back)
                    glm::vec3 flipped_moment = ((-2.0f + absorb) * std::min(glm::dot(-c.normal_ws, obody->P), 0.0f)) * -c.normal_ws;
                    obody->P += flipped_moment;
                } else {
                    // "wall-to-wall", doesn't change
                    continue;
                }
            }

            // update yfinal
            mybody->v = glm::vec3(mybody->P.x / mybody->mass, mybody->P.y / mybody->mass, mybody->P.z / mybody->mass);
            mybody->State_to_Array(mybody->yfinal);
            obody->v = glm::vec3(obody->P.x / obody->mass, obody->P.y / obody->mass, obody->P.z / obody->mass);
            obody->State_to_Array(obody->yfinal);
        } else {
            // remove collision
            mycollider->banned_dirs.erase(ocollider->id);
            ocollider->banned_dirs.erase(mycollider->id);
        }
    }
    this->collider.value()->update_pos(glm::translate(x));
}

glm::mat4 RigidBody::movement_matrix() {
    return glm::translate(x) * glm::mat4(R);
}
