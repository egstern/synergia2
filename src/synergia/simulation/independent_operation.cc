
#include "independent_operation.h"

#include "synergia/libFF/ff_drift.h"
#include "synergia/libFF/ff_sbend.h"
#include "synergia/libFF/ff_quadrupole.h"
#include "synergia/libFF/ff_rfcavity.h"
#include "synergia/libFF/ff_multipole.h"
#include "synergia/libFF/ff_sextupole.h"
#include "synergia/libFF/ff_octupole.h"

#include "synergia/libFF/ff_kicker.h"

namespace
{
    std::unique_ptr<FF_element> make_ff_element(element_type t)
    {
        switch(t)
        {
        case element_type::drift:      return std::make_unique<FF_drift>();
        case element_type::sbend:      return std::make_unique<FF_sbend>();
        case element_type::quadrupole: return std::make_unique<FF_quadrupole>();
        case element_type::rfcavity:   return std::make_unique<FF_rfcavity>();
        case element_type::multipole:  return std::make_unique<FF_multipole>();

        //case element_type::rbend:      return std::make_unique<FF_drift>();
        case element_type::hkicker:    return std::make_unique<FF_kicker>();
        case element_type::vkicker:    return std::make_unique<FF_kicker>();
        case element_type::kicker:     return std::make_unique<FF_kicker>();

        case element_type::monitor:    return std::make_unique<FF_drift>();
        case element_type::sextupole:  return std::make_unique<FF_sextupole>();
        case element_type::octupole:   return std::make_unique<FF_octupole>();
        case element_type::marker:     return std::make_unique<FF_drift>();
        case element_type::instrument: return std::make_unique<FF_drift>();
        case element_type::rcollimator:return std::make_unique<FF_drift>();

        default:
            throw std::runtime_error(
                    "unhandled element type " + std::to_string((int)t));
        }
    }
}


LibFF_operation::LibFF_operation(
        std::vector<Lattice_element_slice> const& slices) 
    : Independent_operation("LibFF")
    , libff_element_slices()
{
    for (auto const & slice : slices)
    {
        auto type = slice.get_lattice_element().get_type();
        libff_element_slices.emplace_back(
                std::make_pair(make_ff_element(type), slice) );
    }
}

void
LibFF_operation::apply_impl(Bunch & bunch, Logger & logger) const
{
    for (auto const & es : libff_element_slices)
        es.first->apply(es.second, bunch);
}




#if 0
Independent_operation::Independent_operation(std::string const& type) :
    type(type)
{
}

Independent_operation::Independent_operation()
{
}

std::string const&
Independent_operation::get_type() const
{
    return type;
}

Fast_mapping_operation::Fast_mapping_operation(Fast_mapping const& mapping) :
    Independent_operation(fast_mapping_type_name), mapping(mapping)
{
}

Fast_mapping_operation::Fast_mapping_operation()
{
}

void
Fast_mapping_operation::apply(Bunch & bunch, int verbosity, Logger & logger)
{
    double t = simple_timer_current();
    bunch.convert_to_state(Bunch::fixed_z_lab);
    t = simple_timer_show(t, "fast_mapping_operation_apply-convert_to_state");
    if (verbosity > 4) {
        logger << "Fast_mapping_operation: length = " << mapping.get_length()
                << ", order = " << mapping.get_order() << std::endl;
    }
    mapping.apply(bunch);
    t = simple_timer_show(t, "fast_mapping_operation_apply-mapping_apply");
}

Fast_mapping const&
Fast_mapping_operation::get_fast_mapping() const
{
    return mapping;
}


Chef_propagate_operation::Chef_propagate_operation(
        Chef_lattice_section const& chef_lattice_section) 
    : Independent_operation(chef_propagate_type_name)
    , chef_propagator( Chef_lattice_section_sptr( 
                new Chef_lattice_section(chef_lattice_section)))
{
}

Chef_propagate_operation::Chef_propagate_operation()
{
}

void
Chef_propagate_operation::apply(Bunch & bunch, int verbosity, Logger & logger)
{
    double t = simple_timer_current();
    bunch.convert_to_state(Bunch::fixed_z_lab);
    t = simple_timer_show(t, "chef_propagate_operation_apply-convert_to_state");
    chef_propagator.apply(bunch, verbosity, logger);
    t = simple_timer_show(t, "chef_propagate_operation_apply-chef_propagator_apply");
}

BOOST_CLASS_EXPORT_IMPLEMENT(Chef_propagate_operation);

LibFF_operation::LibFF_operation(
        Lattice_element_slices const& lattice_element_slices) :
            Independent_operation(libFF_type_name),
            lattice_element_slices(lattice_element_slices)
{
}

LibFF_operation::LibFF_operation()
{
}

void
LibFF_operation::apply(Bunch & bunch, int verbosity, Logger & logger)
{
    double t = simple_timer_current();
    bunch.convert_to_state(Bunch::fixed_z_lab);
    t = simple_timer_show(t, "LibFF_operation_apply-convert_to_state");
    for(Lattice_element_slices::iterator it = lattice_element_slices.begin();
        it != lattice_element_slices.end(); ++it) {
        std::string const& type((*it)->get_lattice_element().get_type());
        the_big_giant_global_ff_element_map.get_element_type(type)->apply(**it, bunch);
    }
    t = simple_timer_show(t, "LibFF_operation_apply-element_apply");
}

#endif
