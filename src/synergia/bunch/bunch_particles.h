#ifndef BUNCH_PARTICLES_H
#define BUNCH_PARTICLES_H


#include "synergia/utils/multi_array_typedefs.h"
#include "synergia/utils/commxx.h"
#include "synergia/utils/logger.h"


typedef Kokkos::View<double*[7], Kokkos::LayoutLeft> Particles;
typedef Kokkos::View<const double*[7], Kokkos::LayoutLeft> ConstParticles;

typedef Particles::HostMirror HostParticles;
typedef ConstParticles::HostMirror ConstHostParticles;

struct BunchParticles
{
    /*
     * Local Particle Array Memory Layout:
     *
     *   P: particle, O: padding, L: lost particle
     *
     *   +=====+
     *   |  P  |
     *   +-----+
     *   |  P  |
     *   +-----+
     *   | ... |
     *   +=====+  <- local_num
     *   |  O  |
     *   +-----+  <- local_num_aligned
     *   |  O  |
     *   +-----+
     *   | ... |
     *   +=====+  <- local_num_padded
     *   |  L  |
     *   +-----+
     *   |  L  |
     *   +-----+
     *   | ... |
     *   +=====+  <- local_num_slots
     *
     *   * number of padding slots  = local_num_padded - local_num
     *   * number of lost particles = local_num_slots - local_num_padded
     *
     *   At bunch construction the size of padding (num_padding) is decided 
     *   such that the local_num_slots is always aligned (depending on the 
     *   vector specification, e.g., SSE or AVX or AVX512). 
     *
     *   local_num_aligned is initialized in the range [local_num, 
     *   local_num_paded], and gets adjusted everytime the local_num 
     *   changes such tht local_num_aligned is always aligned.
     *
     */

    constexpr const static int alignment = 4;
    constexpr static const int particle_index_null = -1;

    int local_num;
    int local_num_aligned;
    int local_num_padded;
    int local_num_slots;

    int total_num;

    Commxx comm;

    Particles parts;
    HostParticles hparts;

    BunchParticles(int total_num, Commxx const& comm);

    void checkin_particles()  { Kokkos::deep_copy(parts, hparts); }
    void checkout_particles() { Kokkos::deep_copy(hparts, parts); }

    void assign_ids(int local_offset);

    void set_local_num(int num);
    void set_total_num(int num);
    void expand_local_num(int num, int added_lost);
    void update_total_num();

    karray2d_row get_particles_in_range(int idx, int num) const;
    karray1d_row get_particle(int idx) const;
    int search_particle(int pid, int last_idx) const;

    void check_pz2_positive();

    void print_particle(size_t idx, Logger & logger) const;
};




#endif
