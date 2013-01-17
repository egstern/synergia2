
#include "mx_parse.h"

#include <cmath>
#include <limits>
#include <iterator>
#include <fstream>
#include <sstream>

#include <boost/any.hpp>

#include <boost/math/constants/constants.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include <boost/fusion/include/std_pair.hpp>


namespace ascii = ::boost::spirit::ascii;
namespace phx   = ::boost::phoenix;
namespace qi    = ::boost::spirit::qi;
namespace ql    = ::boost::spirit::qi::labels;

using ascii::char_;
using ascii::no_case;
using ascii::digit;
using ascii::graph;
using ascii::space;

//using phx::ref;
//using phx::bind;

using qi::_1;
using qi::_2;
using qi::_3;
using qi::_val;
using qi::_a;
using qi::_b;
using qi::eol;
using qi::lexeme;
using qi::lit;
using qi::bool_;
using qi::int_;
using qi::double_;
using qi::no_skip;
using qi::as;
using qi::as_string;
using qi::raw;
using qi::skip;
using qi::locals;

using boost::spirit::qi::real_parser;
using boost::spirit::qi::real_policies;

using boost::any;
using boost::any_cast;

using namespace qi::labels;
using namespace std;


// ------------------------------------------------------------------

namespace
{
  double pos(double v) { return v; }
  double neg(double v) { return -v; }

  double add(double v1, double v2) { return v1+v2; }
  double sub(double v1, double v2) { return v1-v2; }
  double mul(double v1, double v2) { return v1*v2; }
  double div(double v1, double v2) { return v1/v2; }
}


namespace synergia
{
  template <typename Iterator, typename Skip>
    struct expression;

  // parse doc into a statement tree
  template <typename Iterator, typename Skip>
    struct madx_tree_parser;
}


