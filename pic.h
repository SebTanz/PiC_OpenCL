#ifndef PIC_PIC_H
#define PIC_PIC_H

#include "./array.h"

#define CL_TARGET_OPENCL_VERSION 200
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#define MAX_SOURCE_SIZE (0x100000)


struct Particle
{
  double x; // position
  double v; // velocity
  double m; // mass
  double q; // charge
};

// Abbreviate
typedef Array1<double> DArr1;

/** In this kind of implementation, PIC is basically a namespace for (almost-)global
    variables. */
struct PIC
{
  /** ++++ Simulation parameters */

  /** Number of computational particles. */
  static int nParticle;
  static size_t global_item_part;
  static size_t local_item_part;

  /** Integration occurs over several time intervals. This is the time span of each interval. */
  static double interval;

  /** This is the number of intervals. */
  static int nInterval; // # of time intervals in total

  /** The (maximum) time step for leapfrogging. */
  static double dt; // maximum time step to use

  /** System length */
  static double Lx;

  /* Number of grid cells in x-direction and cell width. */
  static int Nx;
  static size_t global_item_x;
  static size_t local_item_x;
  static double dx; // derived from above
  
  //GPU objects
  
  static cl_int ret;
  static cl_context context;
  static cl_command_queue command_queue;
  static cl_program program;
  static cl_kernel moments_kernel;
  static cl_kernel kick_kernel;
  static cl_kernel drift_kernel;
  static cl_kernel setZero_kernel;
  static cl_kernel propMoments_kernel;
  static cl_kernel calcEx_kernel;

  /** ++++ Arrays to hold grid quntities: */

  /** x-positions of cell centers. */
  static DArr1 x;
  static cl_mem x_mem;

  /** Accumulated  density in cells. */
  static DArr1 n; //
  static cl_mem n_mem;
  
  /** Accumulated  mass density in cells. */
  static DArr1 mu; //
  static cl_mem mu_mem;
  
    /** Accumulated charge density in cells. */
  static DArr1 rho; //
  static cl_mem rho_mem;

  /** Averaged velocity in cells. */
  static DArr1 u;
  static cl_mem u_mem;
  
  /** Averaged momentum in cells. */
  static DArr1 nu;
  static cl_mem nu_mem;

  /** Averaged temperature in cells. */
  static DArr1 T;
  static cl_mem T_mem;

  /** Electric field at left boundary of each cell. */
  static DArr1 Ex;
  static cl_mem Ex_mem;

 
  /** ++++ Array of particles. */
  static Array1<Particle> particles;
  static cl_mem part_mem;

  

  /** ++++ Functions */
  
  /* Initialize parameters and data. */
  static void init(int numPart, int numX, double lenX, int numInterv, double lenInterv, double Deltat);
  
  static void deviceInit(int platform, int device);

  /** Start and run simulation. */
  static void run();

  /** Clear allocated data. */
  static void finish();

  /** Compute particle moments on grid. */
  static void moments();

  /* Integrate for one time interval. */
  static void doInterval(int seq);

  /* Write particles to file. Each call will create a new particle and field data file.
     File names are based on outFileNameStem, files are numbered sequentially. */
  static void output();
  static const char* outFileNameStem;
};

#endif
