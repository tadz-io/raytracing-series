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
        double u;
        double v;
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

        virtual double pdf_value(const point3& origin, const vec3& direction) const {
            return 0.0;
        }

        virtual vec3 random(const point3& origin) const {
            return vec3(1,0,0);
        }

        virtual void gather_internal_nodes(std::vector<aabb>& boxes) const {
            boxes.push_back(this->bounding_box());
        };
};

class translate : public hittable {
    public:

        translate(shared_ptr<hittable> object, const vec3& offset) 
            : object(object), offset(offset)
            {
                bbox = object->bounding_box() + offset;
            }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            ray offset_r(r.origin() - offset, r.direction());

            if (!object->hit(offset_r, ray_t, rec))
                return false;

            rec.p += offset;

            return true;
        }

        aabb bounding_box() const override { return bbox; }

    private:
        shared_ptr<hittable> object;
        vec3 offset;
        aabb bbox;
};

class rotate_y : public hittable {
    public:
        rotate_y(shared_ptr<hittable> object, double angle) : object(object) {
            auto radians = degrees_to_radians(angle);
            sin_theta = std::sin(radians);
            cos_theta = std::cos(radians);
            bbox = object->bounding_box();

            point3 min( infinity,  infinity,  infinity);
            point3 max(-infinity, -infinity, -infinity);

            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    for (int k = 0; k < 2; k++) {
                        auto x = i*bbox.x.max + (1-i)*bbox.x.min;
                        auto y = j*bbox.y.max + (1-j)*bbox.y.min;
                        auto z = k*bbox.z.max + (1-k)*bbox.z.min;

                        auto newx =  cos_theta*x + sin_theta*z;
                        auto newz = -sin_theta*x + cos_theta*z;

                        vec3 tester(newx, y, newz);

                        for (int c = 0; c < 3; c++) {
                            min[c] = std::fmin(min[c], tester[c]);
                            max[c] = std::fmax(max[c], tester[c]);
                        }
                    }
                }
            }

            bbox = aabb(min, max);
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            // transform the ray from world to object space
            auto origin = point3(
                (cos_theta * r.origin().x()) - (sin_theta * r.origin().z()),
                r.origin().y(),
                (sin_theta * r.origin().x()) + (cos_theta * r.origin().z())
            );
            auto direction = vec3(
                (cos_theta * r.direction().x()) - (sin_theta * r.direction().z()),
                r.direction().y(),
                (sin_theta * r.direction().x()) + (cos_theta * r.direction().z())
            );
    
            ray rotated_r(origin, direction);
    
            // Determine whether an intersection exists in object space (and if so, where).
    
            if (!object->hit(rotated_r, ray_t, rec))
                return false;
    
            // Transform the intersection from object space back to world space.
    
            rec.p = point3(
                (cos_theta * rec.p.x()) + (sin_theta * rec.p.z()),
                rec.p.y(),
                (-sin_theta * rec.p.x()) + (cos_theta * rec.p.z())
            );
    
            rec.normal = vec3(
                (cos_theta * rec.normal.x()) + (sin_theta * rec.normal.z()),
                rec.normal.y(),
                (-sin_theta * rec.normal.x()) + (cos_theta * rec.normal.z())
            );
    
            return true;
        }

        aabb bounding_box() const override { return bbox; }

    private:
        shared_ptr<hittable> object;
        double sin_theta;
        double cos_theta;
        aabb bbox;


};
