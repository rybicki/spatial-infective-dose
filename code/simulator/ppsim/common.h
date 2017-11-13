#ifndef __COMMON_H_
#define __COMMON_H_

#include <string>
#include <iostream>
#include <sstream>
#include <random>
#include <list>
#include <boost/pool/pool_alloc.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/functional/hash.hpp>

/* DEBUG PRINTING */

#ifdef DEBUG_MSG
#define DMSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DMSG(str) do { } while ( false )
#endif

/*
 * RANDOM NUMBER GENERATOR.
 *
 * The PCG generator seems faster than the standard library Mersenne twister.
 */
#define USE_PCG 1
#if USE_PCG
#include "pcg_random.hpp" 
using rng_t = pcg64; 
#else
using rng_t = std::mt19937;
#endif

static std::uniform_real_distribution<double> uniform_distribution(0,1); /* NOTE: This needs to be at least double precision to avoid LWG issue 2524 */

namespace pp {

/* TYPE ALIASES AND FORWARD DECLARATIONS */
using uint_t = uint64_t; 
using coord_t = double; 
class Point;

/* BUFFER TYPES & MEMORY POOLING */
using pool_allocator_t = boost::default_user_allocator_new_delete;

/*
 * You can increase the performance slightly by disabling the mutex in the pool allocator.
 * However, in this case you cannot have any threads running in the process (accessing the pool).
 */
#define DISABLE_ALLOC_MUTEX 1
#if DISABLE_ALLOC_MUTEX
#define allocated_structure(x,y) x<y,boost::fast_pool_allocator<y,pool_allocator_t>>
#else
#define allocated_structure(x,y) x<y,boost::fast_pool_allocator<y,pool_allocator_t,boost::details::pool::null_mutex>>
#endif

using bucket_t = allocated_structure(std::list,Point*);
using bucket_list_t =  allocated_structure(std::vector,bucket_t);
using point_query_t = allocated_structure(std::vector,Point*);
using point_del_buf_t = allocated_structure(std::vector,Point*);
using point_add_buf_t = allocated_structure(std::vector,Point*);
using point_enum_buf_t = allocated_structure(std::vector,Point*);

/* CONVENIENCE METHODS */

template<typename T>
inline T wrap_coord(T x, T max) {
    if (x < 0) x = max + x;
    else if (x >= max) x = x - max;
    return x;
}

template<class S, class V>
std::string join(const S& sep, const V& v)
{
  std::ostringstream oss; 
  oss << std::fixed << std::setprecision(10);
  if (!v.empty()) {
    auto it = v.begin();
    oss << *it++;
    for (auto e = v.end(); it != e; ++it)
      oss << sep << *it;
  }
  return oss.str();
}

/*
 * Kahan summation for more precise summing of doubles..
 */ 
double sum_kh(const std::vector<double> values) {
    double sum = values[0];
    double correction = 0; // compensation
    for(int i = 1; i<values.size(); i++) {
        double y = values[i] - correction;
        double t = sum + values[i];
        correction = (t - sum) - y;
        sum = t;
    }
    return sum;
}

}


#endif
