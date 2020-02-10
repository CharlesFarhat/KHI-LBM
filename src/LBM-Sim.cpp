#include "olb3D.h"
#include "olb3D.hh"   // use only generic version!
#include <cstdlib>
#include <iostream>

using namespace olb;
using namespace olb::descriptors;
using namespace olb::graphics;
using namespace std;

typedef double T;
#define DESCRIPTOR ShanChenDynOmegaForcedD3Q19Descriptor


// Parameters for the simulation setup
const int nx   = 70;
const int ny   = 35;
const int nz   = 70;
const int maxIter  = 4000;


// Stores geometry information in form of material numbers
void prepareGeometry( SuperGeometry3D<T>& superGeometry ) {

  OstreamManager clout( std::cout,"prepareGeometry" );
  clout << "Prepare Geometry ..." << std::endl;

  // Sets material number for fluid and boundary
  superGeometry.rename( 0,1 );

  Vector<T,3> origin1( -2. );
  Vector<T,3> origin2( -2., ny/2., -2. );
  Vector<T,3> origin3( -2., ny-1., -2. );
  Vector<T,3> extend1( nx+3., 2., nz+3. );
  Vector<T,3> extend2( nx+3., ny/2+2., nz+3. );

  IndicatorCuboid3D<T> bottom( extend1, origin1 );
  IndicatorCuboid3D<T> upper( extend2, origin2 );
  IndicatorCuboid3D<T> top( extend1, origin3 );

  superGeometry.rename( 1,2,upper );
  superGeometry.rename( 1,3,bottom );
  superGeometry.rename( 2,4,top );

  // Removes all not needed boundary voxels outside the surface
  //superGeometry.clean();
  // Removes all not needed boundary voxels inside the surface
  superGeometry.innerClean();
  superGeometry.checkForErrors();

  superGeometry.print();

  clout << "Prepare Geometry ... OK" << std::endl;
}


void prepareLattice( SuperLattice3D<T, DESCRIPTOR>& sLatticeOne,
                     SuperLattice3D<T, DESCRIPTOR>& sLatticeTwo,
                     Dynamics<T, DESCRIPTOR>& bulkDynamics1,
                     Dynamics<T, DESCRIPTOR>& bulkDynamics2,
                     Dynamics<T, DESCRIPTOR>& bounceBackRho0,
                     Dynamics<T, DESCRIPTOR>& bounceBackRho1,
                     SuperGeometry3D<T>& superGeometry ) {

  OstreamManager clout( std::cout,"prepareLattice" );
  clout << "Prepare Lattice ..." << std::endl;

  // The setup is: periodicity along horizontal direction, bounce-back on top
  // and bottom. The upper half is initially filled with fluid 1 + random noise,
  // and the lower half with fluid 2. Only fluid 1 experiences a forces,
  // directed downwards.

  // define lattice Dynamics
  sLatticeOne.defineDynamics( superGeometry, 0, &instances::getNoDynamics<T, DESCRIPTOR>() );
  sLatticeTwo.defineDynamics( superGeometry, 0, &instances::getNoDynamics<T, DESCRIPTOR>() );

  sLatticeOne.defineDynamics( superGeometry, 1, &bulkDynamics1 );
  sLatticeOne.defineDynamics( superGeometry, 2, &bulkDynamics1 );
  sLatticeOne.defineDynamics( superGeometry, 3, &bulkDynamics1 );
  sLatticeOne.defineDynamics( superGeometry, 4, &bulkDynamics1 );
  sLatticeTwo.defineDynamics( superGeometry, 1, &bulkDynamics2 );
  sLatticeTwo.defineDynamics( superGeometry, 2, &bulkDynamics2 );
  sLatticeTwo.defineDynamics( superGeometry, 3, &bulkDynamics2 );
  sLatticeTwo.defineDynamics( superGeometry, 4, &bulkDynamics2 );

  sLatticeOne.defineDynamics( superGeometry, 3, &bounceBackRho0 );
  sLatticeTwo.defineDynamics( superGeometry, 3, &bounceBackRho1 );
  sLatticeOne.defineDynamics( superGeometry, 4, &bounceBackRho1 );
  sLatticeTwo.defineDynamics( superGeometry, 4, &bounceBackRho0 );

  clout << "Prepare Lattice ... OK" << std::endl;
}

