#pragma once

#include "constants.h"
#include "aabb.h"

class material;

class hit_record {
    public:
        point3 p; // point of intersection
        vec3 normal; // normal vector at intersection point
        shared_ptr<material> mat;
        double t; // parameter `t` of ray at the intersection point
        bool front_face; // is the ray hitting from outside the material
        double normal_angle;
        int nbbox_hit = 0;

        void set_face_normal(const ray& r, const vec3& outward_normal) {
            // set the hit record normal vector

            front_face = dot(r.direction(), outward_normal) < 0;
            normal = front_face ? outward_normal : -outward_normal;
        }

        void set_normal_angle(const ray& r, const vec3& outward_normal) {
            // this is the unit direction vector of the ray (|ray_direction| = 1)
            vec3 ray_direction = unit_vector(r.direction());
            double angle = acos(dot(ray_direction, outward_normal));
            normal_angle = angle;
        }
};

class hittable {
    public:
        virtual ~hittable() = default;

        virtual bool hit(const ray&r, interval ray_t, hit_record& rec) const = 0;

        virtual aabb bounding_box() const = 0;
};
