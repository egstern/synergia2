#define BOOST_TEST_MAIN
#include <vector>
#include <cstdlib>
#include <boost/test/unit_test.hpp>
#include "synergia/foundation/diagnostics_write_helper.h"
#include "synergia/utils/hdf5_serial_writer.h"
#include "synergia/utils/serialization_files.h"

#include "synergia/utils/boost_test_mpi_fixture.h"
BOOST_GLOBAL_FIXTURE(MPI_fixture)

BOOST_AUTO_TEST_CASE(construct)
{
    std::string filename("test_write_helper.h5");
    bool serial = true;
    int write_skip = 1;
    Commxx commxx;
    Diagnostics_write_helper d(filename, serial, write_skip, commxx);
}

BOOST_AUTO_TEST_CASE(serialize_)
{
    std::string filename("test_write_helper2.h5");
    bool serial = true;
    int write_skip = 1;
    Commxx commxx;
    {
        Diagnostics_write_helper d(filename, serial, write_skip, commxx);
        xml_save(d, "d.xml");
    }

    Diagnostics_write_helper d_new;
    xml_load(d_new, "d.xml");
}
