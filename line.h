#pragma once

#include "hittable.h"
#include "constants.h"

class line: public hittable {
    public:
        line(point3 start, point3 end, double tickness, shared_ptr<material> mat) 
            : start(start), end(end), tickness(tickness), mat(mat)  {
                point3 pmin(
                    std::min(start.x(), end.x()) - tickness,
                    std::min(start.y(), end.y()) - tickness,
                    std::min(start.z(), end.z()) - tickness);
                
                point3 pmax(
                    std::max(start.x(), end.x()) + tickness,
                    std::max(start.y(), end.y()) + tickness,
                    std::max(start.z(), end.z()) + tickness);
                
                
                bbox = aabb(pmin, pmax);
            }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override {           
            // need to check if d x b = 0 -> ?
            
            // ray:  r = o + t * d
            // line: l = a + s * b
            // b = end - start
            // a = start
            // o = origin
            // vector from origin to start line: OA = start - o
            // distance vector: dist = bxd
            // absolute distance: distance = OA⋅(dist / |dist|)
            auto b = end - start;
            auto oa = start - r.origin();
            auto dist = cross(b, r.direction());
            auto dist_unit = dist / dist.length();
            auto distance = dot(oa, dist_unit);

            // t = (oaxb ⋅ (dxb)) / |dxb|^2
            auto t = dot(cross(oa, b), -dist) / (dist.length_squared());

            if (distance < tickness) {
                rec.t = t;
                rec.p = r.at(rec.t);
                rec.normal = dist_unit;
                rec.mat = mat;
                return true;
            }
            return false;
        }

        aabb bounding_box() const override { return bbox; }

    private:
        point3 start;
        point3 end;
        double tickness;
        shared_ptr<material> mat;
        aabb bbox;
};
