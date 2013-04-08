#include "lattice_simulator.h"
#include "synergia/foundation/physical_constants.h"
#include "synergia/utils/logger.h"
#include "synergia/utils/containers_to_string.h"
#include "synergia/utils/digits.h"

#if __GNUC__ > 4 && __GNUC_MINOR__ > 5
#pragma GCC diagnostic push
#endif
#pragma GCC diagnostic ignored "-Wsequence-point"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#include <beamline/beamline_elements.h>
#include <physics_toolkit/Sage.h>
#include <basic_toolkit/PhysicsConstants.h>
#include <beamline/RefRegVisitor.h>
#if __GNUC__ > 4 && __GNUC_MINOR__ > 5
#pragma GCC diagnostic pop
#endif

#include <stdexcept>
#include <cstring>
#include <cmath>
#include <iostream>
#include <fstream>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multiroots.h>

Lattice_functions::Lattice_functions() :
                alpha_x(0.0),
                alpha_y(0.0),
                beta_x(0.0),
                beta_y(0.0),
                psi_x(0.0),
                psi_y(0.0),
                D_x(0.0),
                D_y(0.0),
                Dprime_x(0.0),
                Dprime_y(0.0),
                arc_length(0.0)
{
}

Lattice_functions::Lattice_functions(LattFuncSage::lattFunc const& latt_func) :
                alpha_x(latt_func.alpha.hor),
                alpha_y(latt_func.alpha.ver),
                beta_x(latt_func.beta.hor),
                beta_y(latt_func.beta.ver),
                psi_x(latt_func.psi.hor),
                psi_y(latt_func.psi.ver),
                D_x(latt_func.dispersion.hor),
                D_y(latt_func.dispersion.ver),
                Dprime_x(latt_func.dPrime.hor),
                Dprime_y(latt_func.dPrime.ver),
                arc_length(latt_func.arcLength)
{
}

template<class Archive>
    void
    Lattice_functions::serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(alpha_x);
        ar & BOOST_SERIALIZATION_NVP(alpha_y);
        ar & BOOST_SERIALIZATION_NVP(beta_x);
        ar & BOOST_SERIALIZATION_NVP(beta_y);
        ar & BOOST_SERIALIZATION_NVP(psi_x);
        ar & BOOST_SERIALIZATION_NVP(psi_y);
        ar & BOOST_SERIALIZATION_NVP(D_x);
        ar & BOOST_SERIALIZATION_NVP(D_y);
        ar & BOOST_SERIALIZATION_NVP(Dprime_x);
        ar & BOOST_SERIALIZATION_NVP(Dprime_y);
        ar & BOOST_SERIALIZATION_NVP(arc_length);
    }

template
void
Lattice_functions::serialize<boost::archive::binary_oarchive >(
        boost::archive::binary_oarchive & ar, const unsigned int version);

template
void
Lattice_functions::serialize<boost::archive::xml_oarchive >(
        boost::archive::xml_oarchive & ar, const unsigned int version);

template
void
Lattice_functions::serialize<boost::archive::binary_iarchive >(
        boost::archive::binary_iarchive & ar, const unsigned int version);

template
void
Lattice_functions::serialize<boost::archive::xml_iarchive >(
        boost::archive::xml_iarchive & ar, const unsigned int version);

ET_lattice_functions::ET_lattice_functions() :
                beta_x(0.0),
                beta_y(0.0),
                alpha_x(0.0),
                alpha_y(0.0),
                phi(0.0),
                arc_length(0.0)
{
}

ET_lattice_functions::ET_lattice_functions(EdwardsTengSage::Info const& ET_Info) :
                beta_x(ET_Info.beta.hor),
                beta_y(ET_Info.beta.ver),
                alpha_x(ET_Info.alpha.hor),
                alpha_y(ET_Info.alpha.ver),
                phi(ET_Info.phi),
                arc_length(ET_Info.arcLength)
{
}

template<class Archive>
    void
    ET_lattice_functions::serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(alpha_x);
        ar & BOOST_SERIALIZATION_NVP(alpha_y);
        ar & BOOST_SERIALIZATION_NVP(beta_x);
        ar & BOOST_SERIALIZATION_NVP(beta_y);
        ar & BOOST_SERIALIZATION_NVP(phi);
        ar & BOOST_SERIALIZATION_NVP(arc_length);
    }

template
void
ET_lattice_functions::serialize<boost::archive::binary_oarchive >(
        boost::archive::binary_oarchive & ar, const unsigned int version);

template
void
ET_lattice_functions::serialize<boost::archive::xml_oarchive >(
        boost::archive::xml_oarchive & ar, const unsigned int version);

template
void
ET_lattice_functions::serialize<boost::archive::binary_iarchive >(
        boost::archive::binary_iarchive & ar, const unsigned int version);

template
void
ET_lattice_functions::serialize<boost::archive::xml_iarchive >(
        boost::archive::xml_iarchive & ar, const unsigned int version);

LB_lattice_functions::LB_lattice_functions() :
                beta_1x(0.0),
                beta_1y(0.0),
                beta_2x(0.0),
                beta_2y(0.0),
                alpha_1x(0.0),
                alpha_1y(0.0),
                alpha_2x(0.0),
                alpha_2y(0.0),
                u1(0.0),
                u2(0.0),
                u3(0.0),
                u4(0.0),
                nu_1(0.0),
                nu_2(0.0),
                arc_length(0.0)
{
}

LB_lattice_functions::LB_lattice_functions(LBSage::Info const& LB_Info) :
                beta_1x(LB_Info.beta_1x),
                beta_1y(LB_Info.beta_1y),
                beta_2x(LB_Info.beta_2x),
                beta_2y(LB_Info.beta_2y),
                alpha_1x(LB_Info.alpha_1x),
                alpha_1y(LB_Info.alpha_1y),
                alpha_2x(LB_Info.alpha_2x),
                alpha_2y(LB_Info.alpha_2y),
                u1(LB_Info.u1),
                u2(LB_Info.u2),
                u3(LB_Info.u3),
                u4(LB_Info.u4),
                nu_1(LB_Info.nu_1),
                nu_2(LB_Info.nu_2),
                arc_length(LB_Info.arcLength)
{
}

template<class Archive>
    void
    LB_lattice_functions::serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(beta_1x);
        ar & BOOST_SERIALIZATION_NVP(beta_1y);
        ar & BOOST_SERIALIZATION_NVP(beta_2x);
        ar & BOOST_SERIALIZATION_NVP(beta_2y);
        ar & BOOST_SERIALIZATION_NVP(alpha_1x);
        ar & BOOST_SERIALIZATION_NVP(alpha_1y);
        ar & BOOST_SERIALIZATION_NVP(alpha_2x);
        ar & BOOST_SERIALIZATION_NVP(alpha_2y);
        ar & BOOST_SERIALIZATION_NVP(u1);
        ar & BOOST_SERIALIZATION_NVP(u2);
        ar & BOOST_SERIALIZATION_NVP(u3);
        ar & BOOST_SERIALIZATION_NVP(u4);
        ar & BOOST_SERIALIZATION_NVP(nu_1);
        ar & BOOST_SERIALIZATION_NVP(nu_2);
        ar & BOOST_SERIALIZATION_NVP(arc_length);
    }

template
void
LB_lattice_functions::serialize<boost::archive::binary_oarchive >(
        boost::archive::binary_oarchive & ar, const unsigned int version);

template
void
LB_lattice_functions::serialize<boost::archive::xml_oarchive >(
        boost::archive::xml_oarchive & ar, const unsigned int version);

template
void
LB_lattice_functions::serialize<boost::archive::binary_iarchive >(
        boost::archive::binary_iarchive & ar, const unsigned int version);

template
void
LB_lattice_functions::serialize<boost::archive::xml_iarchive >(
        boost::archive::xml_iarchive & ar, const unsigned int version);

Dispersion_functions::Dispersion_functions() :
                dispersion_x(0.0),
                dispersion_y(0.0),
                dPrime_x(0.0),
                dPrime_y(0.0),
                closedOrbit_x(0.0),
                closedOrbit_y(0.0),
                closedOrbitP_x(0.0),
                closedOrbitP_y(0.0),
                arc_length(0.0)
{
}

Dispersion_functions::Dispersion_functions(
        DispersionSage::Info const& Disp_Info) :
                dispersion_x(Disp_Info.dispersion.hor),
                dispersion_y(Disp_Info.dispersion.ver),
                dPrime_x(Disp_Info.dPrime.hor),
                dPrime_y(Disp_Info.dPrime.ver),
                closedOrbit_x(Disp_Info.closedOrbit.hor),
                closedOrbit_y(Disp_Info.closedOrbit.ver),
                closedOrbitP_x(Disp_Info.closedOrbitP.hor),
                closedOrbitP_y(Disp_Info.closedOrbitP.ver),
                arc_length(Disp_Info.arcLength)
{
}

