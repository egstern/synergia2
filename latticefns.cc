#include <iostream>

#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cmath>

#include "PhysicsConstants.h"
#include "beamline.h"
#include "bmlfactory.h"
#include "BeamlineContext.h"
#include "RefRegVisitor.h"
#include "ClosedOrbitSage.h"
#include "InsertionList.h"

using namespace std;
extern beamline* DriftsToSlots( beamline& original );

madparser* mp = 0;

#define NEWCHEFnotreally youbetcha
void insert_markers(beamline* bmline, int num_markers_per_element,
			     double momentum)
{
  bmlnElmnt* element;
  DeepBeamlineIterator deep_beamline_iterator(bmline);
  marker *accuracy_marker = new marker("accuracy marker");
  double master_insertion_point = 0.0;
  double marker_interval;
  InsertionList insertion_list(momentum);
  while((element = deep_beamline_iterator++)) {
    if (element->Length() > 0 ) {
      marker_interval = element->Length() / 
	(static_cast<double>(num_markers_per_element) + 1.0);
      double insertion_point = master_insertion_point;
      for(int i = 0; i < num_markers_per_element; i++) {
	insertion_point += marker_interval;
	insertion_list.Append(new InsertionListElement(insertion_point,
						       accuracy_marker));
      }
      master_insertion_point += element->Length();
    } 
  }
  double s_0 = 0.0;  
  slist removed_elements;
  bmline->InsertElementsFromList(s_0, insertion_list, removed_elements);
  bmline->append(accuracy_marker);

  // The following is in reaction to the message:
// *** WARNING ***
// *** WARNING *** File: CF_sbend.cc, Line: 536
// *** WARNING *** void sbend::Split( double pc, bmlnElmnt** a, bmlnElmnt** b )
// *** WARNING *** The new, split elements must be commissioned with
// *** WARNING *** RefRegVisitor before being used.
// *** WARNING ***
  // generated by the insertions above.

  Proton my_proton(sqrt(momentum*momentum+PH_NORM_mp*PH_NORM_mp));
  RefRegVisitor compulsory_visitor(my_proton);
  bmline->accept(compulsory_visitor);
}

int main(int argc, char **argv)
{
  if (argc!=3) {
    cout << "usage: latticefns mad_file line_name\n";
    exit(1);
  }
  string mad_file(argv[1]);
  string line_name(argv[2]);
  double kinetic_energy = 0.4;
  double mass = PH_NORM_mp;
  double energy   = kinetic_energy + mass;
  double momentum = sqrt(energy*energy - mass*mass);
  const int digits=16;

  int order = 1;

#ifdef NEWCHEF
  double scale[]  = { 1.0e-3, 1.0e-3, 1.0e-3, 1.0e-3, 1.0e-3, 1.0e-3 };
  Jet__environment::BeginEnvironment(order);
  coord x(0.0),  y(0.0),  z(0.0),
    px(0.0), py(0.0), pz(0.0);
  EnvPtr<double>::Type JetEnvPtr  =  Jet__environment::EndEnvironment(scale);
  JetC::_lastEnv  =  *JetEnvPtr; // implicit conversion
#else
  Jet::BeginEnvironment(order);
  coord x(0.0),  y(0.0),  z(0.0),
    px(0.0), py(0.0), pz(0.0);
  JetC::_lastEnv = JetC::CreateEnvFrom(Jet::EndEnvironment());
#endif

  double brho = (fabs(momentum))/PH_CNV_brho_to_p;
  bmlfactory* bml_fact = new bmlfactory(mad_file.c_str(), brho);
  beamline* bmline_orig = bml_fact->create_beamline(line_name.c_str());
  bmline_orig->flatten();
  beamline* bmline = DriftsToSlots(*bmline_orig);

  insert_markers(bmline, 10, momentum);

#ifdef NEWCHEF
  Proton jfcproton(energy);
  BeamlineContext* bmln_context = new BeamlineContext(jfcproton,bmline,false);
#else
  BeamlineContext* bmln_context = new BeamlineContext(false,bmline);
#endif

  if (bmln_context->isTreatedAsRing()) {
    cout << "already treated as ring\n";
  } else {
    cout << "does not seem to be a ring, but we will use handleAsRing\n";
    bmln_context->handleAsRing();
  }

//   JetProton jpr(energy);
  
//   LattFuncSage lfsage(bmline,false);
  
//   bmline->propagate(jpr);
//   lfsage.TuneCalc(&jpr,true);
//   lfsage.Disp_Calc(&jpr);
//   lfsage.Slow_CS_Calc(&jpr);

#define notsoHARDWAY iguessso
#ifdef HARDWAY
  bmlnElmnt* be;
  DeepBeamlineIterator deep_beamline_iterator(bmline);
  const LattFuncSage::lattFunc* lf;
  ofstream fout("lat.dat");
  int i=0;
  while((be = deep_beamline_iterator++)) {
    lf = bmln_context->getLattFuncPtr(i);
    if (lf) {
      fout << setprecision(digits) << lf->arcLength << " " 
	   << lf->beta.hor << " " << lf->beta.ver << " " 
	   << lf->alpha.hor << " " << lf->alpha.ver << " " 
	   << endl;
    } else {
      cout << " no lattice function found for "
	   << be->Name() << " " << be->Type() << endl;
    }
    ++i;
  }
  fout.close();
#else
  JetProton jpr(energy);
//   DeepBeamlineIterator deep_beamline_iterator(bmline);
//   bmlnElmnt* be;
//   while(be = deep_beamline_iterator++) {
//     be->propagate(jpr);
//   }
  bmline->propagate(jpr);
  std::cout << jpr.State().Jacobian() << std::endl;
  Matrix M = jpr.State().Jacobian();
  double mxx = M.getElement(0,0);
  double mxpxp = M.getElement(3,3);
  double mxxp = M.getElement(0,3);
  double mu = acos((mxx+mxpxp)/2.0);
  std::cout << "mu = " << mu/3.141592653589793 << " pi" <<  std::endl;
  std::cout << "fractional tune = " << mu/(2.0*3.141592653589793) << std::endl;
  double beta = mxxp/sin(mu);
  std::cout << "beta =  " << beta << std::endl;
  double alpha = (mxx-mxpxp)/(2.0*sin(mu));
  std::cout << "alpha = " << alpha << std::endl;

  double myy = M.getElement(1,1);
  double mypyp = M.getElement(4,4);
  double myyp = M.getElement(1,4);
  mu = acos((myy+mypyp)/2.0);
  std::cout << "mu = " << mu/3.141592653589793 << " pi" <<  std::endl;
  std::cout << "fractional tune = " << mu/(2.0*3.141592653589793) << std::endl;
  beta = myyp/sin(mu);
  std::cout << "beta =  " << beta << std::endl;
  alpha = (myy-mypyp)/(2.0*sin(mu));
  std::cout << "alpha = " << alpha << std::endl;
#endif

  cout << "success!\n";
  return(0);
}
