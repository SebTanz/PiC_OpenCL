#include <iostream>
#include <cmath>

#include "./pic.h"


#define  CL_ENABLE_EXCEPTIONS
// #define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_TARGET_OPENCL_VERSION 200
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

/** Calculate the electric field from Gauss' law, given density n and dx. 
 Note that Ex values are taken at left cell boundaries, while n values are at cell centers (averages).*/


static void calcEx(DArr1& ex, const DArr1& rho, double dx)
{
    
    const int Nx = ex.size();
    ex(0) = 0.;
    double sumE = 0.;
    for(int i = 1; i < Nx; ++i){
        ex(i) = ex(i-1) + dx * rho(i-1);
        sumE += ex(i);
    }

    for(int i=0; i<Nx; ++i){
        ex(i) -= sumE / Nx;
    }
}

void PIC::moments()
{
    
    ret = clEnqueueNDRangeKernel(command_queue, setZero_kernel, 1, NULL, &global_item_x, &local_item_x, 0, NULL, NULL);
//     std::cout << "Error code: " << PIC::ret << std::endl;


    ret = clEnqueueNDRangeKernel(command_queue, moments_kernel, 1, NULL, &global_item_part, &local_item_part, 0, NULL, NULL);
//     PIC::ret = clFinish(PIC::command_queue);
//     std::cout << "Error code: " << PIC::ret << std::endl;
    ret = clEnqueueNDRangeKernel(command_queue, propMoments_kernel, 1, NULL, &global_item_x, &local_item_x, 0, NULL, NULL);
//     PIC::ret = clFinish(PIC::command_queue);
//     std::cout << "Error code: " << PIC::ret << std::endl;
    ret = clEnqueueReadBuffer(command_queue, rho_mem, CL_FALSE, 0, Nx * sizeof(double), rho.data(), 0, NULL, NULL);
    


    // compute proper moments

    calcEx(Ex, rho, dx);
    ret = clEnqueueWriteBuffer(command_queue, Ex_mem, CL_FALSE, 0, Nx * sizeof(double), Ex.data(), 0, NULL, NULL);
}

/** Leapfrog's 'drift' on particles */
static void drift(Array1<Particle>& parts, cl_double dt)
{
    
//    push particles
  PIC::ret = clSetKernelArg(PIC::drift_kernel, 2, sizeof(cl_double), &dt);
  
  PIC::ret = clEnqueueNDRangeKernel(PIC::command_queue, PIC::drift_kernel, 1, NULL, &PIC::global_item_part, &PIC::local_item_part, 0, NULL, NULL);
  PIC::ret = clFinish(PIC::command_queue);
//   std::cout << "Error code: " << PIC::ret << std::endl;
//   PIC::ret = clEnqueueReadBuffer(PIC::command_queue, PIC::part_mem, CL_FALSE, 0, parts.size() * sizeof(Particle), parts.data(), 0, NULL, NULL);
 

}

/** Leapfrog's 'kick' on particles */
static void kick(Array1<Particle>& parts, cl_double dt)
{
    PIC::moments();

//     PIC::ret = clEnqueueWriteBuffer(PIC::command_queue, PIC::Ex_mem, CL_FALSE, 0, PIC::Ex.size() * sizeof(double), PIC::Ex.data(), 0, NULL, NULL);
    PIC::ret = clFinish(PIC::command_queue);
    PIC::ret = clSetKernelArg(PIC::kick_kernel, 4, sizeof(cl_double), &dt);

    PIC::ret = clEnqueueNDRangeKernel(PIC::command_queue, PIC::kick_kernel, 1, NULL, &PIC::global_item_part, &PIC::local_item_part, 0, NULL, NULL);
//     std::cout << "Error code: " << PIC::ret << std::endl;
//     PIC::ret = clEnqueueReadBuffer(PIC::command_queue, PIC::part_mem, CL_FALSE, 0, parts.size() * sizeof(Particle), parts.data(), 0, NULL, NULL);


}

void PIC::doInterval(int i)
{
    std::cout << "Starting interval " << i << " up to t = " << i*interval << std::endl;

    double myDt = fmin(dt, interval); // time step for regular leapfrogging
    kick(particles, -0.5*myDt); // sync in: kick v backwards by dt/2
    double toDo = interval; // remaining time to integrate over
    while( toDo >= myDt ){ // do regular kick-drift sequence with myDt
        kick(particles, (cl_double)myDt);
        drift(particles, (cl_double)myDt);
        toDo -= myDt;
    }
    // Now, there's still a remainder time toDo >= 0 for x, 
    // and v still lags behind by myDt/2
    // sync out of Leapfrog: push v by myDt/2. 
    // Plus: Do a toDo/2 kick followed by toDo drift, followed by toDo/2 kick
    kick(particles, (cl_double)(0.5*myDt + 0.5*toDo));
    drift(particles, (cl_double)toDo); // last drift
    kick(particles, (cl_double)(0.5*toDo)); // last half-kick
    
    
}
