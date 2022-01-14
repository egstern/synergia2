#include "mx_expr.h"
#include "madx.h"

#include <stdexcept>
#include <cmath>
#include <limits>

using namespace synergia;
using namespace boost;


// mx_calculator
double mx_calculator::nan = std::numeric_limits<double>::quiet_NaN();

double 
  mx_calculator::operator()(double val) const
{
  return val;
}

double 
  mx_calculator::operator()(std::string const & ref) const
{
  if( mx==NULL ) {
    if( std::isnan(def) )  
      throw std::runtime_error("Unable to locate reference " + ref);
    else 
      return def;
  }
  return mx->variable_as_number(ref, def);
}

double 
  mx_calculator::operator()(string_pair_t const & ref) const
{
  if( mx==NULL ) {
    if( std::isnan(def) ) 
      throw std::runtime_error("Unable to locate reference " + ref.first + "->" + ref.second);
    else 
      return def;
  }
  return mx->command(ref.first).attribute_as_number(ref.second, def);
}

double 
  mx_calculator::operator()(nop_t const & n) const
{
  return boost::apply_visitor(*this, n.expr);
}

double 
  mx_calculator::operator()(uop_t const & u) const
{
  return u.func( boost::apply_visitor(*this, u.param) );
}

double 
  mx_calculator::operator()(bop_t const & b) const
{
  return b.func( boost::apply_visitor(*this, b.lhs)
               , boost::apply_visitor(*this, b.rhs) );
}

// mx_ref_checker
bool
mx_expr_ref_checker::operator()(double val) const
{ 
    return false;
}

bool
mx_expr_ref_checker::operator()(std::string const & ref) const
{
    return true;
}

bool
mx_expr_ref_checker::operator()(string_pair_t const & ref) const
{
    return true;
}

bool
mx_expr_ref_checker::operator()(nop_t const & n) const
{
    return boost::apply_visitor(*this, n.expr);
}

bool
mx_expr_ref_checker::operator()(uop_t const & u) const
{
    return boost::apply_visitor(*this, u.param);
}

bool
mx_expr_ref_checker::operator()(bop_t const & b) const
{
    return boost::apply_visitor(*this, b.lhs) 
             || boost::apply_visitor(*this, b.rhs);
}

// util functions
double
  synergia::mx_eval(mx_expr const & expr)
{
  return boost::apply_visitor(mx_calculator(), expr);
}

double
  synergia::mx_eval(mx_expr const & expr, MadX const & mx)
{
  return boost::apply_visitor(mx_calculator(mx), expr);
}

std::string
  synergia::mx_expr_refstr(mx_expr const & expr)
{
  mx_expr ex = get<nop_t>(get<nop_t>(get<nop_t>(expr).expr).expr).expr;

  if (ex.which() != 1) // string
    throw std::runtime_error("unable to get ref string from mx_expr");

  return get<std::string>(ex);
}


bool 
  synergia::mx_expr_is_number(mx_expr const& expr)
{
  return not boost::apply_visitor(mx_expr_ref_checker(), expr);
}



