#ifndef LATTICE_SIMULATOR_H_
#define LATTICE_SIMULATOR_H_

#include "synergia/lattice/lattice.h"
#include "synergia/lattice/chef_lattice.h"
#include "synergia/simulation/operation_extractor.h"
#include "synergia/simulation/step.h"
#include <string>

class Lattice_simulator
{
private:
    Lattice_sptr lattice_sptr;
    Lattice_element_slices slices;
    bool have_slices;
    Chef_lattice_sptr chef_lattice_sptr;
    Operation_extractor_map_sptr extractor_map_sptr;
    int map_order;

    void
    construct_extractor_map();
    void
    construct_sliced_chef_beamline();
public:
    Lattice_simulator(Lattice_sptr lattice, int map_order);
    void
    set_slices(Lattice_element_slices const& slices);
    int
    get_map_order() const;
    Operation_extractor_map_sptr
    get_operation_extractor_map_sptr();
    Lattice_sptr
    get_lattice_sptr();
    Chef_lattice_sptr
    get_chef_lattice_sptr();
    ~Lattice_simulator();
};

#endif /* LATTICE_SIMULATOR_H_ */