template<class Archive>
    void
    Dispersion_functions::serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(dispersion_x);
        ar & BOOST_SERIALIZATION_NVP(dispersion_y);
        ar & BOOST_SERIALIZATION_NVP(dPrime_x);
        ar & BOOST_SERIALIZATION_NVP(dPrime_y);
        ar & BOOST_SERIALIZATION_NVP(closedOrbit_x);
        ar & BOOST_SERIALIZATION_NVP(closedOrbit_y);
        ar & BOOST_SERIALIZATION_NVP(closedOrbitP_x);
        ar & BOOST_SERIALIZATION_NVP(closedOrbitP_y);
        ar & BOOST_SERIALIZATION_NVP(arc_length);
    }

template
void
Dispersion_functions::serialize<boost::archive::binary_oarchive >(
        boost::archive::binary_oarchive & ar, const unsigned int version);

template
void
Dispersion_functions::serialize<boost::archive::xml_oarchive >(
        boost::archive::xml_oarchive & ar, const unsigned int version);

template
void
Dispersion_functions::serialize<boost::archive::binary_iarchive >(
        boost::archive::binary_iarchive & ar, const unsigned int version);

template
void
Dispersion_functions::serialize<boost::archive::xml_iarchive >(
        boost::archive::xml_iarchive & ar, const unsigned int version);

void
Lattice_simulator::construct_extractor_map()
{
    Operation_extractor_sptr chef_mixed_operation_extractor(
            new Chef_mixed_operation_extractor(chef_lattice_sptr, map_order));

    extractor_map_sptr->set_extractor(default_operation_extractor_name,
            chef_mixed_operation_extractor);
    extractor_map_sptr->set_extractor(chef_mixed_operation_extractor_name,
            chef_mixed_operation_extractor);
    extractor_map_sptr->set_extractor(chef_propagate_operation_extractor_name,
            Operation_extractor_sptr(
                    new Chef_propagate_operation_extractor(chef_lattice_sptr,
                            map_order)));
    extractor_map_sptr->set_extractor(chef_map_operation_extractor_name,
            Operation_extractor_sptr(
                    new Chef_map_operation_extractor(chef_lattice_sptr,
                            map_order)));
}

void
Lattice_simulator::calculate_beamline_context()
{
    ensure_jet_environment(map_order);
    BmlPtr beamline_sptr(chef_lattice_sptr->get_beamline_sptr());
    beamline_context_sptr = BmlContextPtr(
            new BeamlineContext(
                    reference_particle_to_chef_particle(
                            lattice_sptr->get_reference_particle()),
                    beamline_sptr));
    if (!Sage::isRing(beamline_sptr)) {
        beamline_context_sptr->handleAsRing();
    }
}

void
Lattice_simulator::calculate_sliced_beamline_context()
{
    if (!have_slices) {
        throw std::runtime_error(
                "Lattice_simulator::calculate_sliced_beamline_context called before set_slices");
    }
    ensure_jet_environment(map_order);
    BmlPtr beamline_sptr(chef_lattice_sptr->get_sliced_beamline_sptr());
    sliced_beamline_context_sptr = BmlContextPtr(
            new BeamlineContext(
                    reference_particle_to_chef_particle(
                            lattice_sptr->get_reference_particle()),
                    beamline_sptr));
    if (!Sage::isRing(beamline_sptr)) {
        sliced_beamline_context_sptr->handleAsRing();
    }
}

BmlContextPtr
Lattice_simulator::get_beamline_context()
{
    if (!have_beamline_context) {
        calculate_beamline_context();
        have_beamline_context = true;
    }
    return beamline_context_sptr;
}

BmlContextPtr
Lattice_simulator::get_sliced_beamline_context()
{
    if (!have_sliced_beamline_context) {
        calculate_sliced_beamline_context();
        have_sliced_beamline_context = true;
    }
    return sliced_beamline_context_sptr;
}

bool
Lattice_simulator::is_ring()
{
    get_beamline_context();
    return (beamline_context_sptr->isRing());
}

void
Lattice_simulator::get_tunes(bool use_eigen_tune)
{
    if (!have_tunes) {
        get_beamline_context();
        if (use_eigen_tune) {
            horizontal_tune = beamline_context_sptr->getHorizontalEigenTune();
            vertical_tune = beamline_context_sptr->getVerticalEigenTune();
        } else {
            horizontal_tune = beamline_context_sptr->getHorizontalFracTune();
            vertical_tune = beamline_context_sptr->getVerticalFracTune();
        }

        have_tunes = true;
    }
}

void
Lattice_simulator::construct_aperture_extractor_map()
{
    aperture_extractor_map_sptr->set_extractor(
            Circular_aperture_operation::attribute_name,
            boost::shared_ptr<Circular_extractor >(new Circular_extractor()));

    aperture_extractor_map_sptr->set_extractor("default",
            boost::shared_ptr<Circular_extractor >(new Circular_extractor()));

    aperture_extractor_map_sptr->set_extractor(
            Elliptical_aperture_operation::attribute_name,
            boost::shared_ptr<Elliptical_extractor >(
                    new Elliptical_extractor()));

    aperture_extractor_map_sptr->set_extractor(
            Rectangular_aperture_operation::attribute_name,
            boost::shared_ptr<Rectangular_extractor >(
                    new Rectangular_extractor()));

    aperture_extractor_map_sptr->set_extractor(
            Polygon_aperture_operation::attribute_name,
            boost::shared_ptr<Polygon_extractor >(new Polygon_extractor()));

    aperture_extractor_map_sptr->set_extractor(
            Wire_elliptical_aperture_operation::attribute_name,
            boost::shared_ptr<Wire_elliptical_extractor >(
                    new Wire_elliptical_extractor()));

    aperture_extractor_map_sptr->set_extractor(
            Lambertson_aperture_operation::attribute_name,
            boost::shared_ptr<Lambertson_extractor >(
                    new Lambertson_extractor()));
}

Lattice_simulator::Lattice_simulator(Lattice_sptr lattice_sptr, int map_order) :
                lattice_sptr(lattice_sptr),
                slices(),
                have_slices(false),
                chef_lattice_sptr(new Chef_lattice(lattice_sptr)),
                extractor_map_sptr(new Operation_extractor_map),
                aperture_extractor_map_sptr(
                        new Aperture_operation_extractor_map),
                have_beamline_context(false),
                have_sliced_beamline_context(false),
                map_order(map_order),
                have_element_lattice_functions(false),
                have_slice_lattice_functions(false),
                have_element_et_lattice_functions(false),
                have_slice_et_lattice_functions(false),
                have_element_lb_lattice_functions(false),
                have_slice_lb_lattice_functions(false),
                have_element_dispersion(false),
                have_slice_dispersion(false),
                horizontal_tune(0.0),
                vertical_tune(0.0),
                have_tunes(false),
                horizontal_chromaticity(0.0),
                vertical_chromaticity(0.0),
                have_chromaticities(false),
                linear_one_turn_map(boost::extents[6][6])

{
    construct_extractor_map();
    construct_aperture_extractor_map();
    set_bucket_length();
}

Lattice_simulator::Lattice_simulator()
{
}

Lattice_simulator::Lattice_simulator(Lattice_simulator const& lattice_simulator) :
                lattice_sptr(lattice_simulator.lattice_sptr),
                slices(),
                have_slices(false),
                chef_lattice_sptr(new Chef_lattice(lattice_sptr)),
                extractor_map_sptr(new Operation_extractor_map),
                aperture_extractor_map_sptr(
                        new Aperture_operation_extractor_map),
                have_beamline_context(false),
                have_sliced_beamline_context(false),
                map_order(lattice_simulator.map_order),
                bucket_length(lattice_simulator.bucket_length),
                have_element_lattice_functions(false),
                have_slice_lattice_functions(false),
                have_element_et_lattice_functions(false),
                have_slice_et_lattice_functions(false),
                have_element_lb_lattice_functions(false),
                have_slice_lb_lattice_functions(false),
                have_element_dispersion(false),
                have_slice_dispersion(false),
                horizontal_tune(0.0),
                vertical_tune(0.0),
                have_tunes(false),
                horizontal_chromaticity(0.0),
                vertical_chromaticity(0.0),
                have_chromaticities(false),
                linear_one_turn_map(boost::extents[6][6])

{
    construct_extractor_map();
    construct_aperture_extractor_map();
}

void
Lattice_simulator::set_slices(Lattice_element_slices const& slices)
{
    this->slices = slices;
    have_slices = true;
    construct_sliced_chef_beamline();
}

Lattice_element_slices const&
Lattice_simulator::get_slices() const
{
    if (!have_slices) {
        throw std::runtime_error(
                "Lattice_simulator::get_slices called before set_slices");
    }
    return slices;
}

