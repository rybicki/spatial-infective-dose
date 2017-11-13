#include <vector>
#include <cmath>
#include <iostream>
#include <cassert>

#include "common.h"

#ifndef __ACCUMULATOR_H_
#define __ACCUMULATOR_H_

namespace pp {

template<typename W>
class Accumulator {
    public:
    Accumulator(unsigned int c, unsigned int d) : codes(c), depth(d),
                                                  range_length(int(ceil(c/pow(2,d)))), 
                                                  leaves_start_at(pow(2,d)-1), 
                                                  max_value(int(ceil(c/pow(2,d)))*pow(2,d)),
                                                  nodes(std::vector<W>(pow(2,d+1)-1,0))
    { }
    
    void increment(unsigned int c, W weight) {
        unsigned int index = 0; // start from root
        unsigned int min = 0;
        unsigned int max = max_value; 
        nodes[index] += weight;
        while (!is_leaf(index)) {
            if (c < (min+max)/2) {
                index = left_child(index);
                max = (min+max)/2;
            } else {
                index = right_child(index);
                min = (min+max)/2;
            }
            nodes[index] += weight;
        }
        DMSG("code " << c << " put into " << index);
    }

    /* Find a maximal index for a code whose predecessor codes together have 
     * at least 'weight_to_skip' weight total. 
     * Return the a pair (c,w), where 
     *  - c gives the smallest code found and  
     *  - w gives how much weight needs to be skipped starting from c to reagh the target 'weight_to_skip'.
     */
    std::pair<unsigned int, W> find_start_location(W weight_to_skip) const {
        uint_t index = 0; // start from root
        W remaining_weight = weight_to_skip;
        while (!is_leaf(index)) {
            assert(remaining_weight <= nodes[index]);
            uint_t lc = left_child(index);
            if (nodes[lc] >= remaining_weight) { 
                index = left_child(index);
            } else {
                remaining_weight -= nodes[lc]; 
                index = right_child(index);
            }
        }
        
        assert(is_leaf(index));
        unsigned int start_location = (index - leaves_start_at)*range_length;
        assert(start_location >= 0 && start_location < codes);
        return std::pair<unsigned int, W>(start_location, remaining_weight);
    }

    inline const std::vector<W> &get_nodes() const { return nodes; }
    inline unsigned int left_child(unsigned int index) const { return 2*index+1; }
    inline unsigned int right_child(unsigned int index) const { return 2*(index+1); }
    inline bool is_leaf(unsigned int index) const { return index >= leaves_start_at; }

    const unsigned int codes, // total number of codes
                       depth, // depth of the tree
                       range_length, // 
                       leaves_start_at, // index from which nodes contains leaves
                       max_value; // number of codes rounded up 

    inline const std::vector<W> leaves() const {
        auto start = nodes.begin() + leaves_start_at;
        return std::vector<W>(start, nodes.end());
    }
protected:
    std::vector<W> nodes;
};


template<typename W>
std::ostream &operator<< (std::ostream &os, const Accumulator<W> &a) {
    os << "Accumulator(";
    os << "codes=" << a.codes << " depth=" << a.depth << " range_length=" << a.range_length;
    os << " nodecount=" << a.get_nodes().size() << " leaves_start_at " << a.leaves_start_at << ")";
    return os;
}

} // namespace

#endif 
