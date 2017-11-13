/**
 * Process definitions.
 */
#ifndef __PROCESS_DEFS_H_
#define __PROCESS_DEFS_H_

#include <string>
#include <sstream>

namespace pp {

class Immigration : public Process<0,1> {
public:
    Immigration(uint_t entity, double r) : Process<0,1>({}, {{entity}}, 0), rate(r) {}
    
    double propensity(SimulationState &s) const { return rate * s.area(); }

    void activate(SimulationState &s, point_del_buf_t &removed, point_add_buf_t &added)  { 
        added.push_back(s.new_point(s.random_coord(), output(0)));
    }

protected:
    double rate;
};

class DensityIndependentDeath : public Process<1,0> {
public:
    DensityIndependentDeath(uint_t entity, double r) : Process<1,0>({{entity}}, {}, 0), rate(r) {}
    double propensity() const { return rate; } 
    void activate(SimulationState &s, Point *p, point_del_buf_t &removed, point_add_buf_t &added)  { 
        removed.push_back(p);
    }

private:
    double rate;
};

class ChangeInType : public Process<1,1> {
public:
    ChangeInType(uint_t source, uint_t target, double r) : Process<1,1>({{source}}, {{target}}, 0), rate(r) {}
    double propensity() const { return rate; } 
    void activate(SimulationState &s, Point *p, point_del_buf_t &removed, point_add_buf_t &added)  { 
        removed.push_back(p);
        added.push_back(s.new_point(p->get_coord(), output(0)));
    }
private:
    double rate;
};

template<typename K>
class Jump : public Process<1,1> {
public:
    template<typename... Args>
    Jump(uint_t entity, const Args&... args) : Process<1,1>({{entity}}, {{entity}}, 0), kernel(K(args...)) { }

    double propensity() const { return kernel.integral; }     

    void activate(SimulationState &s, Point *p, point_del_buf_t &removed, point_add_buf_t &added)  { 
        removed.push_back(p);
        auto target = kernel.sample_around_w(s.rng(), p->get_coord(), s.U());
        added.push_back(s.new_point(target, output(0)));
    }
private:
    K kernel;
};

template<typename K>
class Birth : public Process<1,1> {
public:

    template<typename... Args>
    Birth(uint_t parent, uint_t child, const Args&... args) : Process<1,1>({{parent}},{{child}},0), kernel(K(args...)) { 
        in_radius = kernel.radius;
    }

    double propensity() const {
        return kernel.integral;
    }

    void activate(SimulationState &s, Point *p, point_del_buf_t &removed, point_add_buf_t &added)  { 
        auto target = kernel.sample_around_w(s.rng(), p->get_coord(), s.U());
        added.push_back(s.new_point(target, output(0)));

    }
private:
    K kernel;
};

template<typename K>
class Consume : public Process<2,0> {
public:

    template<typename... Args>
    Consume(uint_t consumer, uint_t resource, const Args&... args) : Process<2,0>({{consumer,resource}},{},0), kernel(K(args...)) { 
        in_radius = kernel.radius;
    }
 
    double propensity() const {
        return kernel.value;
    }

    double propensity(SimulationState &s, Point &p, Point &q) const {
        auto d = p.torus_squared_distance(q, s.U());
        return kernel.get_value_squared(d);
    }

    void activate(SimulationState &s, const Configuration &c, point_del_buf_t &removed, point_add_buf_t &added)  { 
        removed.push_back(c[1]);
    }
private:
    K kernel;
};

template<typename K>
class ChangeInTypeByFacilitation : public Process<2,1> {
public:
    template<typename... Args>
    ChangeInTypeByFacilitation(uint_t source, uint_t facilitator, uint_t target, const Args&... args) : Process<2,1>({{source,facilitator}},{{target}},0), kernel(K(args...)) { 
        in_radius = kernel.radius;
    }

    double propensity() const {
        return kernel.value;
    }

    double propensity(SimulationState &s, Point &p, Point &q) const {
        auto d = p.torus_squared_distance(q, s.U());
        return kernel.get_value_squared(d);
    }

    void activate(SimulationState &s, const Configuration &c, point_del_buf_t &removed, point_add_buf_t &added)  { 
        removed.push_back(c[0]);
        auto target = kernel.sample_around_w(s.rng(), c[0]->get_coord(), s.U());
        added.push_back(s.new_point(target, output(0)));
    }

private:
    K kernel;
};

template<typename K>
class ChangeInTypeByConsumption : public Process<2,1> {
public:
    template<typename... Args>
    ChangeInTypeByConsumption(uint_t source, uint_t resource, uint_t target, const Args&... args) : Process<2,1>({{source,resource}},{{target}},0), kernel(K(args...)) { 
        in_radius = kernel.radius;
    }

    double propensity() const {
        return kernel.value;
    }

    double propensity(SimulationState &s, Point &p, Point &q) const {
        auto d = p.torus_squared_distance(q, s.U());
        return kernel.get_value_squared(d);
    }

    void activate(SimulationState &s, const Configuration &c, point_del_buf_t &removed, point_add_buf_t &added)  { 
        removed.push_back(c[0]);
        removed.push_back(c[1]);        
        auto target = kernel.sample_around_w(s.rng(), c[0]->get_coord(), s.U());
        added.push_back(s.new_point(target, output(0)));
    }
    
    std::string info() const {
        std::stringstream s;
        s << "ChangeInTypeByConsumption(" << input(0) << ", " << input(1) << ", " << ", " << output(0) << ", kernel=" << kernel <<")";
        return s.str();
    }
private:
    K kernel;
};

template<typename K>
class BirthByConsumption : public Process<2,1> {
public:

    template<typename... Args>
    BirthByConsumption(uint_t parent, uint_t resource, uint_t child, const Args&... args) : Process<2,1>({{parent,resource}},{{child}},0), kernel(K(args...)) { 
        in_radius = kernel.radius;
        assert(input(0) == parent);
        assert(input(1) == resource);
        assert(output(0) == child);
    }

    double propensity() const {
        return kernel.value;
    }

    double propensity(SimulationState &s, Point &p, Point &q) const {
        auto d = p.torus_squared_distance(q, s.U());
        return kernel.get_value_squared(d);
    }

    void activate(SimulationState &s, const Configuration &c, point_del_buf_t &removed, point_add_buf_t &added)  { 
        removed.push_back(c[1]);
        auto target = kernel.sample_around_w(s.rng(), c[0]->get_coord(), s.U());
        added.push_back(s.new_point(target, output(0)));
    }

    
    std::string info() const {
        std::stringstream s;
        s << "BirthByConsumption(" << input(0) << ", " << input(1) << ", " << output(0) << ", kernel=" << kernel <<")";
        return s.str();
    }
private:
    K kernel;
};

}

#endif
