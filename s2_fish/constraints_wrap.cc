#include "constraints.h"
#undef _POSIX_C_SOURCE
#include <boost/python.hpp>

using namespace boost::python;

BOOST_PYTHON_MODULE(constraints)
{
   // def("apply_longitudinal_periodicity_t", apply_longitudinal_periodicity_t);
    def("apply_longitudinal_periodicity", apply_longitudinal_periodicity);
    def("apply_longitudinal_aperture", apply_longitudinal_aperture);
    def("apply_circular_aperture", apply_circular_aperture);
}
