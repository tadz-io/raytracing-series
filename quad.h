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

                area = n.length();
                
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

        double pdf_value(const point3& origin, const vec3& direction) const override {
            hit_record rec;
            if (!this->hit(ray(origin, direction), interval(0.001, infinity), rec))
                return 0.0;

            auto distance_squared = rec.t * rec.t * direction.length_squared();
            auto cosine = std::fabs(dot(direction, rec.normal) / direction.length());

            return distance_squared / (cosine * area);
        }

        vec3 random(const point3& origin) const override {
            auto p = Q + (random_double() * u) + (random_double() * v);
            return p - origin;
        }

    protected:
        point3 Q;
        vec3 u, v;
        vec3 w;
        shared_ptr<material> mat;
        aabb bbox;
        vec3 normal;
        double D;
        double area;


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

class disk: public quad {
    public:
        disk(const point3& Q, const vec3& u, const vec3& v, shared_ptr<material> mat)
            : quad(Q, u, v, mat) 
            {
                set_bounding_box();
            }
        
        virtual void set_bounding_box() override {
            auto bbox_diagonal1 = aabb(Q-u-v, Q + u + v);
            auto bbox_diagonal2 = aabb(Q-u+v, Q + u - v);
            bbox = aabb(bbox_diagonal1, bbox_diagonal2);
        }

        bool is_interior(double alpha, double beta, hit_record& rec) const override {
            
            if ((alpha*alpha + beta*beta) > 1.0)
                return false;
            // not sure if the uv coordinates are correct
            rec.u = alpha;
            rec.v = beta;
            return true;    
        }

    private:
        double radius;
};

inline shared_ptr<hittable_list> box(const point3& a, const point3& b,  shared_ptr<material> mat) {
    // returns a 3D box (six sides) that contains two opposite vertices a&b

    auto sides = make_shared<hittable_list>();

    // construct the two opposite vertices using the min and max coordinates
    auto min = point3(std::fmin(a.x(),b.x()), std::fmin(a.y(),b.y()), std::fmin(a.z(),b.z()));
    auto max = point3(std::fmax(a.x(),b.x()), std::fmax(a.y(),b.y()), std::fmax(a.z(),b.z()));

    auto dx = vec3(max.x() - min.x(), 0 , 0);
    auto dy = vec3(0, max.y() - min.y(), 0);
    auto dz = vec3(0, 0, max.z() - min.z());

    sides->add(make_shared<quad>(point3(min.x(), min.y(), max.z()),  dx,  dy, mat)); // front
    sides->add(make_shared<quad>(point3(max.x(), min.y(), max.z()), -dz,  dy, mat)); // right
    sides->add(make_shared<quad>(point3(max.x(), min.y(), min.z()), -dx,  dy, mat)); // back
    sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()),  dz,  dy, mat)); // left
    sides->add(make_shared<quad>(point3(min.x(), max.y(), max.z()),  dx, -dz, mat)); // top
    sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()),  dx,  dz, mat)); // bottom

    return sides;
}