void
Lattice_simulator::construct_sliced_chef_beamline()
{
    if (!have_slices) {
        throw std::runtime_error(
                "Lattice_simulator::construct_sliced_chef_beamline called before set_slices");
    }
    chef_lattice_sptr->construct_sliced_beamline(slices);
}

int
Lattice_simulator::get_map_order() const
{
    return map_order;
}

Operation_extractor_map_sptr
Lattice_simulator::get_operation_extractor_map_sptr()
{
    return extractor_map_sptr;
}

Aperture_operation_extractor_map_sptr
Lattice_simulator::get_aperture_operation_extractor_map_sptr()
{
    return aperture_extractor_map_sptr;
}

Lattice &
Lattice_simulator::get_lattice()
{
    return *lattice_sptr;
}

Lattice_sptr
Lattice_simulator::get_lattice_sptr()
{
    return lattice_sptr;
}

Chef_lattice &
Lattice_simulator::get_chef_lattice()
{
    return *chef_lattice_sptr;
}

Chef_lattice_sptr
Lattice_simulator::get_chef_lattice_sptr()
{
    return chef_lattice_sptr;
}

void
Lattice_simulator::set_bucket_length()
{
    double freq(0.), freq2(0.);
    int isw = 0;
    double eps = 1e-6;
    for (Lattice_elements::const_iterator it =
            this->lattice_sptr->get_elements().begin();
            it != this->lattice_sptr->get_elements().end(); ++it) {

        if ((*it)->has_double_attribute("freq")) {
            freq = (*it)->get_double_attribute("freq");
            if ((isw == 1) && (fabs(freq - freq2) > eps)) {
                throw std::runtime_error(
                        "set_bucket_length: rf elements with different frequencies found!!");
            }
            freq2 = freq;
            isw = 1;
        }
        if (isw == 1) {
            double beta =
                    this->get_lattice_sptr()->get_reference_particle().get_beta();
            this->bucket_length = pconstants::c * beta / freq;
        } else {
            this->bucket_length = 0.0;
        }
    }
}

double
Lattice_simulator::get_bucket_length()
{
    return this->bucket_length;
}

int
Lattice_simulator::get_number_buckets()
{
    double eps = 1e-5;
    int number_buckets;
    double bl = get_bucket_length();
    double ol = this->get_lattice_sptr()->get_length();
    bl > eps ? number_buckets = int(ol / bl) : number_buckets = 1;
    return number_buckets;
}

void
Lattice_simulator::update()
{
    chef_lattice_sptr = Chef_lattice_sptr(new Chef_lattice(lattice_sptr));
    construct_extractor_map();
    construct_aperture_extractor_map();
    if (have_slices) {
        construct_sliced_chef_beamline();
    }
    have_element_lattice_functions = false;
    lattice_functions_element_map.clear();
    have_slice_lattice_functions = false;
    lattice_functions_slice_map.clear();

    have_element_et_lattice_functions = false;
    et_lattice_functions_element_map.clear();
    have_slice_et_lattice_functions = false;
    et_lattice_functions_slice_map.clear();

    have_element_lb_lattice_functions = false;
    lb_lattice_functions_element_map.clear();
    have_slice_lb_lattice_functions = false;
    lb_lattice_functions_slice_map.clear();

    have_element_dispersion = false;
    dispersion_element_map.clear();
    have_slice_dispersion = false;
    dispersion_slice_map.clear();

    have_tunes = false;
    have_beamline_context = false;
    have_chromaticities = false;
    normal_form_sage_sptr.reset();
}

void
Lattice_simulator::calculate_element_lattice_functions()
{
    if (!have_element_lattice_functions) {
        BmlPtr beamline_sptr(chef_lattice_sptr->get_beamline_sptr());
        std::vector<LattFuncSage::lattFunc > latt_func(
                get_beamline_context()->getTwissArray());
        beamline::const_iterator it = beamline_sptr->begin();
        for (unsigned int i = 0; i < latt_func.size(); ++i) {
            ElmPtr chef_element(*it);
            if (std::strcmp(chef_element->Name().c_str(),
                    Chef_lattice::internal_marker_name)) {
                Lattice_element * lattice_element_ptr(
                        &(chef_lattice_sptr->get_lattice_element(chef_element)));
                lattice_functions_element_map[lattice_element_ptr] =
                        Lattice_functions(latt_func.at(i));
            }
            ++it;
        }
        have_element_lattice_functions = true;
    }
}

void
Lattice_simulator::calculate_slice_lattice_functions()
{
    if (!have_slice_lattice_functions) {
        BmlPtr beamline_sptr(chef_lattice_sptr->get_sliced_beamline_sptr());
        std::vector<LattFuncSage::lattFunc > latt_func(
                get_sliced_beamline_context()->getTwissArray());
        beamline::const_iterator it = beamline_sptr->begin();
        for (unsigned int i = 0; i < latt_func.size(); ++i) {
            ElmPtr chef_element(*it);
            if (std::strcmp(chef_element->Name().c_str(),
                    Chef_lattice::internal_marker_name)) {
                Lattice_element_slice * lattice_element_slice_ptr(
                        &(chef_lattice_sptr->get_lattice_element_slice(
                                chef_element)));
                lattice_functions_slice_map[lattice_element_slice_ptr] =
                        latt_func.at(i);
            }
            ++it;
        }
        have_slice_lattice_functions = true;
    }
}

void
Lattice_simulator::calculate_element_et_lattice_functions()
{

    if (!have_element_et_lattice_functions) {
        BmlPtr beamline_sptr(chef_lattice_sptr->get_beamline_sptr());

        std::vector<EdwardsTengSage::Info > ET_Info(
                get_beamline_context()->getETArray());

        beamline::const_iterator it = beamline_sptr->begin();
        for (int i = 0; i < ET_Info.size(); ++i) {
            ElmPtr chef_element(*it);
            if (std::strcmp(chef_element->Name().c_str(),
                    Chef_lattice::internal_marker_name)) {
                Lattice_element * lattice_element_ptr(
                        &(chef_lattice_sptr->get_lattice_element(chef_element)));
                et_lattice_functions_element_map[lattice_element_ptr] =
                        ET_lattice_functions(ET_Info.at(i));
            }
            ++it;
        }
        have_element_et_lattice_functions = true;
    }

}

void
Lattice_simulator::calculate_slice_et_lattice_functions()
{

    if (!have_slice_et_lattice_functions) {
        BmlPtr beamline_sptr(chef_lattice_sptr->get_sliced_beamline_sptr());
        std::vector<EdwardsTengSage::Info > ET_Info(
                get_sliced_beamline_context()->getETArray());
        beamline::const_iterator it = beamline_sptr->begin();
        for (int i = 0; i < ET_Info.size(); ++i) {
            ElmPtr chef_element(*it);
            if (std::strcmp(chef_element->Name().c_str(),
                    Chef_lattice::internal_marker_name)) {
                Lattice_element_slice * lattice_element_slice_ptr(
                        &(chef_lattice_sptr->get_lattice_element_slice(
                                chef_element)));
                et_lattice_functions_slice_map[lattice_element_slice_ptr] =
                        ET_Info.at(i);
            }
            ++it;
        }
        have_slice_et_lattice_functions = true;
    }
}

void
Lattice_simulator::calculate_element_lb_lattice_functions()
{

    if (!have_element_lb_lattice_functions) {
        BmlPtr beamline_sptr(chef_lattice_sptr->get_beamline_sptr());
        std::vector<LBSage::Info > LB_Info(
                get_beamline_context()->getLBArray());
        beamline::const_iterator it = beamline_sptr->begin();
        for (int i = 0; i < LB_Info.size(); ++i) {
            ElmPtr chef_element(*it);
            if (std::strcmp(chef_element->Name().c_str(),
                    Chef_lattice::internal_marker_name)) {
                Lattice_element * lattice_element_ptr(
                        &(chef_lattice_sptr->get_lattice_element(chef_element)));
                lb_lattice_functions_element_map[lattice_element_ptr] =
                        LB_lattice_functions(LB_Info.at(i));
            }
            ++it;
        }
        have_element_lb_lattice_functions = true;
    }
}

void
Lattice_simulator::calculate_slice_lb_lattice_functions()
{

    if (!have_slice_lb_lattice_functions) {
        BmlPtr beamline_sptr(chef_lattice_sptr->get_sliced_beamline_sptr());

        std::vector<LBSage::Info > LB_Info(
                get_sliced_beamline_context()->getLBArray());
        beamline::const_iterator it = beamline_sptr->begin();
        for (int i = 0; i < LB_Info.size(); ++i) {
            ElmPtr chef_element(*it);
            if (std::strcmp(chef_element->Name().c_str(),
                    Chef_lattice::internal_marker_name)) {
                Lattice_element_slice * lattice_element_slice_ptr(
                        &(chef_lattice_sptr->get_lattice_element_slice(
                                chef_element)));
                lb_lattice_functions_slice_map[lattice_element_slice_ptr] =
                        LB_Info.at(i);
            }
            ++it;
        }
        have_slice_lb_lattice_functions = true;
    }
}

