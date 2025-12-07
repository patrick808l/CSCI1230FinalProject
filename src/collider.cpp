#include "collider.h"

double calc_diml(glm::vec3 dim) {
    return ((dim.x*dim.x) + (dim.y*dim.y) + (dim.z*dim.z));
}

int random_id = 0;

Collider::Collider(const ScenePrimitive& primitive, glm::mat4 matrix) {
    this->id = random_id++;
    this->body = nullptr;

    this->is_ground = primitive.is_ground;
    this->energy_loss = primitive.energy_loss;
    this->ctm = matrix;
    this->pos = ctm * glm::vec4(0, 0, 0, 1);
    double x = (ctm * glm::vec4(1, 0, 0, 0)).x;
    double y = (ctm * glm::vec4(0, 1, 0, 0)).y;
    double z = (ctm * glm::vec4(0, 0, 1, 0)).z;
    // add a bit of buffer space to give objects buffer to overlap
    this->dim = glm::vec3((x * 0.5) + 0.01, (y * 0.5) + 0.01, (z * 0.5) + 0.01);

    this->diml = calc_diml(this->dim);
}

void Collider::update_pos(glm::mat4 move) {
    this->pos = (this->ctm * move) * glm::vec4(0, 0, 0, 1);
}

double sign(double v) {
    return v > 0 ? 1.0 : -1.0;
}

std::optional<Collision> Collider::collide_cube_to_cube(Collider* other) {
    // cube to cube collision
    glm::vec3 diff = glm::abs(this->pos - other->pos);
    glm::vec3 dimsum = this->dim + other->dim;
    for (int i = 0; i < 3; i++){
        if (diff[i] > dimsum[i]) {
            return std::nullopt;
        }
    }

    // calculate axis of intersection
    int minAxis = 0;
    float minDist = dimsum[0] - diff[0];
    for (int i = 1; i < 3; i++){
        if (minDist > (dimsum[i] - diff[i])) {
            minAxis = i;
            minDist = dimsum[i] - diff[i];
        }
    }

    // figure out sign
    float sign = 1.0;
    if (this->pos[minAxis] > other->pos[minAxis]) {
        sign = -1.0;
    }

    glm::vec3 normal = glm::vec3(0, 0, 0);
    glm::vec3 pos = glm::vec3(0, 0, 0);
    glm::vec3 dim = glm::vec3(0, 0, 0);

    // setup normal on the correct axis in the correct direction
    normal[minAxis] = sign;

    // calculate an intersection area
    for (int i = 0; i < 3; i++) {
        if (i == minAxis) {
            pos[i] = this->pos[i] + (this->dim[i] * sign);
            dim[i] = 0;
        } else {
            float low = std::max(this->pos[i] - this->dim[i], other->pos[i] - other->dim[i]);
            float high = std::min(this->pos[i] + this->dim[i], other->pos[i] + other->dim[i]);
            pos[i] = (low + high) * 0.5;
            dim[i] = high - low;
        }
    }

    // collision found
    return Collision {
        .pos_ws = pos,
        .facedim = dim,
        .normal_ws = normal,
    };
}

std::optional<Collision> Collider::collides(Collider* other) {
    return this->collide_cube_to_cube(other);
}
