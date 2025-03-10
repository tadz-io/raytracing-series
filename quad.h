#pragma once

#include "hittable.h"

class quad: public hittable {
    public:
        quad(const point3& Q, const vec3& u, const vec3& v, shared_ptr<material> mat)
            : Q(Q), u(u), v(v), mat(mat) 
            {
                auto n = cross(u, v);
                normal = unit_vector(n);
                D = dot(normal, Q);
                w = n / dot(n, n);
                
                set_bounding_box();
            }

        virtual void set_bounding_box() {
            auto bbox_diagonal1 = aabb(Q, Q + u + v);
            auto bbox_diagonal2 = aabb(Q + u, Q + v);
            bbox = aabb(bbox_diagonal1, bbox_diagonal2);
        }

        aabb bounding_box() const override { return bbox; }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
            auto denom = dot(normal, r.direction());

            // plane is approximately parallel to the ray's direction
            if (std::abs(denom) < 1e-8)
                return false;

            // return false if hit point t is outside of ray interval
            auto t = (D - dot(normal, r.origin())) / denom;
            if (!ray_t.contains(t))
                return false;

            auto intersection = r.at(t);
            vec3 PQ  = intersection - Q;
            auto alpha = dot(cross(PQ, v), w);
            auto beta  = dot(cross(u, PQ), w);

            if (!is_interior(alpha, beta, rec))
                return false;

            rec.t = t;
            rec.p = intersection;
            rec.mat = mat;
            rec.set_face_normal(r, normal);

            return true;
        }

        virtual bool is_interior(double alpha, double beta, hit_record& rec) const {
            interval unit_interval = interval(0, 1);

            if (!unit_interval.contains(alpha) || !unit_interval.contains(beta))
                return false;

            rec.u = alpha;
            rec.v = beta;
            return true;
        }

    private:
        point3 Q;
        vec3 u, v;
        vec3 w;
        shared_ptr<material> mat;
        aabb bbox;
        vec3 normal;
        double D;


};

class triangle: public quad {
    public:
        triangle(const point3& Q, const vec3& u, const vec3& v, shared_ptr<material> mat)
            : quad(Q, u, v, mat) {}

       bool is_interior(double alpha, double beta, hit_record& rec) const override {
            if (!((alpha > 0) && (beta > 0) && (alpha + beta < 1)))
                return false;
            
            rec.u = alpha;
            rec.v = beta;
            return true;    
        }
};
