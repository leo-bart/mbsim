#include "system.h"
#include <mbsim/integrators/integrators.h>

using namespace std;
using namespace MBSim;
using namespace MBSimIntegrator;

int main (int argc, char* argv[])
{
  // Einzelne Bausteine des MKS erschaffen
  System *sys = new System("TS");

  // Bausteine zum Gesamtsystem zusammenfuegen (zu einem DGL-System) 
  sys->initialize();

  // LSODEIntegrator integrator;
  // RKSuiteIntegrator integrator;
  // RADAU5Integrator integrator;
  // TimeSteppingIntegrator integrator;
  // integrator.setdt(1e-4);
  //
  DOPRI5Integrator integrator;

  integrator.setEndTime(5.0);
  integrator.setPlotStepSize(1e-3);

  //  TimeSteppingIntegrator integrator;
  //   integrator.setdt(1e-3);
  //   integrator.setdtPlot(1e-2);
  //   integrator.settEnd(2.0);

  integrator.integrate(*sys);
  cout << "finished"<<endl;

  delete sys;

  return 0;

}

