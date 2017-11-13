#define CATCH_CONFIG_MAIN  
#include "catch.hpp"

#include "../pp.h"

/**
 * Some basic tests for the data structures 
 **/

TEST_CASE( "accumulator with a bucket for each node", "[accumulator]" ) {
    constexpr int codes = 32;
    int depth = log2(codes);

    pp::Accumulator<int> a(codes, depth);

    SECTION(" Accumulator has correct amount of leaves ") {
        REQUIRE( a.leaves().size() == codes );
    }

    SECTION("Increments & decrements & finds") {
        for(int i = 0; i<codes; i++) {
            a.increment(i,1);
            REQUIRE( a.get_nodes()[0] == i+1);
        }

        //std::cout << pp::join(" ", a.leaves() ) << std::endl;

        auto r = a.find_start_location(0);
        REQUIRE( r.first == 0 );
        REQUIRE( r.second == 0);

        /* We should have each value in its own bucket */
        for(int w = 1; w<=a.get_nodes()[0]; w++) {
            auto result = a.find_start_location(w);
            //std::cout << "w=" << w << " " << result.first << " " << result.second << std::endl;
            REQUIRE( result.first < codes );
            REQUIRE( result.second == 1 );
        }

        for(int i = 0; i<codes; i++) {
            a.increment(i,-1);
            REQUIRE( a.get_nodes()[0] == codes-1-i);
        }
    }
}

rng_t rng_instance;
std::vector<double> random_values(double max, int n) {
    std::vector<double> vs;
    for(int i = 0; i<n; i++) {
        vs.push_back(uniform_distribution(rng_instance) * max);
    }
    return vs;
}

TEST_CASE( "point set", "[pointset]" ) {
    double U = 20;
    double bw = 1; // FIXME: variable U and bw and N?
    int N = 2000;
    int POINT_TYPE = 1;

    pp::PointSet ps(U,bw);

    REQUIRE(ps.get_count() == 0);

    auto xs = random_values(U, N);
    auto ys = random_values(U, N);

    SECTION("Allocations & deallocations") {
        std::set<pp::Point*> points;
        for(auto i = 0; i<N; i++) {
            auto x = xs[i];
            auto y = ys[i];
            auto *p = ps.new_point(x,y,POINT_TYPE);
            REQUIRE((*p)[0] == x);
            REQUIRE((*p)[1] == y);
            REQUIRE(p->get_entity() == POINT_TYPE);
            REQUIRE(!ps.contains(p));
            
            ps.add(p);
            REQUIRE(ps.contains(p));

            points.insert(p);
        }

        SECTION("Get nth") {
            std::set<pp::Point*> found_points;
            for(auto i = 0; i<N; i++) {
                auto p = ps.get_nth(i);
                REQUIRE(points.find(p) != points.end());
                found_points.insert(p);
            }

            /* Check that we picked up all points along the way */
            REQUIRE(found_points.size() == points.size());
            for(auto p : points) {
                REQUIRE(found_points.find(p) != found_points.end());
            }
        }

        SECTION("Destroy points") {
            for(auto p : points) {
                REQUIRE(ps.contains(p));
                ps.destroy_point(p);
                REQUIRE(!ps.contains(p));
            }
        }

        SECTION("Distance queries") {
            double distances[] = { 0.5, 1.0, 2, 3, 4, 10, 15, 20, 100 };

            for(auto d : distances) {
                pp::point_query_t buffer, brute_buffer;
                double d_squared = d*d;

                for(auto p : points) {
                    buffer.clear();
                    brute_buffer.clear();
                    ps.get_within(p, d, buffer);

                    /* Bruteforce solution */
                    for(auto q : points) {
                        if (p->torus_squared_distance(*q,U) <= d_squared && p != q) {
                            brute_buffer.push_back(q);
                        }
                    }

                    /* Check that distances are OK */
                    for(auto q : buffer) {
                        auto dd = p->torus_squared_distance(*q, U);
                        REQUIRE(sqrt(dd) <= d);
                    }

                    /* Check that results match */
                    for(auto q : brute_buffer) {
                        auto it = std::find(buffer.begin(), buffer.end(), q);
                        REQUIRE(it != buffer.end());
                    }
                }
            }
        }
    }
}


TEST_CASE( "configurations", "[configurations]" ) {
    double U = 10;
    double bw = 1; 
    int N = 100;
    int POINT_TYPE = 1;

    /* Generate some points for the configurations */
    pp::PointSet ps(U,bw);
    auto xs = random_values(U, N);
    auto ys = random_values(U, N);
    std::set<pp::Point*> points;
    for(auto i = 0; i<N; i++) {
        auto x = xs[i];
        auto y = ys[i];
        auto *p = ps.new_point(x,y,POINT_TYPE);
        points.insert(p);
    }

    pp::ConfigurationSet<2> cs;
    double weight = 1.0;

    REQUIRE(cs.get_total_weight() == 0);
    REQUIRE(cs.size() == 0);

    std::set<pp::NConfiguration<2>*> cps; // pointers that were created
    for(auto p : points) {
        for(auto q : points) {
            if (p == q) continue;
            auto c = cs.create(weight, p, q);
            REQUIRE(!cs.contains(c));
            cps.insert(c);
        }
    }

    SECTION("Adding and removing") {
        for(auto c : cps) {
            cs.add(c);
            REQUIRE(cs.contains(c));
        }

        SECTION("Removals") {
            for(auto c : cps) {
                cs.remove(c);
                REQUIRE(!cs.contains(c));
            }
        }

        SECTION("Find & destroy") {
            for(auto c : cps) {
                auto p = c->points[0];
                auto q = c->points[1];
                
                REQUIRE(cs.contains(c));
                REQUIRE_NOTHROW(cs.find_and_destroy(p,q));
                REQUIRE(!cs.contains(c));
            }
        }

        SECTION("Get nth") {
            std::set<pp::NConfiguration<2>*> found_cs;
            for(auto i = 0; i<cps.size(); i++) {
                auto *c = cs.get_nth(i);
                found_cs.insert(c);
                bool found = cps.find(c) != cps.end();
                REQUIRE(found);
            }
            /* Check that we got all configurations this way */
            REQUIRE(found_cs.size() == cps.size());
            for(auto c : cps) {
                REQUIRE(found_cs.find(c) != found_cs.end());
            }
        }
    }

}
