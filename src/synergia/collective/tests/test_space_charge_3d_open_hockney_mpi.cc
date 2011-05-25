#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <algorithm>
#include "synergia/collective/space_charge_3d_open_hockney.h"
#include "synergia/foundation/math_constants.h"
using mconstants::pi;
#include "synergia/foundation/physical_constants.h"
using pconstants::epsilon0;
#include "synergia/bunch/bunch.h"
#include "synergia/bunch/populate.h"
#include "synergia/utils/boost_test_mpi_fixture.h"
#include "synergia/utils/multi_array_print.h"
#include "synergia/utils/floating_point.h"
#include "synergia/utils/multi_array_check_equal.h"
#include "synergia/utils/hdf5_file.h"
#include "gaussian_charge_density.h"
#include "space_charge_bunch_fixtures.h"
BOOST_GLOBAL_FIXTURE(MPI_fixture)

const double tolerance = 1.0e-12;

BOOST_AUTO_TEST_CASE(construct1)
{
    std::vector<int > grid_shape(3);
    grid_shape[0] = 16;
    grid_shape[1] = 16;
    grid_shape[2] = 16;
    Commxx comm(MPI_COMM_WORLD);

    Space_charge_3d_open_hockney space_charge(comm, grid_shape);
}

BOOST_AUTO_TEST_CASE(construct2)
{
    std::vector<int > grid_shape(3);
    grid_shape[0] = 16;
    grid_shape[1] = 16;
    grid_shape[2] = 16;
    Commxx comm(MPI_COMM_WORLD);
    bool longitudinal_kicks(false);
    bool periodic_z(true);
    double z_period(1.1);
    bool grid_entire_period(true);
    double n_sigma(7.0);

    Space_charge_3d_open_hockney space_charge(comm, grid_shape,
            longitudinal_kicks, periodic_z, z_period, grid_entire_period,
            n_sigma);
}

BOOST_AUTO_TEST_CASE(construct_bad_period)
{
    std::vector<int > grid_shape(3);
    grid_shape[0] = 16;
    grid_shape[1] = 16;
    grid_shape[2] = 16;
    Commxx comm(MPI_COMM_WORLD);
    bool longitudinal_kicks(false);
    bool periodic_z(true);
    double z_period(0.0);
    bool grid_entire_period(true);
    double n_sigma(7.0);

    bool caught_error(false);
    try {
        Space_charge_3d_open_hockney space_charge(comm, grid_shape,
                longitudinal_kicks, periodic_z, z_period, grid_entire_period,
                n_sigma);
    }
    catch (std::runtime_error) {
        caught_error = true;
    }
    BOOST_CHECK(caught_error == true);
}

BOOST_AUTO_TEST_CASE(get_n_sigma)
{
    std::vector<int > grid_shape(3);
    grid_shape[0] = 16;
    grid_shape[1] = 16;
    grid_shape[2] = 16;
    Commxx comm(MPI_COMM_WORLD);
    bool longitudinal_kicks(false);
    bool periodic_z(true);
    double z_period(1.1);
    bool grid_entire_period(true);
    double n_sigma(7.0);

    Space_charge_3d_open_hockney space_charge(comm, grid_shape,
            longitudinal_kicks, periodic_z, z_period, grid_entire_period,
            n_sigma);
    BOOST_CHECK_CLOSE(space_charge.get_n_sigma(), n_sigma, tolerance);
}

BOOST_FIXTURE_TEST_CASE(auto_tune_comm, Ellipsoidal_bunch_fixture)
{
    Space_charge_3d_open_hockney space_charge(comm, grid_shape);
    space_charge.auto_tune_comm(false);
}

BOOST_FIXTURE_TEST_CASE(update_domain, Ellipsoidal_bunch_fixture)
{
    Space_charge_3d_open_hockney space_charge(comm, grid_shape);
    space_charge.update_domain(bunch);
}

