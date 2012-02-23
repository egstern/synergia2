#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "synergia/lattice/element_adaptor.h"
#include "synergia/utils/serialization.h"
#include "synergia/utils/serialization_files.h"
#include "synergia/utils/boost_test_mpi_fixture.h"
BOOST_GLOBAL_FIXTURE(MPI_fixture)

BOOST_AUTO_TEST_CASE(construct)
{
    Element_adaptor element_adaptor;
}

BOOST_AUTO_TEST_CASE(serialize_xml)
{
    Element_adaptor element_adaptor;
    xml_save<Element_adaptor > (element_adaptor, "element_adaptor.xml");

    Element_adaptor loaded;
    xml_load<Element_adaptor > (loaded, "element_adaptor.xml");
}
