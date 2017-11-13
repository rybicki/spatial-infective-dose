#include <array>
#include <boost/functional/hash.hpp>

namespace pp {

template <typename T, int D>
struct DCoord {
    DCoord() {}
    DCoord(std::array<T,D> vs) : values(vs) {}

    template <typename... Vs>
    DCoord(Vs... vs) : values{{vs...}} {}

    const T& operator[](std::size_t i) const { return values[i]; }

    inline DCoord operator-(const DCoord &q) const { 
        DCoord t;
        for(int i = 0; i<D; i++) {
            t.values[i] = values[i] - q.values[i];
        }
        return t;
    }

    inline DCoord operator+(const DCoord &q) const { 
        DCoord t;
        for(int i = 0; i<D; i++) {
            t.values[i] = values[i] + q.values[i];
        }
        return t;
    }

    inline bool operator==(const DCoord &q) const { 
        for(int i = 0; i<D; i++) {
            if (values[i] != q.values[i]) return false;
        }
        return true;
    }

    inline T squared_norm() const { 
        T sum = 0;
        for(int i = 0; i<D; i++) {
            sum += values[i]*values[i];
        }
        return sum; 
    }

    inline T norm() const { return sqrt(squared_norm()); }

    inline void wrap(const T U) {
        for(int i = 0; i<D; i++) {
            values[i] = wrap_coord<coord_t>(values[i], U);
        }
    }

    inline void wrap(const DCoord &U) {
        for(int i = 0; i<D; i++) {
            values[i] = wrap_coord<coord_t>(values[i], U[i]);
        }
    }

    inline T torus_squared_distance(const DCoord &q, T U) const {
        coord_t sum = 0;
        for(int i = 0; i<D; i++) {
            coord_t t = std::abs(values[i]-q.values[i]);
            coord_t s = std::min(t, U-t);
            sum += s*s;
        }
        return sum;
    }
    
    inline T torus_squared_distance(const DCoord &q, const DCoord &U) const {
        coord_t sum = 0;
        for(int i = 0; i<D; i++) {
            coord_t t = std::abs(values[i]-q.values[i]);
            coord_t s = std::min(t, U[i]-t);
            sum += s*s;
        }
        return sum;
    }

    inline size_t hash() const { 
        size_t value = 0;
        for(auto v : values) {
            boost::hash_combine(value, std::hash<T>{}(v));
        }
        return value;
    }

    const std::array<T,D> &get_values() const { return values; }
private:
    std::array<T,D> values;
};

using Coord = DCoord<coord_t,2>;

}