BOOST_FIXTURE_TEST_CASE(get_domain, Ellipsoidal_bunch_fixture)
{
    Space_charge_3d_open_hockney space_charge(comm, grid_shape);
    space_charge.update_domain(bunch);
    BOOST_CHECK_CLOSE(space_charge.get_domain_sptr()->get_physical_size()[0],
            space_charge.get_n_sigma() * stdz, tolerance);
    BOOST_CHECK_CLOSE(space_charge.get_domain_sptr()->get_physical_size()[1],
            space_charge.get_n_sigma() * stdy, tolerance);
    BOOST_CHECK_CLOSE(space_charge.get_domain_sptr()->get_physical_size()[2],
            space_charge.get_n_sigma() * stdx, tolerance);
    BOOST_CHECK(floating_point_equal(
                    space_charge.get_domain_sptr()->get_physical_offset()[0],
                    0.0, tolerance));
    BOOST_CHECK(floating_point_equal(
                    space_charge.get_domain_sptr()->get_physical_offset()[1],
                    0.0, tolerance));
    BOOST_CHECK(floating_point_equal(
                    space_charge.get_domain_sptr()->get_physical_offset()[2],
                    0.0, tolerance));
    BOOST_CHECK_EQUAL(space_charge.get_domain_sptr()->get_grid_shape()[0],
            grid_shape[2]);
    BOOST_CHECK_EQUAL(space_charge.get_domain_sptr()->get_grid_shape()[1],
            grid_shape[1]);
    BOOST_CHECK_EQUAL(space_charge.get_domain_sptr()->get_grid_shape()[2],
            grid_shape[0]);
}

BOOST_FIXTURE_TEST_CASE(get_doubled_domain, Ellipsoidal_bunch_fixture)
{
    Space_charge_3d_open_hockney space_charge(comm, grid_shape);
    space_charge.update_domain(bunch);
    BOOST_CHECK_EQUAL(space_charge.get_doubled_domain_sptr()->get_grid_shape()[0],
            2*grid_shape[2]);
    BOOST_CHECK_EQUAL(space_charge.get_doubled_domain_sptr()->get_grid_shape()[1],
            2*grid_shape[1]);
    BOOST_CHECK_EQUAL(space_charge.get_doubled_domain_sptr()->get_grid_shape()[2],
            2*grid_shape[0]);
}

BOOST_FIXTURE_TEST_CASE(set_fixed_domain, Ellipsoidal_bunch_fixture)
{
    Space_charge_3d_open_hockney space_charge(comm, grid_shape);
    space_charge.update_domain(bunch);
    Rectangular_grid_domain_sptr domain_sptr = space_charge.get_domain_sptr();
    double scale = 1.23;
    double shift = 0.2345;
    std::vector<double > scaled_size(3), shifted_offset(3);
    for (int i = 0; i < 3; ++i) {
        scaled_size[i] = scale * domain_sptr->get_physical_size()[i];
        shifted_offset[i] = shift + domain_sptr->get_physical_offset()[i];
    }
    Rectangular_grid_domain_sptr new_domain_sptr =
            Rectangular_grid_domain_sptr(new Rectangular_grid_domain(
                    scaled_size, shifted_offset, domain_sptr->get_grid_shape(),
                    domain_sptr->is_periodic()));
    space_charge.set_fixed_domain(new_domain_sptr);
    BOOST_CHECK_CLOSE(space_charge.get_domain_sptr()->get_physical_size()[0],
            scaled_size[0], tolerance);
    BOOST_CHECK_CLOSE(space_charge.get_domain_sptr()->get_physical_size()[1],
            scaled_size[1], tolerance);
    BOOST_CHECK_CLOSE(space_charge.get_domain_sptr()->get_physical_size()[2],
            scaled_size[2], tolerance);
    BOOST_CHECK(floating_point_equal(
                    space_charge.get_domain_sptr()->get_physical_offset()[0],
                    shifted_offset[0], tolerance));
    BOOST_CHECK(floating_point_equal(
                    space_charge.get_domain_sptr()->get_physical_offset()[1],
                    shifted_offset[1], tolerance));
    BOOST_CHECK(floating_point_equal(
                    space_charge.get_domain_sptr()->get_physical_offset()[2],
                    shifted_offset[2], tolerance));
    BOOST_CHECK_EQUAL(space_charge.get_domain_sptr()->get_grid_shape()[0],
            grid_shape[2]);
    BOOST_CHECK_EQUAL(space_charge.get_domain_sptr()->get_grid_shape()[1],
            grid_shape[1]);
    BOOST_CHECK_EQUAL(space_charge.get_domain_sptr()->get_grid_shape()[2],
            grid_shape[0]);

    // make sure that update domain has no effect
    space_charge.update_domain(bunch);
    BOOST_CHECK_CLOSE(space_charge.get_domain_sptr()->get_physical_size()[0],
            scaled_size[0], tolerance);
    BOOST_CHECK_CLOSE(space_charge.get_domain_sptr()->get_physical_size()[1],
            scaled_size[1], tolerance);
    BOOST_CHECK_CLOSE(space_charge.get_domain_sptr()->get_physical_size()[2],
            scaled_size[2], tolerance);
    BOOST_CHECK(floating_point_equal(
                    space_charge.get_domain_sptr()->get_physical_offset()[0],
                    shifted_offset[0], tolerance));
    BOOST_CHECK(floating_point_equal(
                    space_charge.get_domain_sptr()->get_physical_offset()[1],
                    shifted_offset[1], tolerance));
    BOOST_CHECK(floating_point_equal(
                    space_charge.get_domain_sptr()->get_physical_offset()[2],
                    shifted_offset[2], tolerance));
    BOOST_CHECK_EQUAL(space_charge.get_domain_sptr()->get_grid_shape()[0],
            grid_shape[2]);
    BOOST_CHECK_EQUAL(space_charge.get_domain_sptr()->get_grid_shape()[1],
            grid_shape[1]);
    BOOST_CHECK_EQUAL(space_charge.get_domain_sptr()->get_grid_shape()[2],
            grid_shape[0]);
}

