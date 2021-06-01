
#ifndef SYNERGIA_SIMULATION_LATTICE_SIMULATOR_H
#define SYNERGIA_SIMULATION_LATTICE_SIMULATOR_H

#include "synergia/lattice/lattice.h"
#include "synergia/bunch/bunch.h"
#include "synergia/libFF/ff_element.h"

#include "synergia/foundation/trigon.h"
#include "synergia/foundation/normal_form.h"

struct chromaticities_t
{
    double momentum_compaction;

    double horizontal_chromaticity;
    double horizontal_chromaticity_prime;

    double vertical_chromaticity;
    double vertical_chromaticity_prime;

    double slip_factor;
    double slip_factor_prime;
};

namespace Lattice_simulator
{
    constexpr const double default_closed_orbit_tolerance = 1.0e-13;

    void   set_closed_orbit_tolerance(double tolerance);
    double get_closed_orbit_tolerance();

    // Both tune_linear_lattice() and tune_circular_lattice() set the frequency of the 
    // rfcavities based on the momentum of the lattice reference particle.  
    //
    // tune_linear_lattice uses the state of the reference particle as the starting 
    // point for propagation.  
    //
    // tune_circular_lattice() calculates a closed orbit and uses that as the starting 
    // point.  
    //
    // Both returns return an array of state, the transverse coordinates are the final 
    // coordinates. cdt is the c * the total propagation time of the particle which 
    // gives the total path length.

    // find the closed orbit for the circular lattice, propagate the lattice reference 
    // particle through the lattice slices using the closed orbit, and set the reference 
    // c*t for each lattice slice after tuning.
    // return values is the state for calcualted closed orbit
    // note that all the rf cavities will be set to 0 strength during the tuning process
    std::array<double, 6> 
    tune_linear_lattice(
            Lattice & lattice);

    std::array<double, 6> 
    tune_circular_lattice(
            Lattice & lattice);

    std::array<double, 6> 
    tune_rfcavities(
            Lattice & lattice);

    // closed orbit
    std::array<double, 6> 
    calculate_closed_orbit(
            Lattice const& lattice, 
            double dpp = 0.0);

    // [tune_h, tune_v, c_delta_t]
    std::array<double, 3>
    calculate_tune_and_cdt(
            Lattice const& lattice, 
            double dpp);

    chromaticities_t
    get_chromaticities(
            Lattice const& lattice, 
            double dpp);

    // get the full mapping of the one turn map
    template<unsigned int order = 2>
    TMapping<Trigon<double, order, 6>>
    get_one_turn_map(
            Lattice const& lattice, 
            double dpp = 0.0 );

    // only the jacobian of the one turn map
    karray2d_row
    get_linear_one_turn_map(
            Lattice const& lattice);

    // [alpha, beta, psi]
    std::array<double, 3>
    map_to_twiss(
            karray2d_row map);

    template<unsigned int order>
    NormalForm<order>
    calculate_normal_form(
            Lattice const& lattice);

    double
    get_bucket_length(
            Lattice const& lattice);

    void
    adjust_tunes(
            Lattice& lattice,
            double horizontal_tune, 
            double vertical_tune,
            double tolerance );

}

#ifdef __CUDA_ARCH__

// no implementations for CUDA arch

#else

// implementations
namespace Lattice_simulator
{
    template<unsigned int order>
    TMapping<Trigon<double, order, 6>>
    get_one_turn_map(Lattice const& lattice, double dpp)
    {
        using trigon_t = Trigon<double, order, 6>;

        // get the reference particle
        auto const& ref = lattice.get_reference_particle();

        // closed orbit
        auto probe = Lattice_simulator::calculate_closed_orbit(lattice, dpp);

        // comm world
        Commxx comm;

        // trigon bunch to get the one-turn-map
        bunch_t<trigon_t> tb(ref, comm.size(), comm);

        // design reference particle from the closed orbit
        auto ref_l = ref;
        ref_l.set_state(probe);
        tb.set_design_reference_particle(ref_l);

        auto tparts = tb.get_host_particles();

        // init value
        for(int i=0; i<6; ++i) 
            tparts(0, i).set(probe[i], i);

        // check in
        tb.checkin_particles();

        // propagate trigon
        for(auto & ele : lattice.get_elements())
        {
            if (ele.get_type() == element_type::rfcavity)
            {
                Lattice_element dup = ele;
                //dup.set_double_attribute("volt", 0.0);

                FF_element::apply(dup, tb);
            }
            else
            {
                FF_element::apply(ele, tb);
            }
        }

        // checkout particles
        tb.checkout_particles();

        // one-turn-map
        TMapping<trigon_t> map;
        for(int i=0; i<trigon_t::dim; ++i) map[i] = tparts(0, i);

        return map;
    }


    template<unsigned int order>
    NormalForm<order>
    calculate_normal_form(Lattice const& lattice)
    {
        auto one_turn_map = get_one_turn_map<order>(lattice, 0.00);

        auto ref = lattice.get_reference_particle();
        double e0 = ref.get_total_energy();
        double pc0 = ref.get_momentum();
        double mass = ref.get_mass();

        NormalForm<order> nf(one_turn_map, e0, pc0, mass);
        return nf;
    }

}

#endif // __CUDA_ARCH

#endif // LATTICE_SIMULATOR_H

