#ifndef __KERNELS_H_
#define __KERNELS_H_

#define _USE_MATH_DEFINES
#include <cmath>
#include <random>

#include "point.h"
#include "common.h"

namespace pp {

/* The top hat kernel implementation */
class Tophat {
public:
    Tophat(coord_t total_integral, coord_t max_radius) : integral(total_integral), 
                                                         radius(max_radius), 
                                                         radius_squared(radius*radius),
                                                         value(total_integral / (radius_squared*M_PI)) { }

    inline coord_t get_value_squared(coord_t distance_squared) const {
        if (distance_squared <= radius_squared) {
            return value;
        } else {
            return 0;
        }
    }

    /* sample around origo (0,0) */
    inline Coord sample(rng_t &g) const { 
        coord_t r = uniform_distribution(g);
        coord_t t = uniform_distribution(g)*2*M_PI;
        coord_t x = sqrt(r) * cos(t);
        coord_t y = sqrt(r) * sin(t);
        return Coord(x*radius, y*radius);
    }

    /* give a randomly sampled point where origin is given by p */
    inline Coord sample_around(rng_t& g, const Coord &p) { 
        auto deltaloc = sample(g);
        return deltaloc+p;
    }
    
    /* Randomly sample around p but wrap coords using U */
    inline Coord sample_around_w(rng_t& g, const Coord &p, coord_t U) { 
        auto q = sample_around(g, p);
        q.wrap(U);
        return q;
    } 


    const coord_t integral; /* integral of the kernel over the whole domain */
    const coord_t radius; /* radius (support) of the kernel */
    const coord_t radius_squared; /* radius squared */
    const coord_t value; /* the value of the tophat kernel if within truncation distance */
};

std::ostream &operator<< (std::ostream &os, const Tophat &k) {
    os << "Tophat(" << k.integral << ", " << k.radius << ")";
    return os;
}

} // namespace

#endif
