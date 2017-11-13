""" A module for generation parameter combinations. Not very pretty but works """
import itertools
import copy

def parameter_combinations(d):
    """ Generate different parameter combinations.
        Parameter 'd' is a dictionary consisting of (key,list) pairs. The list contains
        all the different values for 'key'. """
    # expand into (key, value) list
    kvls = [[(k, l) for l in d[k]] for k in d.keys()] 

    for vals in itertools.product(*kvls):
        p = dict()
        for (key,value) in vals:
            p[key] = value
        yield p
    raise StopIteration()

def generate_cases(specification):
    combinations = specification["combine"]
    alternatives = specification["alternate"]
    for pc in parameter_combinations(combinations):
        for d in alternatives:
            p = copy.deepcopy(pc)
            p.update(d)
            yield p