void
Lattice_simulator::calculate_element_dispersion_functions()
{
    if (!have_element_dispersion) {
        BmlPtr beamline_sptr(chef_lattice_sptr->get_beamline_sptr());
        std::vector<DispersionSage::Info > Disp_Info(
                get_beamline_context()->getDispersionArray());
        beamline::const_iterator it = beamline_sptr->begin();
        for (int i = 0; i < Disp_Info.size(); ++i) {
            ElmPtr chef_element(*it);
            if (std::strcmp(chef_element->Name().c_str(),
                    Chef_lattice::internal_marker_name)) {
                Lattice_element * lattice_element_ptr(
                        &(chef_lattice_sptr->get_lattice_element(chef_element)));
                dispersion_element_map[lattice_element_ptr] =
                        Dispersion_functions(Disp_Info.at(i));
            }
            ++it;
        }
        have_element_dispersion = true;
    }
}

void
Lattice_simulator::calculate_slice_dispersion_functions()
{

    if (!have_slice_dispersion) {
        BmlPtr beamline_sptr(chef_lattice_sptr->get_sliced_beamline_sptr());
        std::vector<DispersionSage::Info > Disp_Info(
                get_sliced_beamline_context()->getDispersionArray());
        beamline::const_iterator it = beamline_sptr->begin();
        for (int i = 0; i < Disp_Info.size(); ++i) {
            ElmPtr chef_element(*it);
            if (std::strcmp(chef_element->Name().c_str(),
                    Chef_lattice::internal_marker_name)) {
                Lattice_element_slice * lattice_element_slice_ptr(
                        &(chef_lattice_sptr->get_lattice_element_slice(
                                chef_element)));
                dispersion_slice_map[lattice_element_slice_ptr] = Disp_Info.at(
                        i);
            }
            ++it;
        }
        have_slice_dispersion = true;
    }
}

Lattice_functions const&
Lattice_simulator::get_lattice_functions(Lattice_element & lattice_element)
{
    calculate_element_lattice_functions();
    return lattice_functions_element_map[&lattice_element];
}

Lattice_functions const&
Lattice_simulator::get_lattice_functions(
        Lattice_element_slice & lattice_element_slice)
{
    calculate_slice_lattice_functions();
    return lattice_functions_slice_map[&lattice_element_slice];
}

ET_lattice_functions const&
Lattice_simulator::get_et_lattice_functions(Lattice_element & lattice_element)
{
    calculate_element_et_lattice_functions();
    return et_lattice_functions_element_map[&lattice_element];
}

ET_lattice_functions const&
Lattice_simulator::get_et_lattice_functions(
        Lattice_element_slice & lattice_element_slice)
{
    calculate_slice_et_lattice_functions();
    return et_lattice_functions_slice_map[&lattice_element_slice];
}

LB_lattice_functions const&
Lattice_simulator::get_lb_lattice_functions(Lattice_element & lattice_element)
{
    calculate_element_lb_lattice_functions();
    return lb_lattice_functions_element_map[&lattice_element];
}

LB_lattice_functions const&
Lattice_simulator::get_lb_lattice_functions(
        Lattice_element_slice & lattice_element_slice)
{
    calculate_slice_lb_lattice_functions();
    return lb_lattice_functions_slice_map[&lattice_element_slice];
}

Dispersion_functions const&
Lattice_simulator::get_dispersion_functions(Lattice_element & lattice_element)
{
    calculate_element_dispersion_functions();
    return dispersion_element_map[&lattice_element];
}

Dispersion_functions const&
Lattice_simulator::get_dispersion_functions(
        Lattice_element_slice & lattice_element_slice)
{
    calculate_slice_dispersion_functions();
    return dispersion_slice_map[&lattice_element_slice];
}

// calculate the normal form sage object, leave a shared pointer to this object
// in the lattice_simulator class member normal_form_sage_sptr
void
Lattice_simulator::calculate_normal_form()
{
    get_beamline_context();
    Mapping one_turn_map = beamline_context_sptr->getOneTurnMap();
    normal_form_sage_sptr = Normal_form_sage_sptr(
            new normalFormSage(one_turn_map,
                    reference_particle_to_chef_jet_particle(
                            lattice_sptr->get_reference_particle(), map_order),
                    map_order));
}

// return the normal_form_sage_sptr if it exists, otherwise calculate
// it first and then return it.
Normal_form_sage_sptr
Lattice_simulator::get_normal_form_sptr()
{
    if (normal_form_sage_sptr) {
        return normal_form_sage_sptr;
    } else {
        calculate_normal_form();
        return normal_form_sage_sptr;
    }
}

// checkLinearNormalForm
// check the linear part of the normal form calculation
bool
Lattice_simulator::check_linear_normal_form()
{
    Normal_form_sage_sptr nf_sptr(get_normal_form_sptr());
    return nf_sptr->checkLinearNormalForm();
}

// converts a MArray2d of particle coordinates in synergia ordering into
// an MArray2d of complex normal form coordinates stored as a0.real, a0.imag,
//  a1.real,a1.imag, a2.real, a2.imag.

void
Lattice_simulator::convert_human_to_normal(MArray2d_ref coords)
{
    Normal_form_sage_sptr nf_sptr(get_normal_form_sptr());

#if 0
    std::cout << "chef->synergia indices:" << std::endl;
    for (int j=0; j<6; ++j) {
        std::cout << j << " ->  " << get_synergia_index(j) << std::endl;
    }
    std::cout << std::endl;
    std::cout << "synergia->chef indices:" << std::endl;
    for (int j=0; j<6; ++j) {
        std::cout << j << " ->  " << get_chef_index(j) << std::endl;
    }
#endif
    const MArray2d::size_type *coords_shape = coords.shape();
    const MArray2d::index *coords_bases = coords.index_bases();

#if 0
    std::cout << "coords_shape: " << coords_shape[0] << ", " << coords_shape[1] << std::endl;
    std::cout << "coords_bases: " << coords_bases[0] << ", " << coords_bases[1] << std::endl;
#endif

    if ((coords_shape[1] != 7) || (coords_bases[1] != 0)) {
        throw std::runtime_error(
                "Lattice_simulator::convert_human_to_normal expected nx[0:7] array");
    }

    for (unsigned int i = coords_bases[0];
            i != coords_bases[0] + coords_shape[0]; ++i) {
        Vector w(6);
        VectorC a(6);

        for (int j = 0; j < 6; ++j) {
            w(get_chef_index(j)) = coords[i][j];
        }
#if 0
        std::cout << "human->normal human(chef): " << w << std::endl;
#endif
        nf_sptr->cnvDataToNormalForm(w, a);
#if 0
        std::cout << "human->normal normal(chef): " << a << std::endl;
#endif

        for (int j = 0; j < 3; ++j) {
            coords[i][2 * j] = a(j).real();
            coords[i][2 * j + 1] = a(j).imag();
        }
    }
}

// converts a MArray2d of complex normal form particle coordinates into
// human space coordinates in synergia order.
void
Lattice_simulator::convert_normal_to_human(MArray2d_ref coords)
{
    Normal_form_sage_sptr nf_sptr(get_normal_form_sptr());

    const MArray2d::size_type *coords_shape = coords.shape();
    const MArray2d::index *coords_bases = coords.index_bases();

    if ((coords_shape[1] != 7) || (coords_bases[1] != 0)) {
        throw std::runtime_error(
                "Lattice_simulator::convert_normal_to_human expected nx[0:7] array");
    }
    for (unsigned int i = coords_bases[0];
            i != coords_bases[0] + coords_shape[0]; ++i) {
        Vector w(6);
        VectorC a(6);

        for (int j = 0; j < 3; ++j) {
            a(j) = std::complex<double >(coords[i][2 * j],
                    coords[i][2 * j + 1]);
            a(j + 3) = std::conj(a(j));
        }

        // convert to human form in CHEF order
        nf_sptr->cnvDataFromNormalForm(a, w);

        // write back into synergia ordering
        for (int j = 0; j < 6; ++j) {
            coords[i][get_synergia_index(j)] = w(j);
        }
    }
}

// return a vector of the mean actions that will generate a stationary
// beam distribution having the specified standard deviations in each
// of the three planes.
std::vector<double >
Lattice_simulator::get_stationary_actions(const double stdx, const double stdy,
        const double std_cdt)
{
    Normal_form_sage_sptr nf_sptr(get_normal_form_sptr());
    // stationaryActions wants the second moments of the canonical variables
    // which are x,y,t.  Note that the input longitudinal variaable is std_cdt.
    // Convert that into stdt.
    double stdt = std_cdt / pconstants::c;

    std::vector<double > v(nf_sptr->stationaryActions(stdx, stdy, stdt));
    return v;
}