BOOST_FIXTURE_TEST_CASE(set_fixed_domain_bad_shape, Ellipsoidal_bunch_fixture)
{
    Space_charge_3d_open_hockney space_charge(comm, grid_shape);
    space_charge.update_domain(bunch);
    Rectangular_grid_domain_sptr domain_sptr = space_charge.get_domain_sptr();
    std::vector<int > bad_shape(domain_sptr->get_grid_shape());
    // not bad yet
    Rectangular_grid_domain_sptr not_bad_domain_sptr =
            Rectangular_grid_domain_sptr(new Rectangular_grid_domain(
                    domain_sptr->get_physical_size(),
                    domain_sptr->get_physical_offset(), bad_shape,
                    domain_sptr->is_periodic()));
    space_charge.set_fixed_domain(not_bad_domain_sptr);

    bool caught_error;
    // bad in 0
    caught_error = false;
    ++bad_shape[0];
    Rectangular_grid_domain_sptr bad0_domain_sptr =
            Rectangular_grid_domain_sptr(new Rectangular_grid_domain(
                    domain_sptr->get_physical_size(),
                    domain_sptr->get_physical_offset(), bad_shape,
                    domain_sptr->is_periodic()));
    try {
        space_charge.set_fixed_domain(bad0_domain_sptr);
    }
    catch (std::runtime_error) {
        caught_error = true;
    }
    BOOST_CHECK(caught_error == true);

    // bad in 1
    caught_error = false;
    --bad_shape[0];
    ++bad_shape[1];
    Rectangular_grid_domain_sptr bad1_domain_sptr =
            Rectangular_grid_domain_sptr(new Rectangular_grid_domain(
                    domain_sptr->get_physical_size(),
                    domain_sptr->get_physical_offset(), bad_shape,
                    domain_sptr->is_periodic()));
    try {
        space_charge.set_fixed_domain(bad1_domain_sptr);
    }
    catch (std::runtime_error) {
        caught_error = true;
    }
    BOOST_CHECK(caught_error == true);

    // bad in 2
    caught_error = false;
    --bad_shape[1];
    ++bad_shape[2];
    Rectangular_grid_domain_sptr bad2_domain_sptr =
            Rectangular_grid_domain_sptr(new Rectangular_grid_domain(
                    domain_sptr->get_physical_size(),
                    domain_sptr->get_physical_offset(), bad_shape,
                    domain_sptr->is_periodic()));
    try {
        space_charge.set_fixed_domain(bad2_domain_sptr);
    }
    catch (std::runtime_error) {
        caught_error = true;
    }
    BOOST_CHECK(caught_error == true);
}

