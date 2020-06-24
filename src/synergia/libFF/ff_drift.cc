#include "ff_drift.h"

#include "synergia/utils/gsvector.h"
#include "synergia/libFF/ff_algorithm.h"
#include "synergia/utils/logger.h"
#include "synergia/utils/simple_timer.h"

namespace
{
    struct PropDrift
    {
        Particles p;
        ConstParticleMasks masks;

        double len, ref_p, mass, ref_t;

        KOKKOS_INLINE_FUNCTION
        void operator()(const int i) const
        {
            if (masks(i))
                FF_algorithm::drift_unit(
                        p(i, 0), p(i, 1), p(i, 2),
                        p(i, 3), p(i, 4), p(i, 5),
                        len, ref_p, mass, ref_t);
        }
    };

    struct PropDriftSimd
    {
        Particles p;
        ConstParticleMasks masks;

        double len, ref_p, mass, ref_t;

        KOKKOS_INLINE_FUNCTION
        void operator()(const int idx) const
        {
            int i = idx * GSVector::size();

            int m = 0;
            for(int x=i; x<i+GSVector::size(); ++x) m |= masks(x);

            if (m)
            {
                GSVector p0(&p(i, 0));
                GSVector p1(&p(i, 1));
                GSVector p2(&p(i, 2));
                GSVector p3(&p(i, 3));
                GSVector p4(&p(i, 4));
                GSVector p5(&p(i, 5));

                FF_algorithm::drift_unit(
                        p0, p1, p2, p3, p4, p5,
                        len, ref_p, mass, ref_t);

                p0.store(&p(i, 0));
                p2.store(&p(i, 2));
                p4.store(&p(i, 4));
            }
        }
    };


    double get_reference_cdt(double length, Reference_particle & ref)
    {
        double x(ref.get_state()[Bunch::x]);
        double xp(ref.get_state()[Bunch::xp]);
        double y(ref.get_state()[Bunch::y]);
        double yp(ref.get_state()[Bunch::yp]);
        double cdt(0.0);
        double dpop(ref.get_state()[Bunch::dpop]);
        double ref_p = ref.get_momentum();
        double m = ref.get_mass();

        FF_algorithm::drift_unit(x, xp, y, yp, cdt, dpop, length, ref_p, m, 0.0);

        // propagate and update the bunch design reference particle state
        ref.set_state(x, xp, y, yp, cdt, dpop);

        return cdt;
    }

    void apply_impl(BunchParticles & bp, 
            double length, double ref_p, double mass, double ref_cdt)
    {
        if (bp.num_valid())
        {
#if 0
            PropDrift drift{bp.parts, bp.masks, length, ref_p, mass, ref_cdt};
            Kokkos::parallel_for(bp.size(), drift);
#endif
#if 1
            PropDriftSimd drift{bp.parts, bp.masks, length, ref_p, mass, ref_cdt};
            Kokkos::parallel_for(bp.size()/GSVector::size(), drift);
#endif
        }
    }


}

void FF_drift::apply(Lattice_element_slice const& slice, JetParticle& jet_particle)
{
    throw std::runtime_error("Propagate JetParticle through a drift element is yet to be implemented");

#if 0
    double length = slice.get_right() - slice.get_left();

    typedef PropagatorTraits<JetParticle>::State_t State_t;
    typedef PropagatorTraits<JetParticle>::Component_t Component_t;

    State_t& state = jet_particle.State();

    Component_t & x(state[Chef::x]);
    Component_t & xp(state[Chef::xp]);
    Component_t & y(state[Chef::y]);
    Component_t & yp(state[Chef::yp]);
    Component_t & cdt(state[Chef::cdt]);
    Component_t & dpop(state[Chef::dpop]);

    double reference_momentum = jet_particle.ReferenceMomentum();
    double m = jet_particle.Mass();

    Particle chef_particle(jet_particle);
    Reference_particle reference_particle(
                chef_particle_to_reference_particle(chef_particle));
    double reference_cdt = get_reference_cdt(length, reference_particle);

    FF_algorithm::drift_unit(x, xp, y, yp, cdt, dpop, length, reference_momentum, m,
               reference_cdt);
#endif
}

void FF_drift::apply(Lattice_element_slice const& slice, Bunch& bunch)
{
    scoped_simple_timer timer("libFF_drift");

    const double  length = slice.get_right() - slice.get_left();
    const double    mass = bunch.get_mass();

    // zero-length drift does nothing
    if (close_to_zero(length)) return;

    Reference_particle       & ref_l = bunch.get_design_reference_particle();
    Reference_particle const & ref_b = bunch.get_reference_particle();

    const double ref_p   = ref_b.get_momentum() * (1.0 + ref_b.get_state()[Bunch::dpop]);
    const double ref_cdt = get_reference_cdt(length, ref_l);

    // regular particles
    apply_impl(bunch.get_bunch_particles(ParticleGroup::regular),
            length, ref_p, mass, ref_cdt);

    // spectator particles
    apply_impl(bunch.get_bunch_particles(ParticleGroup::spectator),
            length, ref_p, mass, ref_cdt);

    // trajectory
    bunch.get_reference_particle().increment_trajectory(length);

    Kokkos::fence();
}