// returns the linear one turn map for the lattice and beam parameters
// for this lattice_simulator
Const_MArray2d_ref
Lattice_simulator::get_linear_one_turn_map()
{

    get_beamline_context();
    MatrixD lin_one_turn_map =
            beamline_context_sptr->getOneTurnMap().Jacobian();
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 6; ++j) {
            linear_one_turn_map[i][j] = lin_one_turn_map(get_chef_index(i),
                    get_chef_index(j));
        }
    }
    return linear_one_turn_map;
}

// this method should be used instead of the get_horizontal_tune() and get_vertical_tune()
std::pair<double, double >
Lattice_simulator::get_both_tunes(bool use_eigen_tune)
{
    get_tunes(use_eigen_tune);
    update(); // remake CHEF beamline to restore RF turned off by getHorizontalFracTune()
    return std::pair<double, double >(horizontal_tune, vertical_tune);
}

double
Lattice_simulator::get_horizontal_tune(bool use_eigen_tune)
{
    get_tunes(use_eigen_tune);
    update(); // remake CHEF beamline to restore RF turned off by getHorizontalFracTune()
    return horizontal_tune;
}

double
Lattice_simulator::get_vertical_tune(bool use_eigen_tune)
{
    get_tunes(use_eigen_tune);
    update(); // remake CHEF beamline to restore RF turned off by getHorizontalFracTune()
    return vertical_tune;
}

void
write_quad_correctors(Lattice_elements const& horizontal_correctors,
        Lattice_elements const & vertical_correctors,
        Chef_lattice & chef_lattice, std::ofstream & file)
{

    for (Lattice_elements::const_iterator le_it = horizontal_correctors.begin();
            le_it != horizontal_correctors.end(); ++le_it) {
        Chef_elements chef_elements(chef_lattice.get_chef_elements(*(*le_it)));
        for (Chef_elements::iterator ce_it = chef_elements.begin();
                ce_it != chef_elements.end(); ++ce_it) {

            double k1 = (*ce_it)->Strength() / chef_lattice.get_brho();
            (*le_it)->set_double_attribute("k1", k1);
            file << (*le_it)->get_name() << ":  QUADRUPOLE,  L="
                    << std::setprecision(5)
                    << (*le_it)->get_double_attribute("l") << ",    K1="
                    << std::setprecision(11)
                    << (*le_it)->get_double_attribute("k1") << std::endl;

        }
    }

    for (Lattice_elements::const_iterator le_it = vertical_correctors.begin();
            le_it != vertical_correctors.end(); ++le_it) {
        Chef_elements chef_elements(chef_lattice.get_chef_elements(*(*le_it)));
        for (Chef_elements::iterator ce_it = chef_elements.begin();
                ce_it != chef_elements.end(); ++ce_it) {

            double k1 = (*ce_it)->Strength() / chef_lattice.get_brho();
            (*le_it)->set_double_attribute("k1", k1);
            file << (*le_it)->get_name() << ":  QUADRUPOLE,  L="
                    << std::setprecision(5)
                    << (*le_it)->get_double_attribute("l") << ",    K1="
                    << std::setprecision(11)
                    << (*le_it)->get_double_attribute("k1") << std::endl;

        }
    }

}

// extract_quad_strengths is a local function
void
extract_quad_strengths(Lattice_elements const& correctors,
        Chef_lattice & chef_lattice, Logger & logger, int verbosity)
{
    for (Lattice_elements::const_iterator le_it = correctors.begin();
            le_it != correctors.end(); ++le_it) {
        Chef_elements chef_elements(chef_lattice.get_chef_elements(*(*le_it)));
        for (Chef_elements::iterator ce_it = chef_elements.begin();
                ce_it != chef_elements.end(); ++ce_it) {
            double scaled_strength = (*ce_it)->Strength()
                    / chef_lattice.get_brho();
            if ((*le_it)->get_length() > 0) {
                (*le_it)->set_double_attribute("k1", scaled_strength);
            } else {
                (*le_it)->set_double_attribute("k1l", scaled_strength);
            }
            if (verbosity > 1) {
                logger << std::setprecision(6);
                logger << (*le_it)->get_name() << ": " << "quad, l="
                        << (*le_it)->get_length() << ", ";
                logger << std::setprecision(12);
                if ((*le_it)->get_length() > 0.0) {
                    logger << "k1 =" << (*le_it)->get_double_attribute("k1");
                } else {
                    logger << "k1l =" << (*le_it)->get_double_attribute("k1l");
                }
                logger << std::endl;
            }
        }
    }
}

bool
get_strengths_param(Chef_elements const& elements,
        std::vector<double> & original_strengths,
        double & param)
{
    bool relative(false);
    Chef_elements::const_iterator it = elements.begin();
    double lastval = (*it)->Strength();
    const double tolerance = 1.0e-12;
    int i = 0;
    for (; it != elements.end(); ++it) {
        double val = (*it)->Strength();
        if (std::abs(val - lastval) > tolerance) {
            relative = true;
        }
        original_strengths.at(i) = val;
        lastval = val;
        ++i;
    }
    if (relative) {
        param = 1.0;
    } else {
        param = original_strengths.at(0);
    }
    return relative;
}

Chef_elements
get_quad_chef_elements(Lattice_elements const& lattice_elements,
        Chef_lattice & chef_lattice)
{
    Chef_elements retval;
    for (Lattice_elements::const_iterator le_it = lattice_elements.begin();
            le_it != lattice_elements.end(); ++le_it) {
        Chef_elements chef_elements(chef_lattice.get_chef_elements(*(*le_it)));
        for (Chef_elements::iterator ce_it = chef_elements.begin();
                ce_it != chef_elements.end(); ++ce_it) {
            if ((std::strcmp((*ce_it)->Type(), "quadrupole") == 0)
                    || (std::strcmp((*ce_it)->Type(), "thinQuad") == 0)) {
                retval.push_back(*ce_it);
            } else {
                std::string message(
                        "Lattice_simulator::adjust_tunes: Lattice_element ");
                message += (*le_it)->get_name();
                message += " of type ";
                message += (*le_it)->get_type();
                message += " cannot be used as a corrector because it has a";
                message += " chef element of type ";
                message += (*ce_it)->Type();
                throw std::runtime_error(message.c_str());
            }
        }
    }
    return retval;
}

struct Adjust_tunes_params
{
    double h_nu_target, v_nu_target;
    Chef_elements h_elements, v_elements;
    BmlPtr beamline_sptr;
    BmlContextPtr beamline_context_sptr;
    bool h_relative, v_relative;
    std::vector<double > h_original_strengths, v_original_strengths;
    double h_param, v_param;
    Logger & logger;
    int verbosity;
    Adjust_tunes_params(double horizontal_tune, double vertical_tune,
            Lattice_elements const& horizontal_correctors,
            Lattice_elements const& vertical_correctors,
            Chef_lattice & chef_lattice, BmlContextPtr beamline_context_sptr,
            Logger & logger, int verbosity) :
                    h_nu_target(horizontal_tune),
                    v_nu_target(vertical_tune),
                    h_elements(
                            get_quad_chef_elements(horizontal_correctors,
                                    chef_lattice)),
                    v_elements(
                            get_quad_chef_elements(vertical_correctors,
                                    chef_lattice)),
                    beamline_sptr(chef_lattice.get_beamline_sptr()),
                    beamline_context_sptr(beamline_context_sptr),
                    h_original_strengths(horizontal_correctors.size()),
                    v_original_strengths(vertical_correctors.size()),
                    h_param(1.0),
                    v_param(1.0),
                    logger(logger),
                    verbosity(verbosity)
    {
        h_relative = get_strengths_param(h_elements, h_original_strengths,
                h_param);
        v_relative = get_strengths_param(v_elements, v_original_strengths,
                v_param);
        if (verbosity > 1) {
            logger << "Lattice_simulator::adjust_tunes: h_relative = "
                    << h_relative << ", v_relative = " << v_relative
                    << std::endl;
        }
    }
};