template <typename Iterator, typename Skip>
struct synergia::expression
  : qi::grammar< Iterator, mx_expr(), Skip >
{
  // real policy to allow 'd' and 'D' as exponent
  struct ts_real_policies
      : boost::spirit::qi::real_policies<double>
  {
    static bool
      parse_exp(Iterator & first, Iterator const &)
    {
      if( string("eEdD").find(*first) != string::npos )
      {
        ++first; return true;
      }

      return false;
    }
  };


  // symbol table for constants like "pi"
  struct constant_
    : qi::symbols< typename std::iterator_traits<Iterator>::value_type, double>
  {
    constant_()
    {
      this->add ("pi"   , boost::math::constants::pi<double>()  )
                ("true" , 1.0  )
                ("false", 0.0  )
      ;
    }
  } constant;

  // symbol table for unary operator like "- 1.0"
  struct uop_
    : qi::symbols< typename std::iterator_traits<Iterator>::value_type, ufunc_t >
  {
    uop_()
    {
      this->add ("+", (ufunc_t) pos )
                ("-", (ufunc_t) neg )
      ;
    }
  } uop;

  // symbol table for binary operator like "1.0+2.2"
  struct bop1_
    : qi::symbols< typename std::iterator_traits<Iterator>::value_type, bfunc_t >
  {
    bop1_()
    {
      this->add ("+", (bfunc_t) add )
                ("-", (bfunc_t) sub )
      ;
    }
  } bop1;

  // symbol table for binary operator like "1.0*2.2"
  struct bop2_
    : qi::symbols< typename std::iterator_traits<Iterator>::value_type, bfunc_t >
  {
    bop2_()
    {
      this->add ("*", (bfunc_t) mul )
                ("/", (bfunc_t) div )
      ;
    }
  } bop2;

  // symbol table for binary operator like "1.0^2.2"
  struct bop3_
    : qi::symbols< typename std::iterator_traits<Iterator>::value_type, bfunc_t >
  {
    bop3_()
    {
      this->add ("^", (bfunc_t) std::pow  )
      ;
    }
  } bop3;



  // symbol table for unary functions like "abs"
  struct ufunc_
    : qi::symbols< typename std::iterator_traits<Iterator>::value_type, ufunc_t >
  {
    ufunc_()
    {
      this->add
          ("abs"   , (ufunc_t) std::abs  )
          ("acos"  , (ufunc_t) std::acos )
          ("asin"  , (ufunc_t) std::asin )
          ("atan"  , (ufunc_t) std::atan )
          ("ceil"  , (ufunc_t) std::ceil )
          ("cos"   , (ufunc_t) std::cos  )
          ("cosh"  , (ufunc_t) std::cosh )
          ("exp"   , (ufunc_t) std::exp  )
          ("floor" , (ufunc_t) std::floor)
          ("log"   , (ufunc_t) std::log  )
          ("log10" , (ufunc_t) std::log10)
          ("sin"   , (ufunc_t) std::sin  )
          ("sinh"  , (ufunc_t) std::sinh )
          ("sqrt"  , (ufunc_t) std::sqrt )
          ("tan"   , (ufunc_t) std::tan  )
          ("tanh"  , (ufunc_t) std::tanh )
      ;
    }
  } ufunc;

  // symbol table for binary functions like "pow"
  struct bfunc_
    : qi::symbols< typename std::iterator_traits<Iterator>::value_type, bfunc_t >
  {
    bfunc_()
    {
      this->add
          ("pow"  , (bfunc_t) std::pow  )
          ("atan2", (bfunc_t) std::atan2)
      ;
    }
  } bfunc;

  qi::rule< Iterator, mx_expr(), Skip > expr;
  qi::rule< Iterator, mx_expr(), Skip > term;
  qi::rule< Iterator, mx_expr(), Skip > factor;
  qi::rule< Iterator, mx_expr(), Skip > primary;
  qi::rule< Iterator, string() , Skip > name;
  qi::rule< Iterator, string_pair_t(), Skip > cmdref;
  qi::real_parser< double, ts_real_policies > real;

  expression()
    : expression::base_type( expr )
  {
    expr =
        term                 [_val = phx::construct<nop_t>(_1)]
        >> *( bop1 >> term ) [_val = phx::construct<bop_t>(_1, _val, _2)]
        ;  // '+' and '-'

    term =
        factor                 [_val = phx::construct<nop_t>(_1)]
        >> *( bop2 >> factor ) [_val = phx::construct<bop_t>(_1, _val, _2)]
        ;  // '*' and '/'

    factor =
        primary                 [_val = phx::construct<nop_t>(_1)]
        >> *( bop3 >> primary ) [_val = phx::construct<bop_t>(_1, _val, _2)]
        ;  // '^'

    primary =
        real                     [_val = _1]
        | ( '(' >> expr >> ')' ) [_val = phx::construct<nop_t>(_1)]
        | ( uop >> primary     ) [_val = phx::construct<uop_t>(_1, _2)]
        | ( no_case[constant]  ) [_val = _1]
        | ( no_case[ufunc] >> '(' >> expr >> ')')                [_val = phx::construct<uop_t>(_1, _2)]
        | ( no_case[bfunc] >> '(' >> expr >> ',' >> expr >> ')') [_val = phx::construct<bop_t>(_1, _2, _3)]
        | ( cmdref             ) [_val = _1]
        | ( name               ) [_val = _1]
        ;

    cmdref =
        name >> "->" >> name;

    name = char_("a-zA-Z_") >> *char_(".a-zA-Z_0-9");
  }
};


bool synergia::parse_expression( string const & s
                               , double & result )
{
  if( s.empty() ) return false;

  typedef string::const_iterator iter_t;
  typedef qi::rule<iter_t> ws_t;

  ws_t whitespace = space
                  | lit('!')  >> *(char_ - eol) >> eol
                  | lit("//") >> *(char_ - eol) >> eol;

  expression<iter_t, ws_t> parser;
  mx_expr expr;

  iter_t       begin = s.begin();
  iter_t const end   = s.end();

  bool b = qi::phrase_parse( begin, end, parser, whitespace, expr)
         && begin == end;

  result = boost::apply_visitor(synergia::mx_calculator(), expr);

  return b;
}


////
// semantic actions for madx tree parser
namespace synergia
{
  namespace
  {
    void ins_if(mx_if & if_, string const & logic, mx_tree const & block)
    { if_.assign_if(logic, block); }

    void ins_elseif(mx_if & if_, string const & logic, mx_tree const & block)
    { if_.assign_elseif(logic, block); }

    void ins_else(mx_if & if_, mx_tree const & block)
    { if_.assign_else(block); }

    void ins_while(mx_while & while_, string const & logic, mx_tree const & block)
    { while_.assign(logic, block); }

    void set_attr(mx_attr & attr, string const & name, boost::optional<char> c, any const & v)
    { if(c) attr.set_lazy_attr(name, v); else attr.set_attr(name, v); }

    void set_cmd_label(mx_command & cmd, string const & label)
    { cmd.set_label(label); }

    void set_cmd_keyword(mx_command & cmd, string const & keyword, mx_cmd_type tag)
    { cmd.set_keyword(keyword, tag); }