BOOST_FIXTURE_TEST_CASE(get_local_charge_density, Toy_bunch_fixture)
{
    Space_charge_3d_open_hockney space_charge(comm, grid_shape);
    std::vector<int > grid_shape_zyx(3);
    grid_shape_zyx[0] = grid_shape[2];
    grid_shape_zyx[1] = grid_shape[1];
    grid_shape_zyx[2] = grid_shape[0];
    Rectangular_grid_domain_sptr domain_sptr = Rectangular_grid_domain_sptr(
            new Rectangular_grid_domain(physical_size, physical_offset,
                    grid_shape_zyx, false));
    space_charge.set_fixed_domain(domain_sptr);
    std::vector<int > center_zyx(3);
    center_zyx[0] = grid_shape_zyx[0] / 2;
    center_zyx[1] = grid_shape_zyx[1] / 2;
    center_zyx[2] = grid_shape_zyx[2] / 2;

    bunch.set_local_num(1);
    bunch.get_local_particles()[0][0] = 0;
    bunch.get_local_particles()[0][2] = 0;
    bunch.get_local_particles()[0][4] = 0;
    Rectangular_grid_sptr local_charge_density(
            space_charge.get_local_charge_density(bunch));

    for (int i = center_zyx[0] - 1; i < center_zyx[0] + 1; ++i) {
        for (int j = center_zyx[1] - 1; j < center_zyx[1] + 1; ++j) {
            for (int k = center_zyx[2] - 1; k < center_zyx[2] + 1; ++k) {
                expected[i][j][k] = 0.125 * density_norm;
            }
        }
    }
    multi_array_check_equal(local_charge_density->get_grid_points(), expected,
            100 * tolerance);
}