int
adjust_tunes_function(const gsl_vector * x, void * params, gsl_vector * f)
{
    Adjust_tunes_params atparams(*static_cast<Adjust_tunes_params * >(params));
    double h_param = gsl_vector_get(x, 0);
    int i = 0;
    for (Chef_elements::iterator it = atparams.h_elements.begin();
            it != atparams.h_elements.end(); ++it) {
        if (atparams.h_relative) {
            (*it)->setStrength(h_param * atparams.h_original_strengths.at(i));
        } else {
            (*it)->setStrength(h_param);
        }
        ++i;
    }
    double v_param = gsl_vector_get(x, 1);
    i = 0;
    for (Chef_elements::iterator it = atparams.v_elements.begin();
            it != atparams.v_elements.end(); ++it) {
        if (atparams.v_relative) {
            (*it)->setStrength(h_param * atparams.v_original_strengths.at(i));
        } else {
            (*it)->setStrength(v_param);
        }
        ++i;
    }
    if (atparams.verbosity > 2) {
        atparams.logger
                << "Lattice_simulator::adjust_tunes: adjust_tunes_function: ";
        if (atparams.h_relative) {
            atparams.logger << "horizontal strengths = " << atparams.h_param
                    << "*" << container_to_string(atparams.h_original_strengths)
                    << std::endl;
        } else {
            atparams.logger << "horizontal strength = " << atparams.h_param
                    << std::endl;
        }
        atparams.logger
                << "Lattice_simulator::adjust_tunes: adjust_tunes_function: ";
        if (atparams.v_relative) {
            atparams.logger << "vertical strengths = " << atparams.v_param
                    << "*" << container_to_string(atparams.v_original_strengths)
                    << std::endl;
        } else {
            atparams.logger << "vertical strength = " << atparams.v_param
                    << std::endl;
        }
    }
    atparams.beamline_context_sptr->reset();
    double nu_h = atparams.beamline_context_sptr->getHorizontalFracTune();
    double nu_v = atparams.beamline_context_sptr->getVerticalFracTune();
    if (atparams.verbosity > 2) {
        atparams.logger << std::setprecision(10);
        atparams.logger << "Lattice_simulator::adjust_tunes: adjust_tunes_function: horizontal tune = " << nu_h << ", vertical tune = "
                << nu_v << std::endl;
    }
    gsl_vector_set(f, 0, nu_h - atparams.h_nu_target);
    gsl_vector_set(f, 1, nu_v - atparams.v_nu_target);

    return GSL_SUCCESS;
}

void
Lattice_simulator::adjust_tunes(double horizontal_tune, double vertical_tune,
        Lattice_elements const& horizontal_correctors,
        Lattice_elements const& vertical_correctors, double tolerance,
        int verbosity)
{
    Logger logger(0);
    get_beamline_context();

    Adjust_tunes_params atparams(horizontal_tune, vertical_tune,
            horizontal_correctors, vertical_correctors, *chef_lattice_sptr,
            beamline_context_sptr, logger, verbosity);
    const size_t n = 2;
    gsl_multiroot_function f = { &adjust_tunes_function, n, (void*) &atparams };
    gsl_vector * x = gsl_vector_alloc(n);
    gsl_vector_set(x, 0, atparams.h_param);
    gsl_vector_set(x, 1, atparams.v_param);
    const gsl_multiroot_fsolver_type * T = gsl_multiroot_fsolver_hybrids;
    gsl_multiroot_fsolver * s = gsl_multiroot_fsolver_alloc(T, n);
    gsl_multiroot_fsolver_set(s, &f, x);

    int status;
    size_t iter = 0;
    const int max_iter = 100;
    do {
        iter++;
        if (verbosity > 1) {
            logger << "Lattice_simulator::adjust_tunes: iteration " << iter << std::endl;
        }
        status = gsl_multiroot_fsolver_iterate(s);
        if (status) break;
        status = gsl_multiroot_test_residual(s->f, tolerance);
    }
    while (status == GSL_CONTINUE && iter < max_iter);

    gsl_multiroot_fsolver_free(s);
    gsl_vector_free(x);

    if (iter >= max_iter) {
        throw std::runtime_error(
                "Lattice_elements::adjust_tunes: solver failed to converge");
    }

    if (verbosity > 1) {
        logger << "Lattice_simulator::adjust_tunes: begin new corrector quads" << std::endl;
    }
    extract_quad_strengths(horizontal_correctors, *chef_lattice_sptr, logger, verbosity);
    extract_quad_strengths(vertical_correctors, *chef_lattice_sptr, logger, verbosity);
    if (verbosity > 1) {
        logger << "Lattice_simulator::adjust_tunes: end new corrector quads" << std::endl;
    }
    update();

    if (verbosity > 0) {
        logger << "Lattice_simulator::adjust_tunes: convergence achieved in "
                << iter << " iterations" << std::endl;
        logger << std::setprecision(decimal_digits(tolerance)+2);
        logger << "Lattice_simulator::adjust_tunes: final horizontal tune = "
                << get_horizontal_tune() << ", final vertical tune = "
                << get_vertical_tune() << std::endl;
    }
}

void
calculate_tune_and_cdt(double momentum, double dpp, BmlPtr & beamline_sptr, 
		       BmlPtr & beamline0_sptr, double & tune_h, double &  tune_v, 
		       double & c_delta_t)
{
	Proton newprobe;
	newprobe.SetReferenceMomentum(momentum * (1.0 + dpp));
	newprobe.setStateToZero();
        beamline_sptr->setEnergy(newprobe.ReferenceEnergy());
	BeamlineContext probecontext(newprobe, beamline_sptr);           
        probecontext.handleAsRing();
	//tune_h = probecontext.getHorizontalFracTune();
        //tune_v = probecontext.getVerticalFracTune();
	tune_h  = probecontext.getHorizontalEigenTune();
        tune_v  = probecontext.getVerticalEigenTune();
	beamline0_sptr->propagate(newprobe);
	c_delta_t=newprobe.get_cdt();
}
			       

void
Lattice_simulator::get_chromaticities(double dpp)
{
    if (!have_chromaticities) {

        if (Jet__environment::getLastEnv() == 0) {
            JetParticle::createStandardEnvironments(map_order);
        }
        double momentum(lattice_sptr->get_reference_particle().get_momentum());
        Proton probe;
        probe.SetReferenceMomentum(momentum);
        probe.setStateToZero();
        BmlPtr beamline_sptr(chef_lattice_sptr->get_beamline_sptr()->Clone());
        beamline_sptr->setEnergy(probe.ReferenceEnergy());
        BmlPtr copy_beamline_sptr(beamline_sptr->Clone());
        double gamma = probe.Gamma();
        double cT0 = beamline_sptr->OrbitLength(probe) / probe.Beta();



	

	double tune_h_plus, tune_h_minus ;
        double tune_v_plus, tune_v_minus;
	double c_delta_t_plus, c_delta_t_minus;
	calculate_tune_and_cdt(momentum, dpp, beamline_sptr, copy_beamline_sptr, 
			       tune_h_plus,  tune_v_plus, c_delta_t_plus);			       
	calculate_tune_and_cdt(momentum, -dpp, beamline_sptr, copy_beamline_sptr, 
			       tune_h_minus,  tune_v_minus, c_delta_t_minus);		       
			       
			       
	horizontal_chromaticity=0.5*(tune_h_plus-tune_h_minus)/dpp;
	vertical_chromaticity=0.5*(tune_v_plus-tune_v_minus)/dpp;
	slip_factor =0.5*(c_delta_t_plus-c_delta_t_minus)/cT0 / dpp;
	momentum_compaction = slip_factor + 1. / gamma / gamma;
        have_chromaticities = true;	

    }
}


double
Lattice_simulator::get_slip_factor(double dpp)
{
    get_chromaticities(dpp);
    return slip_factor;
}

double
Lattice_simulator::get_momentum_compaction(double dpp)
{
    get_chromaticities(dpp);
    return momentum_compaction;
}

double
Lattice_simulator::get_horizontal_chromaticity(double dpp)
{
    get_chromaticities(dpp);
    return horizontal_chromaticity;
}

double
Lattice_simulator::get_vertical_chromaticity(double dpp)
{
    get_chromaticities(dpp);
    return vertical_chromaticity;
}

void
write_sextupole_correctors(Lattice_elements const& horizontal_correctors,
        Lattice_elements const & vertical_correctors,
        Chef_lattice & chef_lattice, std::ofstream & file)
{

    for (Lattice_elements::const_iterator le_it = horizontal_correctors.begin();
            le_it != horizontal_correctors.end(); ++le_it) {
        file << (*le_it)->get_name() << ":  SEXTUPOLE,  L="
                << std::setprecision(5) << (*le_it)->get_double_attribute("l")
                << ",    K2=" << std::setprecision(11)
                << (*le_it)->get_double_attribute("k2") << std::endl;
    }

    for (Lattice_elements::const_iterator le_it = vertical_correctors.begin();
            le_it != vertical_correctors.end(); ++le_it) {
        file << (*le_it)->get_name() << ":  SEXTUPOLE,  L="
                << std::setprecision(5) << (*le_it)->get_double_attribute("l")
                << ",    K2=" << std::setprecision(11)
                << (*le_it)->get_double_attribute("k2") << std::endl;
    }
}

// extract_sextupole_strengths is a local function
void
extract_sextupole_strengths(Lattice_elements const& correctors,
        Chef_lattice & chef_lattice)
{
    for (Lattice_elements::const_iterator le_it = correctors.begin();
            le_it != correctors.end(); ++le_it) {
        Chef_elements chef_elements(chef_lattice.get_chef_elements(*(*le_it)));
        for (Chef_elements::iterator ce_it = chef_elements.begin();
                ce_it != chef_elements.end(); ++ce_it) {

            double k2 = 2. * (*ce_it)->Strength() / chef_lattice.get_brho();
            (*le_it)->set_double_attribute("k2", k2);

        }
    }
}