    void ins_cmd_attr(mx_command & cmd, mx_attr const & attr)
    { cmd.ins_attr(attr); }
  }
}

template <typename Iterator, typename Skip>
struct synergia::madx_tree_parser
  : qi::grammar< Iterator, mx_tree(), Skip >
{
  // keywords
  struct particle_keywords_
    : boost::spirit::qi::symbols< typename std::iterator_traits<Iterator>::value_type,
                                  string >
  {
    particle_keywords_()
    {
      this->add
           ("proton"     , "proton"     )
           ("electron"   , "electron"   )
           ("positron"   , "positron"   )
           ("anti-proton", "anti-proton")
           ("posmuon"    , "posmuon"    )
           ("negmuon"    , "negmuon"    )
      ;
    }
  } particle_keywords;

  struct element_keywords_
    : boost::spirit::qi::symbols< typename std::iterator_traits<Iterator>::value_type,
                                  string >
  {
    element_keywords_()
    {
      this->add
           ("drift"      , "drift"      )
           ("rbend"      , "rbend"      )
           ("sbend"      , "sbend"      )
           ("dipedge"    , "dipedge"    )
           ("quadrupole" , "quadrupole" )
           ("sextupole"  , "sextupole"  )
           ("octupole"   , "octupole"   )
           ("multipole"  , "multipole"  )
           ("solenoid"   , "solenoid"   )
           ("nllens"     , "nllens"     )
           ("hkicker"    , "hkicker"    )
           ("vkicker"    , "vkicker"    )
           ("kicker"     , "kicker"     )
           ("rfcavity"   , "rfcavity"   )
           ("rfmultipole", "rfmultipole")
           ("crabcavity" , "crabcavity" )
           ("elseparator", "elseparator")
           ("hmonitor"   , "hmonitor"   )
           ("vmonitor"   , "vmonitor"   )
           ("monitor"    , "monitor"    )
           ("instrument" , "instrument" )
           ("rcollimator", "rcollimator")
           ("ecollimator", "ecollimator")
           ("yrotation"  , "yrotation"  )
           ("srotation"  , "srotation"  )
           ("beambeam"   , "beambeam"   )
           ("matrix"     , "matrix"     )
           ("marker"     , "marker"     )
           ("endmark"    , "endmark"    )
      ;
    }
  } element_keywords;

  struct command_keywords_
    : boost::spirit::qi::symbols< typename std::iterator_traits<Iterator>::value_type,
                                  string >
  {
    command_keywords_()
    {
      this->add
           // general
           ("assign"   , "assign"   )
           ("call"     , "call"     )
           ("coguess"  , "coguess"  )
           ("create"   , "create"   )
           ("dumpsequ" , "dumpsequ" )
           ("exec"     , "exec"     )
           ("exit"     , "exit"     )
           ("fill"     , "fill"     )
           ("help"     , "help"     )
           ("option"   , "option"   )
           ("print"    , "print"    )
           ("quit"     , "quit"     )
           ("readtable", "readtable")
           ("return"   , "return"   )
           ("save"     , "save"     )
           ("savebeta" , "savebeta" )
           ("select"   , "select"   )
           ("set"      , "set"      )
           ("show"     , "show"     )
           ("stop"     , "stop"     )
           ("system"   , "system"   )
           ("tabstring", "tabstring")
           ("title"    , "title"    )
           ("use"      , "use"      )
           ("value"    , "value"    )
           ("write"    , "write"    )
           // beam specification
           ("beam"     , "beam"     )
           ("resbeam"  , "resbeam"  )
           // plot
           ("plot"     , "plot"     )
           ("resplot"  , "resplot"  )
           ("setplot"  , "setplot"  )
           // sequence editing
           ("seqedit"  , "seqedit"  )
           ("flatten"  , "flatten"  )
           ("install"  , "install"  )
           ("move"     , "move"     )
           ("remove"   , "remove"   )
           ("cycle"    , "cycle"    )
           ("reflect"  , "reflect"  )
           ("endedit"  , "endedit"  )
           // other commands
           ("twiss"    , "twiss"    )
           // build sequence ( not present in manual )
           ("sequence" , "sequence" )
           ("endsequence", "endsequence" )
      ;
    }
  } command_keywords;


  // rules
  qi::rule<Iterator, mx_tree()     , Skip> doc;
  qi::rule<Iterator, mx_tree()     , Skip> block;
  qi::rule<Iterator, mx_statement(), Skip> statement;
  qi::rule<Iterator, mx_command()  , Skip> variable;
  qi::rule<Iterator, mx_command()  , locals<mx_cmd_type>, Skip> cmd;
  qi::rule<Iterator, mx_command()  , Skip> command;
  qi::rule<Iterator, mx_if()       , Skip> if_flow;
  qi::rule<Iterator, mx_while()    , Skip> while_flow;
  qi::rule<Iterator, string()      , Skip> logic;
  qi::rule<Iterator, mx_attr()     , Skip> attr;

  qi::rule<Iterator, string()      , Skip> name;
  qi::rule<Iterator, string()      , Skip> dblq_str;
  qi::rule<Iterator, string()      , Skip> snglq_str;

  expression<Iterator              , Skip> expr;
  qi::rule<Iterator, mx_exprs()    , Skip> array;
  qi::rule<Iterator, any()         , Skip> value;


  madx_tree_parser()
    : madx_tree_parser::base_type( doc )
    , expr()
  {
    doc = as<mx_statements_t>()[*statement];

    statement =
        if_flow | while_flow | command
        ;

    block =
        as<mx_statements_t>()['{' >> *statement >> '}']
        ;

    logic =  // TODO: need more work
        '(' >> *(char_ - char_(')')) >> ')'
        ;

    if_flow =
            ( no_case["if"]     >> logic >> block ) [phx::bind(&ins_if,     _val, _1, _2)]
        >> *( no_case["elseif"] >> logic >> block ) [phx::bind(&ins_elseif, _val, _1, _2)]
        >> -( no_case["else"]            >> block ) [phx::bind(&ins_else,   _val, _1)]
        ;

    while_flow =
            ( no_case["while"]   >> logic >> block ) [phx::bind(&ins_while,  _val, _1, _2)]
        ;

    array =
        '{' >> expr % ',' >> '}'
        ;

    value =
        dblq_str | snglq_str | no_case[particle_keywords] | expr | array
        ;

    attr =
        ( name >> -char_(':') >> '=' >> value ) [phx::bind(&set_attr, _val, _1, _2, _3)]
        ;

    variable =
        attr [phx::bind(&ins_cmd_attr, _val, _1)]
        ;

    cmd =
           - ( name >> ':' )     [phx::bind(&set_cmd_label, _val, _1)]  // label
        >>   ( element_keywords [_a = MX_CMD_ELEMENT    ]
             | command_keywords [_a = MX_CMD_EXECUTABLE ]
             | name             [_a = MX_CMD_ELEMENT_REF]
             )      [phx::bind(&set_cmd_keyword, _val, _1, _a)]         // keyword
        >> * ( ',' >> attr [phx::bind(&ins_cmd_attr, _val, _1)] )       // attributes
        ;

    command =
        ( variable | cmd ) >> ';'
        ;

    name =
        lexeme[char_("a-zA-Z_") >> *char_(".a-zA-Z_0-9")]
        ;

    dblq_str =
        lexeme['"' >> +(char_ - '"') >> '"']
        ;

    snglq_str =
        lexeme['\'' >> +(char_ - '\'') >> '\'']
        ;
  }
};

