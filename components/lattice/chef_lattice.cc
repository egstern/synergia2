#include "chef_lattice.h"
#include "chef_utils.h"
#include "utils/floating_point.h"
#include "components/foundation/math_constants.h"
#include <beamline/beamline_elements.h>
#include <basic_toolkit/PhysicsConstants.h>
#include <physics_toolkit/DriftConverter.h>
#include <beamline/RefRegVisitor.h>

#include <stdexcept>

beamline
Chef_lattice::construct_raw_beamline()
{
    beamline raw_beamlinee;
    for (Lattice_elements::const_iterator latt_it =
            lattice_ptr->get_elements().begin(); latt_it
            != lattice_ptr->get_elements().end(); ++latt_it) {
        Element_adaptor_map * adaptor_map_ptr =
                &(lattice_ptr->get_element_adaptor_map());
        std::string type((*latt_it)->get_type());
        if (!adaptor_map_ptr->has_adaptor(type)) {
            throw(runtime_error("Chef_lattice: " + type + " not handled"));
        } else {
            Chef_elements celms =
                    adaptor_map_ptr->get_adaptor(type)->get_chef_elements(
                            *(*latt_it), brho);
            for (Chef_elements::const_iterator cel_it = celms.begin(); cel_it
                    != celms.end(); ++cel_it) {
                raw_beamlinee.append(*cel_it);
            }
            raw_beamlinee.append(lattice_element_marker);
        }
    }
    return raw_beamlinee;
}

void
Chef_lattice::register_beamline(beamline & the_beamline)
{
    Particle testpart(reference_particle_to_chef_particle(
            lattice_ptr->get_reference_particle()));
    RefRegVisitor registrar(testpart);
    the_beamline.accept(registrar);
}

void
Chef_lattice::polish_raw_beamline(beamline const& raw_beamlinee)
{
    DriftConverter drift_converter;
    beamline_sptr = drift_converter.convert(raw_beamlinee);
    register_beamline(*beamline_sptr);
}

void
Chef_lattice::extract_element_map()
{
    Chef_elements chef_elements;
    Lattice_elements::iterator le_it = lattice_ptr->get_elements().begin();
    for (beamline::const_iterator b_it = beamline_sptr->begin(); b_it
            != beamline_sptr->end(); ++b_it) {
        if ((*b_it)->Name() == lattice_element_marker->Name()) {
            element_map[le_it->get()] = chef_elements;
            chef_elements.clear();
            ++le_it;
        } else {
            chef_elements.push_back(*b_it);
        }
    }
}

Chef_lattice::Chef_lattice(Lattice & lattice) :
    lattice_ptr(&lattice), beamline_sptr(), lattice_element_marker(new marker(
            "synergia_lattice_element_marker"))
{
    sliced_beamline_sptr = BmlPtr(new beamline("sliced"));
    have_sliced_beamline = false;
    if (!lattice_ptr->has_reference_particle()) {
        throw(std::runtime_error(
                "Chef_lattice: requires a reference particle in Lattice"));
    }
    brho = lattice_ptr->get_reference_particle().get_momentum()
            / PH_CNV_brho_to_p;

    polish_raw_beamline(construct_raw_beamline());
    extract_element_map();
}

Chef_elements &
Chef_lattice::get_chef_elements(Lattice_element const& lattice_element)
{
    return element_map[&lattice_element];
}

Chef_elements &
Chef_lattice::get_chef_elements(
        Lattice_element_slice const& lattice_element_slice)
{
    if (!have_sliced_beamline) {
        throw std::runtime_error(
                "get_chef_elements(Lattice_element_slice const&) called before construct_sliced_beamline\n");
    }
    if (element_slice_map.count(&lattice_element_slice) == 0) {
        throw std::runtime_error(
                "get_chef_elements(Lattice_element_slice const&): slice not found\n");
    }
    return element_slice_map[&lattice_element_slice];
}

ElmPtr
slice_chef_element(ElmPtr & elm, double left, double right, double tolerance)
{
    double length = elm->Length();
    ElmPtr retval, left_part, right_part;
    if (left == 0.0) {
        if (floating_point_equal(length, right, tolerance)) {
            retval = elm;
        } else {
            elm->Split(right / length, left_part, right_part);
            retval = left_part;
        }
    } else {
        elm->Split(left / length, left_part, right_part);
        if (floating_point_equal(length, right, tolerance)) {
            retval = right_part;
        } else {
            ElmPtr second_left_part, second_right_part;
            right_part->Split((right - left) / (length - left),
                    second_left_part, second_right_part);
            retval = second_left_part;
        }
    }

    return retval;
}

Chef_elements
Chef_lattice::get_chef_elements_from_slice(Lattice_element_slice const& slice)
{
    Chef_elements all_elements = element_map[&(slice.get_lattice_element())];
    Chef_elements retval;
    if (slice.is_whole()) {
        retval = all_elements;
    } else {
        const double tolerance = 1.0e-8;
        double left = slice.get_left();
        double right = slice.get_right();
        double s = 0.0;
        Chef_elements::iterator c_it = all_elements.begin();
        bool complete = false;
        double element_left, element_right;
        double total_done = 0.0;
        while (!complete) {
            double chef_element_length = (*c_it)->Length();
            if (!floating_point_leq(left, s + chef_element_length, tolerance)) {
                s += chef_element_length;
                ++c_it;
                if (c_it == all_elements.end()) {
                    throw(std::runtime_error(
                            "get_chef_elements_from_slice iterated beyond end of element list"));
                }
            } else {
                element_left = left - s;
                if (floating_point_leq(right, s + chef_element_length,
                        tolerance)) {
                    element_right = right - s;
                    retval.push_back(slice_chef_element(*c_it, element_left,
                            element_right, tolerance));
                    total_done += element_right - element_left;
                } else {
                    element_right = chef_element_length;
                    retval.push_back(slice_chef_element(*c_it, element_left,
                            element_right, tolerance));
                    s += chef_element_length;
                    ++c_it;
                }
                if (floating_point_equal(element_right, chef_element_length,
                        tolerance)) {
                    while ((++c_it != all_elements.end()) && ((*c_it)->Length()
                            == 0.0)) {
                        retval.push_back(*c_it);
                    }
                }
                if (floating_point_equal(total_done, right - left, tolerance)) {
                    complete = true;
                }

            }
        }
    }

    return retval;
}

void
Chef_lattice::construct_sliced_beamline(Lattice_element_slices const& slices)
{
    sliced_beamline_sptr->clear();
    for (Lattice_element_slices::const_iterator it = slices.begin(); it
            != slices.end(); ++it) {
        Chef_elements chef_elements = get_chef_elements_from_slice(*(*it));
        element_slice_map[&(*(*it))] = chef_elements;
        for (Chef_elements::const_iterator c_it = chef_elements.begin(); c_it
                != chef_elements.end(); ++c_it) {
            sliced_beamline_sptr->append(*c_it);
        }
    }
    register_beamline(*sliced_beamline_sptr);
    have_sliced_beamline = true;
}

BmlPtr
Chef_lattice::get_beamline_sptr()
{
    return beamline_sptr;
}

BmlPtr
Chef_lattice::get_sliced_beamline_sptr()
{
    return sliced_beamline_sptr;
}

Chef_lattice::~Chef_lattice()
{

}
