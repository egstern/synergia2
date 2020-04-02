#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "synergia/lattice/chef_lattice_section.h"
#include "synergia/utils/serialization.h"
#include "synergia/utils/serialization_files.h"
#include "synergia/utils/boost_test_mpi_fixture.h"
BOOST_GLOBAL_FIXTURE(MPI_fixture);

#include "chef_lattice_sptr_fixture.h"

const int begin1 = 0;
const int end1 = 2;
const int end2 = 3;

BOOST_FIXTURE_TEST_CASE(construct1, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section(chef_lattice_sptr);
}

BOOST_FIXTURE_TEST_CASE(construct2, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section(chef_lattice_sptr, begin1, end1);
}

BOOST_FIXTURE_TEST_CASE(extend1, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section1(chef_lattice_sptr);
    Chef_lattice_section chef_lattice_section2(chef_lattice_sptr, begin1, end1);
    chef_lattice_section1.extend(chef_lattice_section2);
    BOOST_CHECK_EQUAL(chef_lattice_section1.get_begin_index(), begin1);
    BOOST_CHECK_EQUAL(chef_lattice_section1.get_end_index(), end1);
}

BOOST_FIXTURE_TEST_CASE(extend2, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section2(chef_lattice_sptr, begin1, end1);
    Chef_lattice_section chef_lattice_section3(chef_lattice_sptr, end1, end2);
    chef_lattice_section2.extend(chef_lattice_section3);
    BOOST_CHECK_EQUAL(chef_lattice_section2.get_begin_index(), begin1);
    BOOST_CHECK_EQUAL(chef_lattice_section2.get_end_index(), end2);
}

BOOST_FIXTURE_TEST_CASE(extend3, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section(chef_lattice_sptr, begin1, end1);
    chef_lattice_section.extend(end1, end2);
    BOOST_CHECK_EQUAL(chef_lattice_section.get_begin_index(), begin1);
    BOOST_CHECK_EQUAL(chef_lattice_section.get_end_index(), end2);
}

BOOST_FIXTURE_TEST_CASE(extend_invalid1, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section(chef_lattice_sptr, 0, 1);
    Chef_lattice_section chef_lattice_section4(chef_lattice_sptr, 3, 4);
    bool caught = false;
    try {
        chef_lattice_section.extend(chef_lattice_section4);
    }
    catch (std::runtime_error&) {
        caught = true;
    }
    BOOST_CHECK(caught);
}

BOOST_FIXTURE_TEST_CASE(extend_invalid2, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section(chef_lattice_sptr, 0, 1);
    bool caught = false;
    try {
        chef_lattice_section.extend(3, 4);
    }
    catch (std::runtime_error&) {
        caught = true;
    }
    BOOST_CHECK(caught);
}

BOOST_FIXTURE_TEST_CASE(get_begin_index, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section(chef_lattice_sptr, begin1, end1);

    BOOST_CHECK_EQUAL(chef_lattice_section.get_begin_index(), begin1);
}

BOOST_FIXTURE_TEST_CASE(get_end_index, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section(chef_lattice_sptr, begin1, end1);

    BOOST_CHECK_EQUAL(chef_lattice_section.get_end_index(), end1);
}

BOOST_FIXTURE_TEST_CASE(begin_iterator, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section(chef_lattice_sptr, begin1, end1);

    beamline::iterator it = chef_lattice_section.begin();
    BOOST_CHECK(it != chef_lattice_section.end());
}

BOOST_FIXTURE_TEST_CASE(end_iterator, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section(chef_lattice_sptr, begin1, end1);

    beamline::iterator it = chef_lattice_section.end();
    BOOST_CHECK(it != chef_lattice_section.begin());
}

BOOST_FIXTURE_TEST_CASE(begin_const_iterator, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section(chef_lattice_sptr, begin1, end1);

    beamline::const_iterator it = chef_lattice_section.begin();
    BOOST_CHECK(it != chef_lattice_section.end());
}

BOOST_FIXTURE_TEST_CASE(end_const_iterator, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section(chef_lattice_sptr, begin1, end1);

    beamline::const_iterator it = chef_lattice_section.end();
    BOOST_CHECK(it != chef_lattice_section.begin());
}

BOOST_FIXTURE_TEST_CASE(empty, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section(chef_lattice_sptr, begin1, end1);
    Chef_lattice_section chef_lattice_section_empty(chef_lattice_sptr);

    BOOST_CHECK(!chef_lattice_section.empty());
    BOOST_CHECK(chef_lattice_section_empty.empty());
}

BOOST_FIXTURE_TEST_CASE(clear, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section(chef_lattice_sptr, begin1, end1);

    BOOST_CHECK(!chef_lattice_section.empty());

    chef_lattice_section.clear();

    BOOST_CHECK(chef_lattice_section.empty());
}

BOOST_FIXTURE_TEST_CASE(serialize_xml, Chef_lattice_sptr_fixture)
{
    Chef_lattice_section chef_lattice_section(chef_lattice_sptr, begin1, end1);
    xml_save<Chef_lattice_section > (chef_lattice_section,
            "chef_lattice_section.xml");

    Chef_lattice_section loaded;
    xml_load<Chef_lattice_section > (loaded, "chef_lattice_section.xml");
}