void
set_chef_chrom_correctors(Lattice_elements const& correctors,
        Chef_lattice & chef_lattice, BeamlineContext & beamline_context,
        bool horizontal)
{
    for (Lattice_elements::const_iterator le_it = correctors.begin();
            le_it != correctors.end(); ++le_it) {
        Chef_elements chef_elements(chef_lattice.get_chef_elements(*(*le_it)));
        for (Chef_elements::iterator ce_it = chef_elements.begin();
                ce_it != chef_elements.end(); ++ce_it) {

            if ((std::strcmp((*ce_it)->Type(), "sextupole") == 0)
                    && ((!(*le_it)->has_double_attribute("tilt")
                            && !(*le_it)->has_string_attribute("tilt"))
                            || ((*le_it)->has_double_attribute("tilt")
                                    && fabs(
                                            (*le_it)->get_double_attribute(
                                                    "tilt")) < 1.e-6))) {
                if (horizontal) {
                    beamline_context.addHChromCorrector(
                            boost::dynamic_pointer_cast<sextupole >(*ce_it));
                } else {
                    beamline_context.addVChromCorrector(
                            boost::dynamic_pointer_cast<sextupole >(*ce_it));
                }
            } else if ((std::strcmp((*ce_it)->Type(), "thinSextupole") == 0)
                    && ((!(*le_it)->has_double_attribute("tilt")
                            && !(*le_it)->has_string_attribute("tilt"))
                            || ((*le_it)->has_double_attribute("tilt")
                                    && fabs(
                                            (*le_it)->get_double_attribute(
                                                    "tilt")) < 1.e-6))) {

                if (horizontal) {
                    beamline_context.addHChromCorrector(
                            boost::dynamic_pointer_cast<thinSextupole >(
                                    *ce_it));
                } else {
                    beamline_context.addVChromCorrector(
                            boost::dynamic_pointer_cast<thinSextupole >(
                                    *ce_it));
                }
            } else {
                /*std::stringstream hda_tilt, hsa_tilt, gda_tilt ;
                 hda_tilt<<(*le_it)->has_double_attribute("tilt");
                 hsa_tilt<<(*le_it)->has_string_attribute("tilt");
                 gda_tilt<<(*le_it)->get_double_attribute("tilt");*/
                std::string message(
                        "Lattice_simulator::adjust_chromaticities: Lattice_element ");
                message += (*le_it)->get_name();
                message += " of type ";
                message += (*le_it)->get_type();
                message += " cannot be used as a corrector because it has a";
                message += " chef element of type ";
                message += (*ce_it)->Type();
                message += " or it is skewed (i.e. nonzero tilt)";
// 		message += "; has_double_attribute tilt: ";
// 		message += hda_tilt.str();
// 		message += " ,  has_string_attribute tilt: ";
// 		message += hsa_tilt.str();
// 		message += "  tilt: ";
// 		message += gda_tilt.str();
                throw std::runtime_error(message.c_str());
            }
        }
    }
}

void
Lattice_simulator::adjust_chromaticities(double horizontal_chromaticity,
        double vertical_chromaticity,
        Lattice_elements const& horizontal_correctors,
        Lattice_elements const& vertical_correctors, double tolerance,
        int max_steps)
{
    if (Jet__environment::getLastEnv() == 0) {
        JetParticle::createStandardEnvironments(map_order);
    }
    BmlPtr beamline_sptr(chef_lattice_sptr->get_beamline_sptr());
    BeamlineContext beamline_context(
            reference_particle_to_chef_particle(
                    lattice_sptr->get_reference_particle()), beamline_sptr);
    beamline_context.handleAsRing();
    set_chef_chrom_correctors(horizontal_correctors, *chef_lattice_sptr,
            beamline_context, true);
    set_chef_chrom_correctors(vertical_correctors, *chef_lattice_sptr,
            beamline_context, false);

    double chr_h = get_horizontal_chromaticity();
    double chr_v = get_vertical_chromaticity();
    int rank = Commxx().get_rank();
    if (rank == 0) {
        std::cout << "        Initial chromaticity: horizontal    : " << chr_h
                << std::endl;
        ;
        std::cout << "                       vertical      : " << chr_v
                << std::endl;
        std::cout << "        Desired chromaticity: horizontal      : "
                << horizontal_chromaticity << std::endl;
        std::cout << "                     vertical        : "
                << vertical_chromaticity << std::endl;
    }

    double dh = horizontal_chromaticity - chr_h;
    double dv = vertical_chromaticity - chr_v;
    int count = 0;

    while (((fabs(dh) > tolerance) || (fabs(dv) > tolerance))
            && (count < max_steps)) {
        if (rank == 0) {
            std::cout << " chr_h=" << chr_h << "  chr_v  =" << chr_v
                    << "  count=" << count << std::endl;
            std::cout << " dh=" << dh << "  dv=" << dv << "  count=" << count
                    << std::endl;
        }
        int status = beamline_context.changeChromaticityBy(dh, dv);

        if (status == BeamlineContext::NO_CHROMATICITY_ADJUSTER) {
            throw std::runtime_error(
                    "Lattice_simulator::adjust_chromaticities: no corrector elements found");
        } else if (status != BeamlineContext::OKAY) {
            throw std::runtime_error(
                    "Lattice_simulator::adjust_chromaticities: failed with unknown status");
        }

        //  extract_sextupole_strengths(horizontal_correctors, *chef_lattice_sptr);
        //  extract_sextupole_strengths(vertical_correctors, *chef_lattice_sptr);

        have_chromaticities = false;
        chr_h = get_horizontal_chromaticity();
        chr_v = get_vertical_chromaticity();
        dh = horizontal_chromaticity - chr_h;
        dv = vertical_chromaticity - chr_v;
        count++;

    }

    extract_sextupole_strengths(horizontal_correctors, *chef_lattice_sptr);
    extract_sextupole_strengths(vertical_correctors, *chef_lattice_sptr);
    update();

    if (rank == 0) {
        std::ofstream file;
        file.open("sextupole_correctors.txt");
        file
                << " ! the sextupole correctors are set for chromaticity (H, V):  ("
                << chr_h << " ,  " << chr_v << " ) " << std::endl;
        write_sextupole_correctors(horizontal_correctors, vertical_correctors,
                *chef_lattice_sptr, file);
        file.close();
    }

    have_chromaticities = false;
    if (count == max_steps) {
        throw std::runtime_error(
                "Lattice_simulator::adjust_chromaticities: Convergence not achieved. Increase the maximum number of steps.");
    }
    if (rank == 0) {
        std::cout << " Chromaticity adjusted in " << count << " steps"
                << std::endl;
        std::cout << "  final    chromaticity:  horizontal    : " << chr_h
                << "     vertical     : " << chr_v << std::endl;
    }

}

void
Lattice_simulator::print_cs_lattice_functions()
{
    try {
        int rank = Commxx().get_rank();
        if (rank == 0) {

            std::ofstream file;
            file.open("CS_lattice_functions.dat");

            file
                    << "#    element      arc[m]     beta_x[m]      beta_y[m]     alpha_x     alpha_y      "
                    << " psi_x      psi_y       D_x[m]      D_y[m]      Dprime_x     Dprime_y"
                    << std::endl;
            file << "#" << std::endl;

            for (Lattice_elements::const_iterator it =
                    this->lattice_sptr->get_elements().begin();
                    it != this->lattice_sptr->get_elements().end(); ++it) {

                Lattice_functions lfs = get_lattice_functions(*(*it));

                file << std::setw(19) << (*it)->get_name() << "    "
                        << lfs.arc_length << "   " << lfs.beta_x << "    "
                        << lfs.beta_y << "   " << lfs.alpha_x << "   "
                        << lfs.alpha_y << "    " << lfs.psi_x << "   "
                        << lfs.psi_y << "   " << lfs.D_x << "    " << lfs.D_y
                        << "   " << lfs.Dprime_x << "   " << lfs.Dprime_y
                        << std::endl;

            }
            file.close();
        }
    }
    catch (std::exception const& e) {
        std::cout << e.what() << std::endl;
    }
}

void
Lattice_simulator::print_et_lattice_functions()
{

    try {
        int rank = Commxx().get_rank();
        if (rank == 0) {

            std::ofstream file;
            file.open("ET_lattice_functions.dat");

            file
                    << "#    element      arc[m]     beta_x[m]      beta_y[m]     alpha_x     alpha_y      "
                    << " phi_x " << std::endl;
            file << "#" << std::endl;

            for (Lattice_elements::const_iterator it =
                    this->lattice_sptr->get_elements().begin();
                    it != this->lattice_sptr->get_elements().end(); ++it) {

                ET_lattice_functions etinfo = get_et_lattice_functions(*(*it));

                file << std::setw(19) << (*it)->get_name() << "    "
                        << etinfo.arc_length << "   " << etinfo.beta_x << "    "
                        << etinfo.beta_y << "   " << etinfo.alpha_x << "   "
                        << etinfo.alpha_y << "    " << etinfo.phi << std::endl;

            }
            file.close();
        }
    }
    catch (std::exception const& e) {
        std::cout << e.what() << std::endl;
    }
}

