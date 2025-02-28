#pragma once

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>

// c++ std usings
using std::make_shared;
using std::shared_ptr;

// constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// utility functions
inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    // returns a random real in [0,1)
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    // returns a random real in [min, max)
    return min + (max-min)*random_double();
}

inline int random_int(int min, int max) {
    // returns a random integer in [min,max]
    return int(random_double(min, max+1));
}

// common headers
#include "color.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"
