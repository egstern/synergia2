
#include "synergia/lattice/madx_reader.h"
#include "synergia/simulation/independent_stepper_elements.h"
#include "synergia/simulation/lattice_simulator.h"
#include "synergia/simulation/propagator.h"
#include "synergia/utils/cereal_files.h"
#include "synergia/utils/utils.h"

#include <iostream>

void
run()
{
  Logger screen(0, LoggerV::DEBUG);
  Logger log(0, LoggerV::INFO_TURN);

  // This lattice has just a regular sbend
  const std::string sbend = R"(
	  fmag: sbend, l=2.889612, angle=0.07074218219630160065;
          dz: drift, l=0;
          model: line=(dz, fmag, dz);
          beam, particle=proton, energy=0.8+pmass;
        )";


  // read lattice
  MadX_reader reader;
  reader.parse(sbend);
  
  auto lattice = reader.get_lattice("model");

  // tune the lattice
  lattice.set_all_string_attribute("extractor_type", "libff");
  Lattice_simulator::tune_circular_lattice(lattice);

  lattice.print(screen);

  // one turn map
  const int order =
    3; // higher orders use more memory and may not compile on many systems
  auto mapping = Lattice_simulator::get_one_turn_map<order>(lattice);

  for (int comp = 0; comp < 6; ++comp) {
    auto trigon = mapping[comp];
    std::cout << "\n\nComponent " << comp << "\n";

    trigon.each_term([&trigon](int i, auto const& inds, auto term) {
      std::cout << "power = " << inds.size();

      std::array<int, trigon.dim> exp{0};
      for (auto const& x : inds) ++exp[x];

      std::cout << ", exp = [";
      for (int x = 0; x < exp.size() - 1; ++x) std::cout << exp[x] << ", ";
      std::cout << exp.back() << "]";

      std::cout << ", term = " << term << "\n";
    });
  }
}

int
main(int argc, char** argv)
{
  synergia::initialize(argc, argv);

  run();

  synergia::finalize();
  return 0;
}
