

#include "synergia/simulation/propagator.h"
#include "synergia/foundation/physical_constants.h"
#include "synergia/bunch/populate.h"
#include "synergia/bunch/diagnostics_track.h"
#include "synergia/bunch/diagnostics_bulk_track.h"
#include "synergia/lattice/madx_reader.h"

#include "synergia/collective/space_charge_2d_open_hockney.h"


int run()
{
    Logger screen(0, LoggerV::DEBUG);

    MadX_reader reader;
    auto lattice = reader.get_lattice("machine", "sis18.madx");
    auto const & ref = lattice.get_reference_particle();

    //lattice.print(screen);
    screen << "reference momentum = " << ref.get_momentum() << " GeV\n";

    // space charge
    Space_charge_2d_open_hockney_options sc_ops(32, 32, 128);
    sc_ops.comm_group_size = 1;

    // stepper
    Split_operator_stepper_elements stepper(1, sc_ops);

    // Propagator
    Propagator propagator(lattice, stepper);
    //propagator.print_steps(screen);

    // bunch simulator
    auto sim = Bunch_simulator::create_single_bunch_simulator(
            lattice.get_reference_particle(), 1024 * 1024 * 1, 1e13,
            Commxx() );

    auto & bunch = sim.get_bunch();

#if 1
    // populate particle data
    karray1d means("means", 6);
    for (int i=0; i<6; ++i) means(i) = 0.0;

    karray2d covariances("covariances", 6, 6);
    for (int i=0; i<6; ++i)
        for (int j=0; j<6; ++j)
            covariances(i, j) = 0.0;

    covariances(0,0) = 1e-2;
    covariances(1,1) = 1e-2;
    covariances(2,2) = 1e-2;
    covariances(3,3) = 1e-2;
    covariances(4,4) = 1e-2;
    covariances(5,5) = 1e-2;

    Random_distribution dist(5, Commxx());
    populate_6d(dist, bunch, means, covariances);
#endif

#if 0
    // or read from file
    bunch.read_file("turn_particles_0000.h5");
#endif

    // print
    bunch.checkout_particles();
    auto local_num = bunch.get_local_num();
    auto hparts = bunch.get_host_particles();

    double sum = 0;
    for(int p=0; p<bunch.get_local_num(); ++p)
        for(int i=0; i<6; ++i) sum += hparts(p, i);

    screen(LoggerV::DEBUG) 
        << std::setprecision(8)
        << "\n\npopulated sum = " << sum << "\n";

    for (int p=0; p<4; ++p) bunch.print_particle(p, screen);
    screen << "\n";

#if 0
    // diagnostics
    Diagnostics_track diag_track(2, "part_2_track.h5");
    sim.reg_diag_per_turn("track_2", diag_track);

    Diagnostics_bulk_track diag_bulk_track(6, 0, "bulk_track.h5");
    sim.reg_diag_per_turn("bulk_track", diag_bulk_track);
#endif

    // propagate options
    sim.set_turns(0, 1); // (start, num_turns)

    // propagate
    double t0 = MPI_Wtime();
    propagator.propagate(sim, screen);
    double t1 = MPI_Wtime();
    screen <<"propagate time = " << t1-t0 << "\n";

    // print particles after propagate
    bunch.checkout_particles();

    sum = 0;
    for(int p=0; p<bunch.get_local_num(); ++p)
        for(int i=0; i<6; ++i) sum += hparts(p, i);

    screen(LoggerV::DEBUG) 
        << std::setprecision(8)
        << "\n\npropagated sum = " << sum << "\n";

    for (int p=0; p<4; ++p) bunch.print_particle(p, screen);

    double g_sum = 0;
    MPI_Reduce(&sum, &g_sum, 1, MPI_DOUBLE, MPI_SUM, 0, bunch.get_comm());

    screen(LoggerV::DEBUG) 
        << std::setprecision(8)
        << "\n\npropagated sum (reduced) = " << g_sum << "\n";

    screen << "\n";

    return 0;
}


int main(int argc, char ** argv)
{
    MPI_Init(&argc, &argv);
    Kokkos::initialize(argc, argv);

    run();

    Kokkos::finalize();
    MPI_Finalize();
    return 0;
}

