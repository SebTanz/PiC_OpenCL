#include <iostream>
#include <cmath>
#include "./timer.h"

#include "./pic.h"

#define CL_TARGET_OPENCL_VERSION 200
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#define MAX_SOURCE_SIZE (0x100000)

// Definition of static (class-) variables
int PIC::nParticle;
size_t PIC::global_item_part;
size_t PIC::local_item_part;
double PIC::interval;
int PIC::nInterval;
double PIC::dt;

double PIC::Lx;
int PIC::Nx;
size_t PIC::global_item_x;
size_t PIC::local_item_x;
double PIC::dx;

cl_int PIC::ret;
cl_context PIC::context;
cl_command_queue PIC::command_queue;
cl_program PIC::program;
cl_kernel PIC::moments_kernel;
cl_kernel PIC::kick_kernel;
cl_kernel PIC::drift_kernel;
cl_kernel PIC::setZero_kernel;
cl_kernel PIC::propMoments_kernel;
cl_kernel PIC::calcEx_kernel;

DArr1 PIC::x;
cl_mem PIC::x_mem;
DArr1 PIC::n;
cl_mem PIC::n_mem;
DArr1 PIC::mu;
cl_mem PIC::mu_mem;
DArr1 PIC::rho;
cl_mem PIC::rho_mem;
DArr1 PIC::u;
cl_mem PIC::u_mem;
DArr1 PIC::nu;
cl_mem PIC::nu_mem;
DArr1 PIC::T;
cl_mem PIC::T_mem;
DArr1 PIC::Ex;
cl_mem PIC::Ex_mem;

Array1<Particle> PIC::particles;
cl_mem PIC::part_mem;

const char* PIC::outFileNameStem;
void deviceInit(int platform, int device);

/* Main entry function. */
int main(int argc, char** argv)
{
    Timer timer;
    
    
    PIC::init(100000, 9000, 3., 100, 1., .01);
    PIC::deviceInit(0,0);
    std::cout << "Hello, particles!" << std::endl;
    timer.reset();
    PIC::output(); // initial output
    PIC::run();
    double t = timer.elapsed();
    std::cout << "Zeit: " << timer.elapsed() << "s" << std::endl;
    std::cout << t/PIC::nParticle << ", " << t/PIC::Nx << ", " << t/PIC::nInterval << std::endl;
    PIC::finish();

    return 0;
}


void PIC::run()
{
  std::cout << "Time interval = " << interval
            << ", #intervals = " << nInterval
            << ", dt = " << dt
            << ", substeps: " << ceil(interval/dt) << std::endl;

  // iteration loop
  for(int iter = 1; iter <= nInterval; ++iter){
    doInterval(iter);
    output();
  }
}

