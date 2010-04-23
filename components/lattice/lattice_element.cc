#include "lattice_element.h"
#include <algorithm>
#include <iostream>

Lattice_element::Lattice_element(std::string const& type,
        std::string const& name) :
    type(type), name(name), ancestors(), double_attributes(),
            string_attributes(), length_attribute_name("l"),
            bend_angle_attribute_name("angle")
{

}

Lattice_element::Lattice_element(Lattice_element const& lattice_element) :
            type(lattice_element.type),
            name(lattice_element.name),
            ancestors(),
            double_attributes(),
            string_attributes(),
            length_attribute_name(lattice_element.length_attribute_name),
            bend_angle_attribute_name(lattice_element.bend_angle_attribute_name)
{
    std::copy(lattice_element.ancestors.begin(),
            lattice_element.ancestors.end(), std::inserter(ancestors,
                    ancestors.begin()));
    std::copy(lattice_element.double_attributes.begin(),
            lattice_element.double_attributes.end(), std::inserter(
                    double_attributes, double_attributes.begin()));
    std::copy(lattice_element.string_attributes.begin(),
            lattice_element.string_attributes.end(), std::inserter(
                    string_attributes, string_attributes.begin()));
}

std::string const &
Lattice_element::get_type() const
{
    return type;
}

std::string const &
Lattice_element::get_name() const
{
    return name;
}

void
Lattice_element::add_ancestor(std::string const& ancestor)
{
    ancestors.push_back(ancestor);
}

std::list<std::string > const&
Lattice_element::get_ancestors() const
{
    return ancestors;
}

void
Lattice_element::set_double_attribute(std::string const& name, double value)
{
    double_attributes[name] = value;
}

bool
Lattice_element::has_double_attribute(std::string const& name) const
{
    return (double_attributes.count(name) > 0);
}

double
Lattice_element::get_double_attribute(std::string const& name) const
{
    std::map<std::string, double >::const_iterator iter =
            double_attributes.find(name);
    return iter->second;
}

std::map<std::string, double > const &
Lattice_element::get_double_attributes() const
{
    return double_attributes;
}

void
Lattice_element::set_string_attribute(std::string const& name,
        std::string const& value)
{
    string_attributes[name] = value;
}

bool
Lattice_element::has_string_attribute(std::string const& name) const
{
    return (string_attributes.count(name) > 0);
}

std::string const&
Lattice_element::get_string_attribute(std::string const& name) const
{
    std::map<std::string, std::string >::const_iterator result =
            string_attributes.find(name);
    return result->second;
}

void
Lattice_element::set_length_attribute_name(std::string const& attribute_name)
{
    length_attribute_name = attribute_name;
}

void
Lattice_element::set_bend_angle_attribute_name(
        std::string const& attribute_name)
{
    bend_angle_attribute_name = attribute_name;
}

std::map<std::string, std::string > const &
Lattice_element::get_string_attributes() const
{
    return string_attributes;
}

double
Lattice_element::get_length() const
{
    std::map<std::string, double >::const_iterator iter =
            double_attributes.find(length_attribute_name);
    double retval = 0.0;
    if (iter != double_attributes.end()) {
        retval = iter->second;
    }
    return retval;
}

double
Lattice_element::get_bend_angle() const
{
    std::map<std::string, double >::const_iterator iter =
            double_attributes.find(bend_angle_attribute_name);
    double retval = 0.0;
    if (iter != double_attributes.end()) {
        retval = iter->second;
    }
    return retval;
}

void
Lattice_element::print() const
{
    for (std::list<std::string >::const_iterator it = ancestors.begin(); it
            != ancestors.end(); ++it) {
        std::cout << (*it) << ":";
    }
    std::cout << " " << type << " ";
    std::cout << name << ": ";
    bool first_attr = true;
    for (std::map<std::string, double >::const_iterator it =
            double_attributes.begin(); it != double_attributes.end(); ++it) {
        if (first_attr) {
            first_attr = false;
        } else {
            std::cout << ", ";
        }
        std::cout << it->first << "=" << it->second;
    }
    for (std::map<std::string, std::string >::const_iterator it =
            string_attributes.begin(); it != string_attributes.end(); ++it) {
        if (first_attr) {
            first_attr = false;
        } else {
            std::cout << ", ";
        }
        std::cout << it->first << "=" << it->second;
    }
    std::cout << std::endl;
}
