#ifndef __POINT_H_
#define __POINT_H_

#include "common.h"
#include "coord.h"

namespace pp {

/* A point consists of a coordinate and a mark (entity type). 
 * For safety reasons, only PointSet can allocate these. */
class Point {
public:
    const coord_t& operator[](std::size_t i) const { return coord[i]; }
    inline uint_t get_entity() const { return entity; }
    inline const Coord &get_coord() const { return coord; }
    inline double torus_squared_distance(const Point &q, double U) const { return coord.torus_squared_distance(q.coord, U); }
    inline size_t hash() const { return hash_value; }
protected:
    friend class PointSet;
    friend class boost::object_pool<Point>;
    
    Point(coord_t x, coord_t y, uint_t e) : entity(e), coord(Coord(x,y)) { 
        hash_value = coord.hash();
        boost::hash_combine(hash_value, std::hash<uint_t>{}(entity));
    }

    uint_t entity;
    Coord coord;
    bucket_t::iterator it;
    size_t bucket;
    size_t hash_value;
};

std::ostream &operator<< (std::ostream &os, const pp::Point &p) {
    return os << "Point(" << join(", ", p.get_coord().get_values()) << "; " << p.get_entity() << ")";
}

} // namespace

#endif
