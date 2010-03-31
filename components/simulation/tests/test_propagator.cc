#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "components/simulation/propagator.h"
#include "components/foundation/physical_constants.h"
#include "components/lattice/chef_utils.h"

const std::string name("foo");
const double mass = constants::mp;
const double total_energy = 125.0;
const double tolerance = 1.0e-12;
const double quad_length = 0.2;
const double drift_length = 3.0;
const double bend_length = 4.0;

BOOST_AUTO_TEST_CASE(construct)
{

    Four_momentum four_momentum(mass, total_energy);
    Reference_particle reference_particle(four_momentum);
    Lattice_element f("quadrupole", "f");
    f.set_double_attribute("l", quad_length);
    Lattice_element o("drift", "o");
    o.set_double_attribute("l", drift_length);
    Lattice_element d("quadrupole", "d");
    d.set_double_attribute("l", quad_length);

    Lattice lattice(name);
    lattice.append(f);
    lattice.append(o);
    lattice.append(d);
    lattice.append(o);
    lattice.set_reference_particle(reference_particle);

    Collective_operator_sptr space_charge(new Collective_operator(
            "space_charge"));

    Split_operator_stepper stepper7(lattice, 7, space_charge);
//    stepper7.print();

    Propagator propagator(stepper7);
//    std::cout << "original\n";
//    print_chef_beamline(propagator.get_chef_lattice().get_beamline_sptr());
//    std::cout << "sliced\n";
//    print_chef_beamline(propagator.get_chef_lattice().get_sliced_beamline_sptr());
}

