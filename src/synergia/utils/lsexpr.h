#ifndef LSEXPR_H
#define LSEXPR_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

struct Lsexpr;

struct Lsexpr
{
    bool has_label;
    std::string label;
    bool is_atom;
    std::string atom;
    std::vector<Lsexpr> sequence;

    Lsexpr() :
        has_label(false)
      , label("")
      , is_atom(false)
      , atom("")
      , sequence()
    { }

    Lsexpr(std::string const& atom) :
        has_label(false)
      , label("")
      , is_atom(true)
      , atom(atom)
      , sequence()
    { }

    Lsexpr(int atom) :
        has_label(false)
      , label("")
      , is_atom(true)
      , atom()
      , sequence()
    {
        std::stringstream ss;
        ss << atom;
        this->atom = ss.str();
    }

    Lsexpr(double atom) :
        has_label(false)
      , label("")
      , is_atom(true)
      , atom(atom)
      , sequence()
    {
        std::stringstream ss;
        const int precision = 15;
        ss.precision(precision);
        ss << atom;
        this->atom = ss.str();
    }

    void set_label(std::string const& label)
    {
        this->label = label;
        has_label = true;
    }
};

Lsexpr read_lsexpr(std::istream & stream);

void write_lsexpr(Lsexpr const& lsexpr, std::ostream & stream);

#endif // LSEXPR_H