void setBoundaryValues( SuperLattice3D<T, DESCRIPTOR>& sLatticeOne,
                        SuperLattice3D<T, DESCRIPTOR>& sLatticeTwo,
                        T force, int iT, SuperGeometry3D<T>& superGeometry ) {

  OstreamManager clout( std::cout,"setBoundaryValues" );


  if ( iT==0 ) {

    clout << "Setting boundaries values ..." << std::endl;
    AnalyticalConst3D<T,T> noise( 4.e-2 );

    // define speed  of the 2 fluids  :
    std::vector<T> v( 3,T() );
    std::vector<T> v1 = {0,0,1};
    std::vector<T> v2 = {0,0,0};

    AnalyticalConst3D<T,T> Vf1( v1 );
    AnalyticalConst3D<T,T> zeroV( v2 );


    AnalyticalConst3D<T,T> zero( 0. );
    AnalyticalLinear3D<T,T> one( 0.,-force*descriptors::invCs2<T,DESCRIPTOR>(),0.,0.98+force*ny*descriptors::invCs2<T,DESCRIPTOR>() );
    AnalyticalConst3D<T,T> onePlus( 0.98+force*ny/2.*descriptors::invCs2<T,DESCRIPTOR>() );
    AnalyticalRandom3D<T,T> random;
    AnalyticalIdentity3D<T,T> randomOne( random*noise+one );
    AnalyticalIdentity3D<T,T> randomPlus( random*noise+onePlus );
    std::vector<T> F( 3,T() );
    F[1] = -force;
    AnalyticalConst3D<T,T> f( F );

    // for each material set the defineRhou and the Equilibrium

    sLatticeOne.defineRhoU( superGeometry, 1, zero, zeroV );
    sLatticeOne.iniEquilibrium( superGeometry, 1, zero, zeroV );
    sLatticeOne.defineField<descriptors::EXTERNAL_FORCE>( superGeometry, 1, f );
    sLatticeTwo.defineRhoU( superGeometry, 1, randomPlus, zeroV );
    sLatticeTwo.iniEquilibrium( superGeometry, 1, randomPlus, zeroV );

    sLatticeOne.defineRhoU( superGeometry, 2, randomOne, zeroV );
    sLatticeOne.iniEquilibrium( superGeometry, 2, randomOne, zeroV );
    sLatticeOne.defineField<descriptors::EXTERNAL_FORCE>( superGeometry, 2, f );
    sLatticeTwo.defineRhoU( superGeometry, 2, zero, zeroV );
    sLatticeTwo.iniEquilibrium( superGeometry, 2, zero, zeroV );

    /*sLatticeOne.defineRhoU(superGeometry, 3, zero, zeroV);
    sLatticeOne.iniEquilibrium(superGeometry, 3, zero, zeroV);
    sLatticeOne.defineField<descriptors:::EXTERNAL_FORCE>(superGeometry, 3, f);
    sLatticeTwo.defineRhoU(superGeometry, 3, one, zeroV);
    sLatticeTwo.iniEquilibrium(superGeometry, 3, one, zeroV);

    sLatticeOne.defineRhoU(superGeometry, 4, one, zeroV);
    sLatticeOne.iniEquilibrium(superGeometry, 4, one, zeroV);
    sLatticeOne.defineField<descriptors:::EXTERNAL_FORCE>(superGeometry, 4, f);
    sLatticeTwo.defineRhoU(superGeometry, 4, zero, zeroV);
    sLatticeTwo.iniEquilibrium(superGeometry, 4, zero, zeroV);*/

    // Make the lattice ready for simulation
    sLatticeOne.initialize();
    sLatticeTwo.initialize();
    clout << "Setting boundaries values ..." << std::endl;
  }
}

void getResults( SuperLattice3D<T, DESCRIPTOR>& sLatticeTwo,
                 SuperLattice3D<T, DESCRIPTOR>& sLatticeOne, int iT,
                 SuperGeometry3D<T>& superGeometry, Timer<T>& timer ) {

  OstreamManager clout( std::cout,"getResults" );
  SuperVTMwriter3D<T> vtmWriter( "rayleighTaylor3dsLatticeOne" );

  const int vtkIter  = 50;
  const int statIter = 10;

  if ( iT==0 ) {
    // Writes the geometry, cuboid no. and rank no. as vti file for visualization
    SuperLatticeGeometry3D<T, DESCRIPTOR> geometry( sLatticeOne, superGeometry );
    SuperLatticeCuboid3D<T, DESCRIPTOR> cuboid( sLatticeOne );
    SuperLatticeRank3D<T, DESCRIPTOR> rank( sLatticeOne );
    vtmWriter.write( geometry );
    vtmWriter.write( cuboid );
    vtmWriter.write( rank );
    vtmWriter.createMasterFile();
  }

  // Get statistics
  if ( iT%statIter==0 && iT > 0 ) {
    // Timer console output
    timer.update( iT );
    timer.printStep();

    clout << "averageRhoFluidOne="   << sLatticeOne.getStatistics().getAverageRho();
    clout << "; averageRhoFluidTwo=" << sLatticeTwo.getStatistics().getAverageRho() << std::endl;
  }

  // Writes the VTK files
  if ( iT%vtkIter==0 ) {
    clout << "Writing VTK ..." << std::endl;
    SuperLatticeVelocity3D<T, DESCRIPTOR> velocity( sLatticeOne );
    SuperLatticeDensity3D<T, DESCRIPTOR> density( sLatticeOne );
    vtmWriter.addFunctor( velocity );
    vtmWriter.addFunctor( density );
    vtmWriter.write( iT );

    //BlockReduction3D2D<T> planeReduction( density, {0, 0, 1} );
    // write output as JPEG
    //heatmap::write(planeReduction, iT);

    clout << "Writing VTK ... OK" << std::endl;
  }
}


