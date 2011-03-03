#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "synergia/simulation/propagator.h"
#include "synergia/simulation/lattice_simulator.h"
#include "synergia/foundation/physical_constants.h"
#include "synergia/lattice/chef_utils.h"
#include "synergia/bunch/bunch.h"
#include "lattice_fixture.h"
#include "synergia/utils/boost_test_mpi_fixture.h"
BOOST_GLOBAL_FIXTURE(MPI_fixture)

const double tolerance = 1.0e-12;

BOOST_FIXTURE_TEST_CASE(construct, Lattice_fixture)
{
    Dummy_collective_operator_sptr space_charge(new Dummy_collective_operator(
            "space_charge"));
    Lattice_simulator lattice_simulator(lattice_sptr, 2);

    Split_operator_stepper_sptr stepper_sptr(new Split_operator_stepper(
            lattice_simulator, space_charge, 7));

    Propagator propagator(stepper_sptr);
}

BOOST_FIXTURE_TEST_CASE(propagate, Lattice_fixture)
{
    Dummy_collective_operator_sptr space_charge(new Dummy_collective_operator(
            "space_charge"));
    Lattice_simulator lattice_simulator(lattice_sptr, 2);

    Split_operator_stepper_sptr stepper_sptr(new Split_operator_stepper(
            lattice_simulator, space_charge, 4));
    Propagator propagator(stepper_sptr);

    Diagnostics_basic_sptr diagnostics_sptr(new Diagnostics_basic);
    Diagnostics_writer per_step_diagnostics("test_propagate_per_step.h5",
            diagnostics_sptr);
    Diagnostics_full2_sptr diagnostics_full2_sptr(new Diagnostics_full2);
    Diagnostics_writer per_turn_diagnostics("test_propagate_per_turn.h5",
            diagnostics_full2_sptr);
    int num_turns = 4;
    propagator.propagate(b.bunch, num_turns, per_step_diagnostics,
            per_turn_diagnostics);
}

BOOST_FIXTURE_TEST_CASE(propagate2, Lattice_fixture)
{
    Dummy_collective_operator_sptr space_charge(new Dummy_collective_operator(
            "space_charge"));
    Lattice_simulator lattice_simulator(lattice_sptr, 2);

    Split_operator_stepper_sptr stepper_sptr(new Split_operator_stepper(
            lattice_simulator, space_charge, 4));
    Propagator propagator(stepper_sptr);

    Diagnostics_basic_sptr diagnostics_sptr(new Diagnostics_basic);
    Diagnostics_writer_sptr per_step_diagnostics1(new Diagnostics_writer(
            "test_propagate_per_step1.h5", diagnostics_sptr));
    Diagnostics_full2_sptr diagnostics_full2_sptr(new Diagnostics_full2);
    Diagnostics_writer_sptr per_step_diagnostics2(new Diagnostics_writer(
            "test_propagate_per_step2.h5", diagnostics_full2_sptr));
    Multi_diagnostics_writer multi_diagnostics_writer_step;
    multi_diagnostics_writer_step.append(per_step_diagnostics1);
    multi_diagnostics_writer_step.append(per_step_diagnostics2);

    Diagnostics_basic_sptr diagnostics_sptr_t(new Diagnostics_basic);
    Diagnostics_writer_sptr per_turn_diagnostics1(
            new Diagnostics_writer("test_propagate_per_turn1.h5",
                    diagnostics_sptr_t));
    Diagnostics_full2_sptr diagnostics_full2_sptr_t(new Diagnostics_full2);
    Diagnostics_writer_sptr per_turn_diagnostics2(
            new Diagnostics_writer("test_propagate_per_turn2.h5",
                    diagnostics_full2_sptr_t));
    Multi_diagnostics_writer multi_diagnostics_writer_turn;
    multi_diagnostics_writer_turn.append(per_turn_diagnostics1);
    multi_diagnostics_writer_turn.append(per_turn_diagnostics2);

    int num_turns = 4;
    propagator.propagate(b.bunch, num_turns, multi_diagnostics_writer_step,
            multi_diagnostics_writer_turn);
}

