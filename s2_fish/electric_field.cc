#include "electric_field.h"
#include "mpi.h"
#include "mytimer.h"

void
broadcast_E(Real_scalar_field &E, int i_lower, int i_upper)
{
    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int uppers[size];
    MPI_Allgather(reinterpret_cast<void*>(&i_upper),1,MPI_INT,
            reinterpret_cast<void*>(uppers),1,MPI_INT,MPI_COMM_WORLD);
    Int3 shape(E.get_points().get_shape());
    int last_upper = 0;
    int sender = 0;
    while (last_upper<shape[0]) {
        MPI_Bcast(reinterpret_cast<void*>(E.get_points().get_offset_base_address(last_upper)),
                (uppers[sender]-last_upper)*shape[1]*shape[2],
                MPI_DOUBLE, sender, MPI_COMM_WORLD);
        last_upper = uppers[sender];
        sender++;
    }
}

Real_scalar_field
calculate_E_n(Real_scalar_field &phi, int n)
{
    reset_timer();
    if ((n < 0) || (n > 2)) {
        std::stringstream message("");
        message << "calculate_E_n: invalid argument n=" << n
        << ". Argument be in range 0<=n<=2";
        throw std::invalid_argument(message.str());
    }
    Real_scalar_field E(phi.get_points().get_shape(), phi.get_physical_size(),
                        phi.get_physical_offset());
    Int3 shape(phi.get_points().get_shape());
    Double3 hi(phi.get_cell_size());
    double h(hi[n]), delta;
    Int3 point;
    Int3 offset_plus(0, 0, 0), offset_minus(0, 0, 0);
    offset_plus[n] = 1;
    offset_minus[n] = -1;
    double deriv;
   // calculate i range taking into account guard grids
    int i_lower, i_upper;
    i_lower = phi.get_points().get_dim0_lower();
    if (i_lower > 0) {
            i_lower += 1;
    }
    i_upper = phi.get_points().get_dim0_upper();
    if (i_upper < shape[0] -1) {
            i_upper -= 1;
    }
    for (int i = i_lower; i < i_upper; ++i) {
        point[0] = i;
        for (int j = 0; j < shape[1]; ++j) {
            point[1] = j;
            for (int k = 0; k < shape[2]; ++k) {
                point[2] = k;
                Int3 p0(point), p1(point);
                if (point[n] == 0) {
                    p1.add(offset_plus);
                    delta = h;
                } else if (point[n] == shape[n] - 1) {
                    p0.add(offset_minus);
                    delta = h;
                } else {
                    p0.add(offset_minus);
                    p1.add(offset_plus);
                    delta = 2.0 * h;
                }
                deriv = (phi.get_points().get(p1) - phi.get_points().get(p0)) / delta;
                E.get_points().set(point, deriv);
            }
        }
    }
    timer("E calc");
    broadcast_E(E,i_lower,i_upper);
    timer("E broadcast");
    return E;
}

void
apply_E_n_kick(Real_scalar_field &E, int n_axis, double tau,
               Macro_bunch_store &mbs)
{
    if ((n_axis < 0) || (n_axis > 2)) {
        std::stringstream message("");
        message << "apply_E_n_kick: invalid argument n_axis=" << n_axis
        << ". Argument be in range 0<=n_axis<=2";
        throw std::invalid_argument(message.str());
    }
    // jfa: I am taking this calculation of "factor" more-or-less
    // directly from Impact.  I should really redo it in terms that make
    // sense to me
    double gamma = -1 * mbs.ref_particle(5);
    double beta = sqrt(gamma * gamma - 1.0) / gamma;
    const  double c = 2.99792458e8;
    const  double pi = 4.0 * atan(1.0);

    // GACK! THIS IS HARDCODED FOR PROTONS! FIXME FIXME FIXME FIXME
    // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
    // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
    double mass = 0.93827231e9; // should I come from macro_bunch_store?
    double eps0 = 1.0 / (4 * pi * c * c * 1.0e-7); // using c^2 = 1/(eps0 * mu0)
    double Brho = gamma * beta * mass / c;
    double perveance0 = mbs.total_current / (2 * pi * eps0 * Brho * gamma * gamma*\
                        beta * c * beta * c);
    double xk = mbs.units(0);

    double factor = pi * perveance0 * gamma * beta * beta / xk * 4.0 * pi;
    factor *= 1.0 / mbs.total_num;
    if (n_axis == 2) {
        factor *= beta * gamma * gamma;
    } else {
        // (We think) this is for the Lorentz transformation of the transverse
        // E field.
        factor *= gamma;
    }
    int index = 2 * n_axis + 1; // for axis n_axis = (0,1,2) corresponding to x,y,z,
    // in particle store indexing, px,py,pz = (1,3,5)
    double kick;
    for (int n = 0; n < mbs.local_num; ++n) {
            kick = tau * factor * E.get_val(Double3(mbs.local_particles(0, n),
                                                    mbs.local_particles(2, n),
                                                    mbs.local_particles(4, n)));
        mbs.local_particles(index, n) -= kick;
    }
}

void
full_kick(Real_scalar_field &phi, double tau, Macro_bunch_store &mbs)
{
    for (int axis = 0; axis < 3; ++axis) {
        Real_scalar_field E = calculate_E_n(phi, axis);
        apply_E_n_kick(E, axis, tau, mbs);
    }
}
