#include "collider.h"
#include "shapes/staticmesh.h"

#define STRIDE 22

std::unordered_map<std::string, std::pair<glm::vec4, glm::vec4>> cached_mesh_dims;
std::pair<glm::vec4, glm::vec4> get_dim(std::string meshfile) {
    if (cached_mesh_dims.contains(meshfile)) {
        return cached_mesh_dims[meshfile];
    } else {
        auto m = StaticMesh(meshfile);
        m.updateVertexData(3, 3);
        auto triangles = m.getVertexData();
        double x1 = triangles->at(0);
        double y1 = triangles->at(1);
        double z1 = triangles->at(2);
        double x2 = triangles->at(0);
        double y2 = triangles->at(1);
        double z2 = triangles->at(2);
        for (int i = 0; i < triangles->size(); i += STRIDE) {
            float x = triangles->at(i);
            float y = triangles->at(i+1);
            float z = triangles->at(i+2);

            // update x dims
            if (x1 > x) {
                x1 = x;
            }
            if (x2 < x) {
                x2 = x;
            }

            // update y dims
            if (y1 > y) {
                y1 = y;
            }
            if (y2 < y) {
                y2 = y;
            }

            // update z dims
            if (z1 > z) {
                z1 = z;
            }
            if (z2 < z) {
                z2 = z;
            }
        }

        std::pair<glm::vec4, glm::vec4> dim = std::make_pair(
            glm::vec4((x1+x2)/2.0, (y1+y2)/2.0, (z1+z2)/2.0, 1), // center
            glm::vec4((x2-x1)/2.0, (y2-y1)/2.0, (z2-z1)/2.0, 0)  // dimensions
        );
        cached_mesh_dims[meshfile] = dim;
        return dim;
    }
}

int random_id = 0;

Collider::Collider(const ScenePrimitive& primitive, glm::mat4 matrix) {
    this->id = random_id++;
    this->body = nullptr;
    this->is_player = primitive.is_player;

    this->is_ground = primitive.is_ground;
    this->energy_loss = primitive.energy_loss;
    this->ctm = matrix;

    double x, y, z;

    switch(primitive.type) {
    case PrimitiveType::PRIMITIVE_ANIMATED_MODEL:
    case PrimitiveType::PRIMITIVE_CUBE:
    case PrimitiveType::PRIMITIVE_CONE:
    case PrimitiveType::PRIMITIVE_CYLINDER:
    case PrimitiveType::PRIMITIVE_SPHERE:
        x = (ctm * glm::vec4(1, 0, 0, 0)).x;
        y = (ctm * glm::vec4(0, 1, 0, 0)).y;
        z = (ctm * glm::vec4(0, 0, 1, 0)).z;
        // add a bit of buffer space to give objects buffer to overlap
        this->org_pos = glm::vec4(0, 0, 0, 1);
        this->pos = ctm * org_pos;
        this->dim = glm::vec3((x * 0.5) + 0.01, (y * 0.5) + 0.01, (z * 0.5) + 0.01);
        break;
    case PrimitiveType::PRIMITIVE_MESH:
        this->org_pos = get_dim(primitive.meshfile).first;
        this->pos = ctm * org_pos;
        this->dim = ctm * get_dim(primitive.meshfile).second;
        break;
    }
}

void Collider::update_pos(glm::mat4 move) {
    this->pos = (this->ctm * move) * this->org_pos;
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

glm::vec3 Collider::filter_bans(glm::vec3 dir) {
    for (auto pair : this->banned_dirs) {
        if (dir.x > 0 && pair.second.x > 0.1){
            dir.x = 0;
        }
        if (dir.y > 0 && pair.second.y > 0.1){
            dir.y = 0;
        }
        if (dir.z > 0 && pair.second.z > 0.1){
            dir.z = 0;
        }

        if (dir.x < 0 && pair.second.x < -0.1){
            dir.x = 0;
        }
        if (dir.y < 0 && pair.second.y < -0.1){
            dir.y = 0;
        }
        if (dir.z < 0 && pair.second.z < -0.1){
            dir.z = 0;
        }
    }
    return dir;
}

std::optional<Collision> Collider::collides(Collider* other) {
    return this->collide_cube_to_cube(other);
}
