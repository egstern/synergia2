#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "synergia/simulation/operator.h"
#include "synergia/simulation/step.h"
#include "synergia/simulation/lattice_simulator.h"
#include "bunch_fixture.h"
#include "lattice_fixture.h"
#include "synergia/utils/multi_array_check_equal.h"
#include "synergia/utils/boost_test_mpi_fixture.h"
BOOST_GLOBAL_FIXTURE(MPI_fixture)

const double tolerance = 1.0e-12;
const int map_order = 2;

BOOST_FIXTURE_TEST_CASE(construct, Lattice_fixture)
{
    Lattice_simulator lattice_simulator(lattice_sptr, map_order);
    Independent_operator independent_operator("test",
            lattice_simulator.get_operation_extractor_map_sptr());
}

BOOST_FIXTURE_TEST_CASE(append_slice, Lattice_fixture)
{
    Lattice_simulator lattice_simulator(lattice_sptr, map_order);
    Independent_operator independent_operator("test",
            lattice_simulator.get_operation_extractor_map_sptr());

    Lattice_element_sptr element_sptr = lattice_sptr->get_elements().front();
    double length = element_sptr->get_length();
    Lattice_element_slice_sptr first_half(
            new Lattice_element_slice(*element_sptr, 0.0, 0.5 * length));
    independent_operator.append_slice(first_half);
    Lattice_element_slice_sptr second_half(
            new Lattice_element_slice(*element_sptr, 0.5 * length, length));
    independent_operator.append_slice(second_half);
}

BOOST_FIXTURE_TEST_CASE(get_slices, Lattice_fixture)
{
    Lattice_simulator lattice_simulator(lattice_sptr, map_order);
    Independent_operator independent_operator("test",
            lattice_simulator.get_operation_extractor_map_sptr());

    Lattice_element_sptr element_sptr = lattice_sptr->get_elements().front();
    double length = element_sptr->get_length();
    Lattice_element_slice_sptr first_half(
            new Lattice_element_slice(*element_sptr, 0.0, 0.5 * length));
    independent_operator.append_slice(first_half);
    Lattice_element_slice_sptr second_half(
            new Lattice_element_slice(*element_sptr, 0.5 * length, length));
    independent_operator.append_slice(second_half);

    Lattice_element_slices slices(independent_operator.get_slices());
    BOOST_CHECK_EQUAL(slices.size(), 2);
}

BOOST_FIXTURE_TEST_CASE(apply, Bunch_fixture)
{
    Lattice_fixture l;
    Lattice_simulator lattice_simulator(l.lattice_sptr, map_order);
    Independent_operator independent_operator("test",
            lattice_simulator.get_operation_extractor_map_sptr());

    Lattice_element_sptr element_sptr;
    // find the first quad
    for (Lattice_elements::iterator it = l.lattice_sptr->get_elements().begin(); it
            != l.lattice_sptr->get_elements().end(); ++it) {
        if ((*it)->get_type() == "quadrupole") {
            element_sptr = *it;
            break;
        }
    }

    double length = element_sptr->get_length();
    Lattice_element_slice_sptr first_half(
            new Lattice_element_slice(*element_sptr, 0.0, 0.5 * length));
    independent_operator.append_slice(first_half);
    Lattice_element_slice_sptr second_half(
            new Lattice_element_slice(*element_sptr, 0.5 * length, length));
    independent_operator.append_slice(second_half);
    lattice_simulator.set_slices(independent_operator.get_slices());

    double step_length = 1.0;
    Step stub_step(1.0);

    independent_operator.apply(bunch, step_length, stub_step);
}

BOOST_FIXTURE_TEST_CASE(apply_accelerated, Bunch_fixture)
{
    Lattice_fixture l;
    Lattice_simulator lattice_simulator(l.lattice_sptr, map_order);
    Independent_operator independent_operator("test",
            lattice_simulator.get_operation_extractor_map_sptr());

    Lattice_element_sptr element_sptr;
    // find the first quad
    for (Lattice_elements::iterator it = l.lattice_sptr->get_elements().begin(); it
            != l.lattice_sptr->get_elements().end(); ++it) {
        if ((*it)->get_type() == "quadrupole") {
            element_sptr = *it;
            break;
        }
    }

    double length = element_sptr->get_length();
    Lattice_element_slice_sptr first_half(
            new Lattice_element_slice(*element_sptr, 0.0, 0.5 * length));
    independent_operator.append_slice(first_half);
    Lattice_element_slice_sptr second_half(
            new Lattice_element_slice(*element_sptr, 0.5 * length, length));
    independent_operator.append_slice(second_half);
    lattice_simulator.set_slices(independent_operator.get_slices());

    double step_length = 1.0;
    Step stub_step(1.0);

    Bunch orig_bunch(bunch);

    independent_operator.apply(orig_bunch, step_length, stub_step);
    independent_operator.apply(bunch, step_length, stub_step);

    multi_array_check_equal(bunch.get_local_particles(),
            orig_bunch.get_local_particles(), tolerance);

    independent_operator.apply(orig_bunch, step_length, stub_step);

    double old_total_energy = bunch.get_reference_particle().get_total_energy();
    bunch.get_reference_particle().set_total_energy(old_total_energy * 2.0);
    independent_operator.apply(bunch, step_length, stub_step);

    bool equal = true;
    MArray2d_ref p(bunch.get_local_particles());
    MArray2d_ref op(orig_bunch.get_local_particles());
    // Verify that the first particle is different
    for (int comp = 0; comp < 6; ++comp) {
        if (!floating_point_equal(p[0][comp], op[0][comp], tolerance)) {
            equal = false;
        }
    }
    BOOST_CHECK(!equal);
}