int main( int argc, char *argv[] ) {

  // === 1st Step: Initialization ===

  olbInit( &argc, &argv );
  singleton::directories().setOutputDir( "./tmp_KHI/" );
  OstreamManager clout( std::cout,"main" );

  const T omega1 = 1.0;
  const T omega2 = 1.0;
  const T G      = 3.;
  T force        = 7./( T )ny/( T )ny;

  // === 2nd Step: Prepare Geometry ===
  // Instantiation of a cuboidGeometry with weights

#ifdef PARALLEL_MODE_MPI
  CuboidGeometry3D<T> cGeometry( 0, 0, 0, 1, nx, ny, nz, singleton::mpi().getSize() );
#else
  CuboidGeometry3D<T> cGeometry( 0, 0, 0, 1, nx, ny, nz, 1 );
#endif

  cGeometry.setPeriodicity( true, false, true );

  HeuristicLoadBalancer<T> loadBalancer( cGeometry );

  SuperGeometry3D<T> superGeometry( cGeometry,loadBalancer,2 );

  prepareGeometry( superGeometry );

  // === 3rd Step: Prepare Lattice ===

  SuperLattice3D<T, DESCRIPTOR> sLatticeOne( superGeometry );
  SuperLattice3D<T, DESCRIPTOR> sLatticeTwo( superGeometry );

  ForcedBGKdynamics<T, DESCRIPTOR> bulkDynamics1 (
    omega1, instances::getExternalVelocityMomenta<T,DESCRIPTOR>() );
  ForcedBGKdynamics<T, DESCRIPTOR> bulkDynamics2 (
    omega2, instances::getExternalVelocityMomenta<T,DESCRIPTOR>() );

  // A bounce-back node with fictitious density 1,
  //   which is experienced by the partner fluid
  BounceBack<T, DESCRIPTOR> bounceBackRho1( 1. );
  // A bounce-back node with fictitious density 0,
  //   which is experienced by the partner fluid
  BounceBack<T, DESCRIPTOR> bounceBackRho0( 0. );

  std::vector<T> rho0;
  rho0.push_back( 1 );
  rho0.push_back( 1 );
  PsiEqualsRho<T,T> interactionPotential;
  ShanChenForcedGenerator3D<T,DESCRIPTOR> coupling( G,rho0,interactionPotential );

  sLatticeOne.addLatticeCoupling(coupling, sLatticeTwo );

  prepareLattice( sLatticeOne, sLatticeTwo, bulkDynamics1, bulkDynamics2,
                  bounceBackRho0, bounceBackRho1, superGeometry );

  // === 4th Step: Main Loop with Timer ===
  int iT = 0;
  clout << "starting simulation..." << endl;
  Timer<T> timer( maxIter, superGeometry.getStatistics().getNvoxel() );
  timer.start();

  for ( iT=0; iT<maxIter; ++iT ) {

    // === 5th Step: Definition of Initial and Boundary Conditions ===
    setBoundaryValues( sLatticeOne, sLatticeTwo, force, iT, superGeometry );

    // === 6th Step: Collide and Stream Execution ===
    sLatticeOne.collideAndStream();
    sLatticeTwo.collideAndStream();

    sLatticeOne.communicate();
    sLatticeTwo.communicate();

    sLatticeOne.executeCoupling();
    //sLatticeTwo.executeCoupling();

    // === 7th Step: Computation and Output of the Results ===
    getResults( sLatticeTwo, sLatticeOne, iT, superGeometry, timer );
  }

  timer.stop();
  timer.printSummary();
}
