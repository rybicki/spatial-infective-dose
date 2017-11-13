#include "accumulator.h"
#include "common.h"
#include <vector>
#include <cassert>

template<typename T>
struct BruteAccumulator {
    BruteAccumulator(int c) : codes(c), locations(std::vector<T>(c,0)) {}
    
    void increment(int c, T v) { 
        locations[c] += v; 
    }

    T sum() {
        T s = 0;
        for(T x : locations) {
            s += x;
        }
        return s;
    }

    std::vector<T> locations;
    int codes;
};


template<typename T>
bool verify_contents(Accumulator<T> &a, BruteAccumulator<T> &b) {
    auto a_leaves = a.leaves();
    int i = 0;
    bool ok = true;

    for(i = 0; i<b.locations.size(); i++) {
        if (a_leaves[i] != b.locations[i]) {
            ok = false;
            break;
        }
    }
    for(auto j = i; j<a_leaves.size(); j++) {
        if (a_leaves[j] != 0)  {
            ok = false;
            break;
        }
    }

    if (!ok) {
        std::cout << "MISMATCH!" << std::endl;
        std::cout << "A: " << join(" ",a.leaves()) << std::endl;
        std::cout << "B: " << join(" ",b.locations) << std::endl;
    }
    return ok;
}

template<typename T>
bool verify(Accumulator<T> &a, BruteAccumulator<T> &b, int n) {
    // check that nth value was found correctly

    auto correct_loc = -1;
    auto correct_skip = -1;
    bool found = false;
    T x = 0;
    for(int i = 0; i < b.locations.size(); i++) {
        auto nx = x + b.locations[i];
        if (nx >= n) {
            correct_loc = i;
            correct_skip = (n-x);
            found = true;
            break;
        }
        x = nx;
    }

    auto res = a.find_start_location(n);
    auto start_location = res.first;
    auto skip = res.second;
    bool ok = (start_location == correct_loc && skip == correct_skip);
    if (!ok) {
        std::cout << "Get " << n << std::endl;
        std::cout << "A: " << start_location << " " << skip << std::endl;
        std::cout << "B: " << correct_loc << " " << correct_skip << " was found? " << found << std::endl;
        std::cout << "A: " << join(" ",a.leaves()) << std::endl;
        std::cout << "B: " << join(" ",b.locations) << std::endl;
    }
    return ok;
}


template<typename T>
bool brute_find(Accumulator<T> &a, T n) {
    auto ls = a.leaves();
    auto res = a.find_start_location(n);
    auto start_location = res.first;
    auto skip = res.second;
    auto found_loc = -1;
    auto to_skip = -1;
    T w = 0;
    /*
    for(int i = 0; i<a.codes; i++) {
        for(int j = 0; j<ls[i]; j++) {
            w++;
            if (w == n+1) {
                found_loc = i;
                to_skip = j;
                break;
            }
        }
    }*/
    bool ok = start_location == found_loc && to_skip == skip;
    if (!ok) {
        std::cout << "Skip " << n << std::endl;
        std::cout << "ARRAY: " << join(" ", ls) << std::endl;
        std::cout << "Fast: " << start_location << " " << skip << std::endl;
        std::cout << "Slow: " << found_loc << " " << to_skip << std::endl;
        std::cout << std::endl;
    }
    return ok;
}


int main() {
    constexpr int MAX_CODES = 100;
    constexpr int MAX_DEPTH = 3;
    for(int codes = 1; codes<MAX_CODES; codes++) {
    for(int depth = 1; depth<MAX_DEPTH; depth++) {
        std::cout << " == CODES " << codes << " DEPTH " << depth << " == "<< std::endl;
        Accumulator<int> a(codes, depth);
        BruteAccumulator<int> b(codes);
        for(auto i = 0; i<codes; i++) {
            assert(b.sum() == a.get_nodes()[0]);
            a.increment(i,1);
            b.increment(i,1);
            /*
            assert(b.sum() == a.get_nodes()[0]);
            assert(verify_contents(a,b));
            brute_find(a,i);*/
        }

        for(auto i = 0; i<codes; i++) {
            if (brute_find(a, i)) { } 
            else { std::cout << "FAIL " << i << std::endl; assert(true); }
        }
        for(auto i = 0; i<codes; i++) {
            auto r = a.find_start_location(i);
            DMSG(i << " -> " << "start=" << r.first << " skip=" << r.second);
        }
    }
    }
}