bool synergia::parse_madx_tree( string const & s, mx_tree & doc )
{
  if( s.empty() ) return true;

  typedef string::const_iterator iter_t;
  typedef qi::rule<iter_t> ws_t;

  ws_t whitespace = space
                  | lit('!')  >> *(char_ - eol) >> eol
                  | lit("//") >> *(char_ - eol) >> eol;

  madx_tree_parser<iter_t, ws_t> parser;

  iter_t       begin = s.begin();
  iter_t const end   = s.end();

  bool b = qi::phrase_parse( begin, end, parser, whitespace, doc )
         && begin == end;

  return b;
}

bool synergia::parse_madx( string_t const & str, MadX & mx )
{
  // first parse the madx doc into a statement tree
  mx_tree tree;
  bool r = parse_madx_tree(str, tree);
  if( !r ) return false;

  // print for debug purpose
  //tree.print();

  // interpret the syntax tree into a MadX object
  r = tree.interpret(mx);

  return r;
}


bool synergia::parse_madx_file( string_t const & fname, MadX & mx )
{

  ifstream file;
  file.open(fname.c_str());

  if( !file.is_open() )
    throw runtime_error( "Failed to open file " + fname + " for parsing");

  file.seekg(0, std::ios::end);
  string str;
  str.reserve(file.tellg());
  file.seekg(0, std::ios::beg);

  str.assign((istreambuf_iterator<char>(file)),
              istreambuf_iterator<char>());

  file.close();

  return parse_madx( str, mx );
}