BOOST_AUTO_TEST_CASE(get_global_charge_density2_reduce_scatter)
{
    Commxx comm;

    std::vector<int > grid_shape(3);
    grid_shape[0] = 5;
    grid_shape[1] = 6;
    grid_shape[2] = 16;
    std::vector<int > grid_shape_zyx(3);
    grid_shape_zyx[0] = grid_shape[2];
    grid_shape_zyx[1] = grid_shape[1];
    grid_shape_zyx[2] = grid_shape[0];
    std::vector<double > physical_size(3);
    physical_size[0] = 1.0;
    physical_size[1] = 1.0;
    physical_size[2] = 1.0;
    std::vector<double > physical_offset(3);
    physical_offset[0] = 0.0;
    physical_offset[1] = 0.0;
    physical_offset[2] = 0.0;

    Space_charge_3d_open_hockney space_charge(comm, grid_shape);
    Rectangular_grid_domain_sptr domain_sptr(new Rectangular_grid_domain(
            physical_size, physical_offset, grid_shape_zyx, false));
    space_charge.set_fixed_domain(domain_sptr);
    Rectangular_grid_sptr local_rho(new Rectangular_grid(domain_sptr));
    for (int i = 0; i < grid_shape_zyx[0]; ++i) {
        for (int j = 0; j < grid_shape_zyx[1]; ++j) {
            for (int k = 0; k < grid_shape_zyx[2]; ++k) {
                local_rho->get_grid_points()[i][j][k] = i + 100 * j + 1000 * k;
            }
        }
    }
    Distributed_rectangular_grid_sptr rho2 =
            space_charge.get_global_charge_density2_reduce_scatter(*local_rho); // [C/m^3]
    std::vector<int > nondoubled_shape(
            local_rho->get_domain_sptr()->get_grid_shape());
    std::vector<int > doubled_shape(rho2->get_domain_sptr()->get_grid_shape());
    BOOST_CHECK_EQUAL(2*local_rho->get_domain_sptr()->get_grid_shape()[0],
            doubled_shape[0]);
    BOOST_CHECK_EQUAL(2*local_rho->get_domain_sptr()->get_grid_shape()[1],
            doubled_shape[1]);
    BOOST_CHECK_EQUAL(2*local_rho->get_domain_sptr()->get_grid_shape()[2],
            doubled_shape[2]);

    int lower = rho2->get_lower();
    int upper = rho2->get_upper();
    for (int i = lower; i < upper; ++i) {
        for (int j = 0; j < doubled_shape[1]; ++j) {
            for (int k = 0; k < doubled_shape[2]; ++k) {
                if ((i < nondoubled_shape[0]) && (j < nondoubled_shape[1])
                        && (k < nondoubled_shape[2])) {
                    BOOST_CHECK_CLOSE(rho2->get_grid_points()[i][j][k]*
                            rho2->get_normalization(),
                            local_rho->get_grid_points()[i][j][k]*
                            local_rho->get_normalization()*comm.get_size(),
                            tolerance);
                } else {
                    BOOST_CHECK_EQUAL(rho2->get_grid_points()[i][j][k], 0.0);
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(get_global_charge_density2_allreduce)
{
    Commxx comm;

    std::vector<int > grid_shape(3);
    grid_shape[0] = 5;
    grid_shape[1] = 6;
    grid_shape[2] = 16;
    std::vector<int > grid_shape_zyx(3);
    grid_shape_zyx[0] = grid_shape[2];
    grid_shape_zyx[1] = grid_shape[1];
    grid_shape_zyx[2] = grid_shape[0];
    std::vector<double > physical_size(3);
    physical_size[0] = 1.0;
    physical_size[1] = 1.0;
    physical_size[2] = 1.0;
    std::vector<double > physical_offset(3);
    physical_offset[0] = 0.0;
    physical_offset[1] = 0.0;
    physical_offset[2] = 0.0;

    Space_charge_3d_open_hockney space_charge(comm, grid_shape);
    Rectangular_grid_domain_sptr domain_sptr(new Rectangular_grid_domain(
            physical_size, physical_offset, grid_shape_zyx, false));
    space_charge.set_fixed_domain(domain_sptr);
    Rectangular_grid_sptr local_rho(new Rectangular_grid(domain_sptr));
    // local_rho will be overwritten by the in-place allreduce, so we
    // keep a copy for comparison.
    Rectangular_grid_sptr local_rho_orig(new Rectangular_grid(domain_sptr));
    for (int i = 0; i < grid_shape_zyx[0]; ++i) {
        for (int j = 0; j < grid_shape_zyx[1]; ++j) {
            for (int k = 0; k < grid_shape_zyx[2]; ++k) {
                local_rho->get_grid_points()[i][j][k] = i + 100 * j + 1000 * k;
                local_rho_orig->get_grid_points()[i][j][k] = i + 100 * j + 1000
                        * k;
            }
        }
    }
    Distributed_rectangular_grid_sptr rho2 =
            space_charge.get_global_charge_density2_allreduce(*local_rho); // [C/m^3]
    std::vector<int > nondoubled_shape(
            local_rho->get_domain_sptr()->get_grid_shape());
    std::vector<int > doubled_shape(rho2->get_domain_sptr()->get_grid_shape());
    BOOST_CHECK_EQUAL(2*local_rho->get_domain_sptr()->get_grid_shape()[0],
            doubled_shape[0]);
    BOOST_CHECK_EQUAL(2*local_rho->get_domain_sptr()->get_grid_shape()[1],
            doubled_shape[1]);
    BOOST_CHECK_EQUAL(2*local_rho->get_domain_sptr()->get_grid_shape()[2],
            doubled_shape[2]);

    int lower = rho2->get_lower();
    int upper = rho2->get_upper();
    for (int i = lower; i < upper; ++i) {
        for (int j = 0; j < doubled_shape[1]; ++j) {
            for (int k = 0; k < doubled_shape[2]; ++k) {
                if ((i < nondoubled_shape[0]) && (j < nondoubled_shape[1])
                        && (k < nondoubled_shape[2])) {
                    BOOST_CHECK_CLOSE(rho2->get_grid_points()[i][j][k]*
                            rho2->get_normalization(),
                            local_rho_orig->get_grid_points()[i][j][k]*
                            local_rho_orig->get_normalization()*comm.get_size(),
                            tolerance);
                } else {
                    BOOST_CHECK_EQUAL(rho2->get_grid_points()[i][j][k], 0.0);
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(get_global_charge_density2_simple)
{
    Commxx comm;

    std::vector<int > grid_shape(3);
    grid_shape[0] = 5;
    grid_shape[1] = 6;
    grid_shape[2] = 16;
    std::vector<int > grid_shape_zyx(3);
    grid_shape_zyx[0] = grid_shape[2];
    grid_shape_zyx[1] = grid_shape[1];
    grid_shape_zyx[2] = grid_shape[0];
    std::vector<double > physical_size(3);
    physical_size[0] = 1.0;
    physical_size[1] = 1.0;
    physical_size[2] = 1.0;
    std::vector<double > physical_offset(3);
    physical_offset[0] = 0.0;
    physical_offset[1] = 0.0;
    physical_offset[2] = 0.0;

    Space_charge_3d_open_hockney space_charge(comm, grid_shape);
    Rectangular_grid_domain_sptr domain_sptr(new Rectangular_grid_domain(
            physical_size, physical_offset, grid_shape_zyx, false));
    space_charge.set_fixed_domain(domain_sptr);
    Rectangular_grid_sptr local_rho(new Rectangular_grid(domain_sptr));
    // local_rho will be overwritten by the in-place allreduce, so we
    // keep a copy for comparison.
    Rectangular_grid_sptr local_rho_orig(new Rectangular_grid(domain_sptr));
    for (int i = 0; i < grid_shape_zyx[0]; ++i) {
        for (int j = 0; j < grid_shape_zyx[1]; ++j) {
            for (int k = 0; k < grid_shape_zyx[2]; ++k) {
                local_rho->get_grid_points()[i][j][k] = i + 100 * j + 1000 * k;
                local_rho_orig->get_grid_points()[i][j][k] = i + 100 * j + 1000
                        * k;
            }
        }
    }
    Distributed_rectangular_grid_sptr rho2 =
            space_charge.get_global_charge_density2(*local_rho); // [C/m^3]
    std::vector<int > nondoubled_shape(
            local_rho->get_domain_sptr()->get_grid_shape());
    std::vector<int > doubled_shape(rho2->get_domain_sptr()->get_grid_shape());
    BOOST_CHECK_EQUAL(2*local_rho->get_domain_sptr()->get_grid_shape()[0],
            doubled_shape[0]);
    BOOST_CHECK_EQUAL(2*local_rho->get_domain_sptr()->get_grid_shape()[1],
            doubled_shape[1]);
    BOOST_CHECK_EQUAL(2*local_rho->get_domain_sptr()->get_grid_shape()[2],
            doubled_shape[2]);

    int lower = rho2->get_lower();
    int upper = rho2->get_upper();
    for (int i = lower; i < upper; ++i) {
        for (int j = 0; j < doubled_shape[1]; ++j) {
            for (int k = 0; k < doubled_shape[2]; ++k) {
                if ((i < nondoubled_shape[0]) && (j < nondoubled_shape[1])
                        && (k < nondoubled_shape[2])) {
                    BOOST_CHECK_CLOSE(rho2->get_grid_points()[i][j][k]*
                            rho2->get_normalization(),
                            local_rho_orig->get_grid_points()[i][j][k]*
                            local_rho_orig->get_normalization()*comm.get_size(),
                            tolerance);
                } else {
                    BOOST_CHECK_EQUAL(rho2->get_grid_points()[i][j][k], 0.0);
                }
            }
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_green_fn2_pointlike, Ellipsoidal_bunch_fixture)
{
    Space_charge_3d_open_hockney space_charge(comm, grid_shape);
    space_charge.update_domain(bunch);
    Distributed_rectangular_grid_sptr
            G2(space_charge.get_green_fn2_pointlike());
    MArray3d_ref G2_a(G2->get_grid_points());
    double norm = G2->get_normalization();
    int imirror, jmirror, kmirror;
    double dz, dy, dx;
    const double coeff = 2.8;
    double G000 = coeff / std::min(G2->get_domain_sptr()->get_cell_size()[0],
            std::min(G2->get_domain_sptr()->get_cell_size()[1],
                    G2->get_domain_sptr()->get_cell_size()[2]));

    for (int i = G2->get_lower(); i < G2->get_upper(); ++i) {
        int i_dz;
        if (i < G2->get_domain_sptr()->get_grid_shape()[0] / 2) {
            i_dz = i;
        } else {
            i_dz = G2->get_domain_sptr()->get_grid_shape()[0] - i;
        }
        dz = i_dz * G2->get_domain_sptr()->get_cell_size()[0];
        for (int j = 0; j < G2->get_domain_sptr()->get_grid_shape()[1] / 2; ++j) {
            dy = j * G2->get_domain_sptr()->get_cell_size()[1];
            jmirror = G2->get_domain_sptr()->get_grid_shape()[1] - j;
            if (jmirror == G2->get_domain_sptr()->get_grid_shape()[1]) {
                jmirror = j;
            }
            for (int k = 0; k < G2->get_domain_sptr()->get_grid_shape()[2] / 2; ++k) {
                dx = k * G2->get_domain_sptr()->get_cell_size()[2];
                kmirror = G2->get_domain_sptr()->get_grid_shape()[2] - k;
                if (kmirror == G2->get_domain_sptr()->get_grid_shape()[2]) {
                    kmirror = k;
                }
                double G;
                if ((i == 0) && (j == 0) && (k == 0)) {
                    G = G000;
                } else {
                    G = 1 / std::sqrt(dx * dx + dy * dy + dz * dz);
                }
                BOOST_CHECK_CLOSE(G2_a[i][j][k]*norm, G, tolerance);
                BOOST_CHECK_CLOSE(G2_a[i][jmirror][k]*norm, G, tolerance);
                BOOST_CHECK_CLOSE(G2_a[i][jmirror][kmirror]*norm, G, tolerance);
                BOOST_CHECK_CLOSE(G2_a[i][j][kmirror]*norm, G, tolerance);
            }
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_green_fn2_pointlike_periodic, Ellipsoidal_bunch_fixture)
{
    double z_period = 10.0 * stdz;
    Space_charge_3d_open_hockney space_charge(comm, grid_shape, true, true,
            z_period);
    space_charge.update_domain(bunch);
    Distributed_rectangular_grid_sptr
            G2(space_charge.get_green_fn2_pointlike());
    MArray3d_ref G2_a(G2->get_grid_points());
    double norm = G2->get_normalization();
    int imirror, jmirror, kmirror;
    double dz, dy, dx;
    const double coeff = 2.8;
    double G000 = coeff / std::min(G2->get_domain_sptr()->get_cell_size()[0],
            std::min(G2->get_domain_sptr()->get_cell_size()[1],
                    G2->get_domain_sptr()->get_cell_size()[2]));
    const int num_images = 8;
    for (int i = G2->get_lower(); i < G2->get_upper(); ++i) {
        int i_dz;
        if (i < G2->get_domain_sptr()->get_grid_shape()[0] / 2) {
            i_dz = i;
        } else {
            i_dz = G2->get_domain_sptr()->get_grid_shape()[0] - i;
        }
        dz = i_dz * G2->get_domain_sptr()->get_cell_size()[0];
        for (int j = 0; j < G2->get_domain_sptr()->get_grid_shape()[1] / 2; ++j) {
            dy = j * G2->get_domain_sptr()->get_cell_size()[1];
            jmirror = G2->get_domain_sptr()->get_grid_shape()[1] - j;
            if (jmirror == G2->get_domain_sptr()->get_grid_shape()[1]) {
                jmirror = j;
            }
            for (int k = 0; k < G2->get_domain_sptr()->get_grid_shape()[2] / 2; ++k) {
                dx = k * G2->get_domain_sptr()->get_cell_size()[2];
                kmirror = G2->get_domain_sptr()->get_grid_shape()[2] - k;
                if (kmirror == G2->get_domain_sptr()->get_grid_shape()[2]) {
                    kmirror = k;
                }
                double G;
                if ((i == 0) && (j == 0) && (k == 0)) {
                    G = G000;
                } else {
                    G = 1 / std::sqrt(dx * dx + dy * dy + dz * dz);
                }
                for (int image = -num_images; image <= num_images; ++image) {
                    double dz_image = dz + image * z_period;
                    if (image != 0) {
                        if ((j == 0) && (k == 0) && (std::abs(dz_image)
                                < 1.0e-9)) {
                            G += G000;
                        } else {
                            G += 1.0 / std::sqrt(dx * dx + dy * dy + dz_image
                                    * dz_image);
                        }
                    }
                }
                BOOST_CHECK_CLOSE(G2_a[i][j][k]*norm, G, tolerance);
                BOOST_CHECK_CLOSE(G2_a[i][jmirror][k]*norm, G, tolerance);
                BOOST_CHECK_CLOSE(G2_a[i][jmirror][kmirror]*norm, G, tolerance);
                BOOST_CHECK_CLOSE(G2_a[i][j][kmirror]*norm, G, tolerance);
            }
        }
    }
}

BOOST_FIXTURE_TEST_CASE(get_green_fn2_no_domain, Ellipsoidal_bunch_fixture)
{
    Space_charge_3d_open_hockney space_charge(comm, grid_shape);
    bool caught_error = false;
    try {
        Distributed_rectangular_grid_sptr G2(
                space_charge.get_green_fn2_pointlike());
    }
    catch (std::runtime_error) {
        caught_error = true;
    }
    BOOST_CHECK(caught_error == true);
}

//// solver tests in test_space_charge_3d_open_hockney(2-4).cc
//
//void
//simple_populate(Bunch & bunch, Random_distribution & distribution)
//{
//    MArray2d covariances(boost::extents[6][6]);
//    MArray1d means(boost::extents[6]);
//    for (int i = 0; i < 6; ++i) {
//        means[i] = 0.0;
//        for (int j = i; j < 6; ++j) {
//            covariances[i][j] = 0.0;
//        }
//    }
//    // This bunch shape is contrived to make longitudinal kicks be comparable
//    // to transverse kicks
//    double stdx = 1.1e-3;
//    double stdy = 2.3e-3;
//    double stdz = 3.5e-7;
//    covariances[0][0] = stdx * stdx;
//    covariances[2][2] = stdy * stdy;
//    covariances[4][4] = stdz * stdz;
//    covariances[1][1] = covariances[3][3] = covariances[5][5] = 1.0e-3;
//    populate_6d(distribution, bunch, means, covariances);
//
//}
//
//BOOST_FIXTURE_TEST_CASE(apply_full, Ellipsoidal_bunch_fixture)
//{
//    simple_populate(bunch, distribution);
//    Bunch original_bunch(bunch);
//    Space_charge_3d_open_hockney space_charge(comm, grid_shape, true);
//    const double time_fraction = 1.0;
//    Step dummy_step(time_fraction);
//    const double time_step = 0.3;
//    space_charge.apply(bunch, time_step, dummy_step);
//
//    double total_x_kick2 = 0.0;
//    double total_y_kick2 = 0.0;
//    double total_p_kick2 = 0.0;
//    for (int i = 0; i < bunch.get_local_num(); ++i) {
//        double kick;
//        kick = bunch.get_local_particles()[i][Bunch::xp]
//                - original_bunch.get_local_particles()[i][Bunch::xp];
//        total_x_kick2 += kick * kick;
//        kick += bunch.get_local_particles()[i][Bunch::yp]
//                - original_bunch.get_local_particles()[i][Bunch::yp];
//        total_y_kick2 += kick * kick;
//        kick = bunch.get_local_particles()[i][Bunch::dpop]
//                - original_bunch.get_local_particles()[i][Bunch::dpop];
//        total_p_kick2 += kick * kick;
//    }
//    double avg_x_kick2 = total_x_kick2 / bunch.get_local_num();
//    double avg_y_kick2 = total_y_kick2 / bunch.get_local_num();
//    double avg_p_kick2 = total_p_kick2 / bunch.get_local_num();
//
//    const double rough_tolerance = 50.0;
//    BOOST_CHECK_CLOSE(avg_x_kick2, 2.6e5, rough_tolerance);
//    BOOST_CHECK_CLOSE(avg_y_kick2, 3.4e5, rough_tolerance);
//    BOOST_CHECK_CLOSE(avg_p_kick2, 7.0e7, rough_tolerance);
//}
//
//BOOST_FIXTURE_TEST_CASE(apply_transverse, Ellipsoidal_bunch_fixture)
//{
//    simple_populate(bunch, distribution);
//    Bunch original_bunch(bunch);
//    Space_charge_3d_open_hockney space_charge(comm, grid_shape, false);
//    const double time_fraction = 1.0;
//    Step dummy_step(time_fraction);
//    const double time_step = 0.3;
//    space_charge.apply(bunch, time_step, dummy_step);
//
//    double total_x_kick2 = 0.0;
//    double total_y_kick2 = 0.0;
//    double total_p_kick2 = 0.0;
//    for (int i = 0; i < bunch.get_local_num(); ++i) {
//        double kick;
//        kick = bunch.get_local_particles()[i][Bunch::xp]
//                - original_bunch.get_local_particles()[i][Bunch::xp];
//        total_x_kick2 += kick * kick;
//        kick += bunch.get_local_particles()[i][Bunch::yp]
//                - original_bunch.get_local_particles()[i][Bunch::yp];
//        total_y_kick2 += kick * kick;
//        kick = bunch.get_local_particles()[i][Bunch::dpop]
//                - original_bunch.get_local_particles()[i][Bunch::dpop];
//        total_p_kick2 += kick * kick;
//    }
//    double avg_x_kick2 = total_x_kick2 / bunch.get_local_num();
//    double avg_y_kick2 = total_y_kick2 / bunch.get_local_num();
//    double avg_p_kick2 = total_p_kick2 / bunch.get_local_num();
//
//    const double rough_tolerance = 50.0;
//    BOOST_CHECK_CLOSE(avg_x_kick2, 2.6e5, rough_tolerance);
//    BOOST_CHECK_CLOSE(avg_y_kick2, 3.4e5, rough_tolerance);
//    BOOST_CHECK_CLOSE(avg_p_kick2, 7.1e4, rough_tolerance);
//}
