#pragma once

#include "ray.h"

class hit_record {
    public:
        point3 p; // point of intersection
        vec3 normal; // normal vector at intersection point
        double t; // parameter `t` of ray at the intersection point
        bool front_face; // is the ray hitting from outside the material

        void set_face_normal(const ray& r, const vec3& outward_normal) {
            // set the hit record normal vector

            front_face = dot(r.direction(), outward_normal) < 0;
            normal = front_face ? outward_normal : -outward_normal;
        }
};

class hittable {
    public:
        virtual ~hittable() = default;

        virtual bool hit(const ray&r, double ray_tmin, double ray_tmax, hit_record& rec) const = 0;
};