void
Lattice_simulator::print_lb_lattice_functions()
{
    try {
        int rank = Commxx().get_rank();
        if (rank == 0) {

            std::ofstream file;
            file.open("LB_lattice_functions.dat");

            file
                    << "#    element      arc[m]     beta_1x[m]      beta_1y       beta_2x     beta_2y[m]     "
                    << "     alpha_1x     alpha_1y       alpha_2x     alpha_2y     "
                    << "     u1           u2            u3           u4       nu_1       nu_2"
                    << std::endl;
            file << "#" << std::endl;

            for (Lattice_elements::const_iterator it =
                    this->lattice_sptr->get_elements().begin();
                    it != this->lattice_sptr->get_elements().end(); ++it) {

                LB_lattice_functions lbinfo = get_lb_lattice_functions(*(*it));

                file << std::setw(19) << (*it)->get_name() << "    "
                        << lbinfo.arc_length << "   " << lbinfo.beta_1x
                        << "    " << lbinfo.beta_1y << "   " << lbinfo.beta_2x
                        << "   " << lbinfo.beta_2y << "    " << lbinfo.alpha_1x
                        << "   " << lbinfo.alpha_1y << "   " << lbinfo.alpha_2x
                        << "    " << lbinfo.alpha_2y << "     " << lbinfo.u1
                        << "     " << lbinfo.u2 << "     " << lbinfo.u3
                        << "     " << lbinfo.u4 << "     " << lbinfo.nu_1
                        << "     " << lbinfo.nu_2 << std::endl;

            }
            file.close();
        }
    }
    catch (std::exception const& e) {
        std::cout << e.what() << std::endl;
    }
}

void
Lattice_simulator::print_dispersion_closedOrbit()
{
    try {
        int rank = Commxx().get_rank();
        if (rank == 0) {

            std::ofstream file;
            file.open("Dispersion_CloseOrbit.dat");

            file
                    << "#    element     arc[m]     dispersion_x[m]     dispersion_y[m] "
                    << "     dPrime_x     dPrime_y      closedOrbit_x[m]     closedOrbit_y[m]"
                    << " closedOrbitP_x     closedOrbitP_y " << std::endl;
            file << "#" << std::endl;

            for (Lattice_elements::const_iterator it =
                    this->lattice_sptr->get_elements().begin();
                    it != this->lattice_sptr->get_elements().end(); ++it) {

                Dispersion_functions dispfs = get_dispersion_functions(*(*it));

                file << std::setw(19) << (*it)->get_name() << "    "
                        << dispfs.arc_length << "   " << dispfs.dispersion_x
                        << "   " << dispfs.dispersion_y << "   "
                        << dispfs.dPrime_x << "   " << dispfs.dPrime_y << "   "
                        << dispfs.closedOrbit_x << "   " << dispfs.closedOrbit_y
                        << "   " << dispfs.closedOrbitP_x << "   "
                        << dispfs.closedOrbitP_y << std::endl;

            }
            file.close();
        }
    }
    catch (std::exception const& e) {
        std::cout << e.what() << std::endl;
    }
}

void
Lattice_simulator::print_lattice_functions()
{
    print_cs_lattice_functions();
    print_et_lattice_functions();
    print_lb_lattice_functions();
    print_dispersion_closedOrbit();
}

template<class Archive>
    void
    Lattice_simulator::save(Archive & ar, const unsigned int version) const
    {
        ar & BOOST_SERIALIZATION_NVP(lattice_sptr);
        ar & BOOST_SERIALIZATION_NVP(slices);
        ar & BOOST_SERIALIZATION_NVP(have_slices);
        ar & BOOST_SERIALIZATION_NVP(chef_lattice_sptr);
        ar & BOOST_SERIALIZATION_NVP(extractor_map_sptr);
        ar & BOOST_SERIALIZATION_NVP(aperture_extractor_map_sptr);
        ar & BOOST_SERIALIZATION_NVP(have_beamline_context);
        ar & BOOST_SERIALIZATION_NVP(map_order);
        ar & BOOST_SERIALIZATION_NVP(bucket_length);
        ar & BOOST_SERIALIZATION_NVP(have_element_lattice_functions);
        ar & BOOST_SERIALIZATION_NVP(have_slice_lattice_functions);
        ar & BOOST_SERIALIZATION_NVP(horizontal_tune);
        ar & BOOST_SERIALIZATION_NVP(vertical_tune);
        ar & BOOST_SERIALIZATION_NVP(have_tunes);
        ar & BOOST_SERIALIZATION_NVP(horizontal_chromaticity);
        ar & BOOST_SERIALIZATION_NVP(vertical_chromaticity);
        ar & BOOST_SERIALIZATION_NVP(have_chromaticities);
        ar & BOOST_SERIALIZATION_NVP(lattice_functions_element_map);
        ar & BOOST_SERIALIZATION_NVP(lattice_functions_slice_map);
        ar & BOOST_SERIALIZATION_NVP(et_lattice_functions_element_map);
        ar & BOOST_SERIALIZATION_NVP(et_lattice_functions_slice_map);
        ar & BOOST_SERIALIZATION_NVP(lb_lattice_functions_element_map);
        ar & BOOST_SERIALIZATION_NVP(lb_lattice_functions_slice_map);
        ar & BOOST_SERIALIZATION_NVP(dispersion_element_map);
        ar & BOOST_SERIALIZATION_NVP(dispersion_slice_map);
        ar & BOOST_SERIALIZATION_NVP(linear_one_turn_map);
    }
template<class Archive>
    void
    Lattice_simulator::load(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_NVP(lattice_sptr);
        ar & BOOST_SERIALIZATION_NVP(slices);
        ar & BOOST_SERIALIZATION_NVP(have_slices);
        ar & BOOST_SERIALIZATION_NVP(chef_lattice_sptr);
        ar & BOOST_SERIALIZATION_NVP(extractor_map_sptr);
        ar & BOOST_SERIALIZATION_NVP(aperture_extractor_map_sptr);
        ar & BOOST_SERIALIZATION_NVP(have_beamline_context);
        ar & BOOST_SERIALIZATION_NVP(map_order);
        ar & BOOST_SERIALIZATION_NVP(bucket_length);
        ar & BOOST_SERIALIZATION_NVP(have_element_lattice_functions);
        ar & BOOST_SERIALIZATION_NVP(have_slice_lattice_functions);
        ar & BOOST_SERIALIZATION_NVP(horizontal_tune);
        ar & BOOST_SERIALIZATION_NVP(vertical_tune);
        ar & BOOST_SERIALIZATION_NVP(have_tunes);
        ar & BOOST_SERIALIZATION_NVP(horizontal_chromaticity);
        ar & BOOST_SERIALIZATION_NVP(vertical_chromaticity);
        ar & BOOST_SERIALIZATION_NVP(have_chromaticities);
        ar & BOOST_SERIALIZATION_NVP(lattice_functions_element_map);
        ar & BOOST_SERIALIZATION_NVP(lattice_functions_slice_map);
        ar & BOOST_SERIALIZATION_NVP(et_lattice_functions_element_map);
        ar & BOOST_SERIALIZATION_NVP(et_lattice_functions_slice_map);
        ar & BOOST_SERIALIZATION_NVP(lb_lattice_functions_element_map);
        ar & BOOST_SERIALIZATION_NVP(lb_lattice_functions_slice_map);
        ar & BOOST_SERIALIZATION_NVP(dispersion_element_map);
        ar & BOOST_SERIALIZATION_NVP(dispersion_slice_map);
        ar & BOOST_SERIALIZATION_NVP(linear_one_turn_map);
        if (have_beamline_context) {
            calculate_beamline_context();
        }
        normal_form_sage_sptr.reset();
    }

template
void
Lattice_simulator::save<boost::archive::binary_oarchive >(
        boost::archive::binary_oarchive & ar, const unsigned int version) const;
template
void
Lattice_simulator::save<boost::archive::xml_oarchive >(
        boost::archive::xml_oarchive & ar, const unsigned int version) const;

template
void
Lattice_simulator::load<boost::archive::binary_iarchive >(
        boost::archive::binary_iarchive & ar, const unsigned int version);
template
void
Lattice_simulator::load<boost::archive::xml_iarchive >(
        boost::archive::xml_iarchive & ar, const unsigned int version);

Lattice_simulator::~Lattice_simulator()
{
}