BOOST_FIXTURE_TEST_CASE(apply_modified_lattice, Bunch_fixture)
{
    std::cout << "jfa: start apply_modified_lattice\n";
    Lattice_fixture l;
    Lattice_simulator lattice_simulator(l.lattice_sptr, map_order);
    Independent_operator independent_operator("test",
            lattice_simulator.get_operation_extractor_map_sptr());

    Lattice_element_sptr element_sptr;
    // find the first quad
    for (Lattice_elements::iterator it = l.lattice_sptr->get_elements().begin(); it
            != l.lattice_sptr->get_elements().end(); ++it) {
        if ((*it)->get_type() == "quadrupole") {
            element_sptr = *it;
            break;
        }
    }

    double length = element_sptr->get_length();
    Lattice_element_slice_sptr first_half(
            new Lattice_element_slice(*element_sptr, 0.0, 0.5 * length));
    independent_operator.append_slice(first_half);
    Lattice_element_slice_sptr second_half(
            new Lattice_element_slice(*element_sptr, 0.5 * length, length));
    independent_operator.append_slice(second_half);
    lattice_simulator.set_slices(independent_operator.get_slices());

    double step_length = 1.0;
    Step stub_step(1.0);

    Bunch orig_bunch(bunch);

    std::cout << "jfa: bunch " << bunch.get_local_particles()[0][0] << " "
            << bunch.get_local_particles()[0][1] << " "
            << bunch.get_local_particles()[0][2] << " "
            << bunch.get_local_particles()[0][3] << " "
            << bunch.get_local_particles()[0][4] << " "
            << bunch.get_local_particles()[0][5] << " " << std::endl;
    std::cout << "jfa: orig_bunch " << orig_bunch.get_local_particles()[0][0]
            << " " << orig_bunch.get_local_particles()[0][1] << " "
            << orig_bunch.get_local_particles()[0][2] << " "
            << orig_bunch.get_local_particles()[0][3] << " "
            << orig_bunch.get_local_particles()[0][4] << " "
            << orig_bunch.get_local_particles()[0][5] << " " << std::endl;
    independent_operator.apply(orig_bunch, step_length, stub_step);
    independent_operator.apply(bunch, step_length, stub_step);
    multi_array_check_equal(bunch.get_local_particles(),
            orig_bunch.get_local_particles(), tolerance);

    std::cout << "jfa: a bunch " << bunch.get_local_particles()[0][0] << " "
            << bunch.get_local_particles()[0][1] << " "
            << bunch.get_local_particles()[0][2] << " "
            << bunch.get_local_particles()[0][3] << " "
            << bunch.get_local_particles()[0][4] << " "
            << bunch.get_local_particles()[0][5] << " " << std::endl;
    std::cout << "jfa: a orig_bunch " << orig_bunch.get_local_particles()[0][0]
            << " " << orig_bunch.get_local_particles()[0][1] << " "
            << orig_bunch.get_local_particles()[0][2] << " "
            << orig_bunch.get_local_particles()[0][3] << " "
            << orig_bunch.get_local_particles()[0][4] << " "
            << orig_bunch.get_local_particles()[0][5] << " " << std::endl;

    independent_operator.apply(orig_bunch, step_length, stub_step);

    double old_k1 = element_sptr->get_double_attribute("k1");
    element_sptr->set_double_attribute("k1", 10*old_k1);
    lattice_simulator.update();
    independent_operator.apply(bunch, step_length, stub_step);

    std::cout << "jfa: a2 bunch " << bunch.get_local_particles()[0][0] << " "
            << bunch.get_local_particles()[0][1] << " "
            << bunch.get_local_particles()[0][2] << " "
            << bunch.get_local_particles()[0][3] << " "
            << bunch.get_local_particles()[0][4] << " "
            << bunch.get_local_particles()[0][5] << " " << std::endl;
    std::cout << "jfa: a2 orig_bunch " << orig_bunch.get_local_particles()[0][0]
            << " " << orig_bunch.get_local_particles()[0][1] << " "
            << orig_bunch.get_local_particles()[0][2] << " "
            << orig_bunch.get_local_particles()[0][3] << " "
            << orig_bunch.get_local_particles()[0][4] << " "
            << orig_bunch.get_local_particles()[0][5] << " " << std::endl;


    bool equal = true;
    MArray2d_ref p(bunch.get_local_particles());
    MArray2d_ref op(orig_bunch.get_local_particles());
    // Verify that the first particle is different
    for (int comp = 0; comp < 6; ++comp) {
        if (!floating_point_equal(p[0][comp], op[0][comp], tolerance)) {
            equal = false;
        }
    }
    BOOST_CHECK(!equal);
}
