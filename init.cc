#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <iostream>

#include "./pic.h"

#define  CL_ENABLE_EXCEPTIONS
// #define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_TARGET_OPENCL_VERSION 200
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

static double zuf() { return 2.*drand48()-1.; }

void PIC::init(int numPart, int numX, double lenX, int numInterv, double lenInterv, double Deltat)
{
  // set parameters:
  nParticle = numPart;
  global_item_part = (size_t)nParticle;

  // iteration:
  interval = lenInterv;
  nInterval = numInterv;
  dt = Deltat;

  Lx = lenX;
  Nx = numX;
  global_item_x = (size_t)Nx;
  dx = Lx / (double)Nx;

  outFileNameStem = "data/";
  

  // initialize particles
  particles.allocate(nParticle);
  for(int ip=0; ip<nParticle; ++ip){
    Particle& p = particles(ip);
    p.x = Lx * ip / nParticle;
//     p.x = (ip < nParticle/2) ? 1. : 2.;
    
//     p.v = zuf();
    p.v = 0.02*sin(2.*M_PI* p.x / Lx);
//     p.m = Nx;
    
//     p.v =  zuf();
//     p.v = (ip < nParticle/2) ?  -1. : 1.;
    
    
//     p.m = 10000+100000 * cos(5 * 2.*M_PI* p.x / Lx)*cos(5 * 2.*M_PI* p.x / Lx);
    p.m = (ip < nParticle/2) ?  10000: 100000.;
//     p.m = (ip%2 == 0) ? 100000: 1000000.;
    
//     p.q = zuf();
//     p.q = (sin(5 * 2.*M_PI* p.x / Lx));
//     p.q = pow(-1., ip);
//     p.q = 0;
    p.q = (ip < nParticle/2) ? -1. : 1.;
//     p.q = (ip%2 == 0) ? -1.: 1.;
  }

  // allocate grid and fields
  x.allocate(Nx);
  n.allocate(Nx);
  mu.allocate(Nx);
  rho.allocate(Nx);
  u.allocate(Nx);
  nu.allocate(Nx);
  T.allocate(Nx);
  Ex.allocate(Nx);

  // set x-positions of cell centers
  for(int i = 0; i < Nx; ++i){
    x(i) = (i + 0.5)*dx;
  }
}

void PIC::deviceInit(int platform, int device){
    
    char* value;
    size_t valueSize;
    cl_uint platformCount;
    cl_platform_id* platforms;
    cl_uint deviceCount;
    cl_device_id* devices;

    ret = clGetPlatformIDs(0, NULL, &platformCount);
    platforms = new cl_platform_id[platformCount];
    ret = clGetPlatformIDs(platformCount, platforms, NULL);


    ret = clGetDeviceIDs(platforms[platform], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);
    devices = new cl_device_id[deviceCount];
    ret = clGetDeviceIDs(platforms[platform], CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL);
    
    
    cl_device_id device0 = devices[device];
    
    ret = clGetDeviceInfo(devices[device], CL_DEVICE_NAME, 0, NULL, &valueSize);
    value = new char[valueSize];
    ret = clGetDeviceInfo(devices[device], CL_DEVICE_NAME, valueSize, value, NULL);    

    delete[] value;
        
    
    delete[] devices;

    delete[] platforms;
    
    context = clCreateContext( NULL, 1, &device0, NULL, NULL, &ret);
    
    command_queue = clCreateCommandQueueWithProperties(context, device0, NULL, &ret);
    
    FILE *fp = fopen("pic_kernel.cl", "r");
    if (!fp) {
        exit(1);
    }
    char* source_str = new char[MAX_SOURCE_SIZE];
    size_t source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );
    
    program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
    
    ret = clBuildProgram(program, 1, &device0, NULL, NULL, NULL);
    size_t log_size;
    clGetProgramBuildInfo(program, device0, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

    // Allocate memory for the log
    char *log = new char[log_size];

    // Get the log
    clGetProgramBuildInfo(program, device0, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

    // Print the log
    std::cout << log << std::endl;
    delete[] log;
    if (ret == CL_BUILD_PROGRAM_FAILURE) {
    // Determine the size of the log
        
        exit(CL_BUILD_PROGRAM_FAILURE);
    }
    moments_kernel = clCreateKernel(program, "moments_kernel", &ret);
    kick_kernel = clCreateKernel(program, "kick_kernel", &ret);
    drift_kernel = clCreateKernel(program, "drift_kernel", &ret);
    
    setZero_kernel = clCreateKernel(program, "setZero_kernel", &ret);
    propMoments_kernel = clCreateKernel(program, "propMoments_kernel", &ret);
    calcEx_kernel = clCreateKernel(program, "calcEx_kernel", &ret);
    
    
    x_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, x.size() * sizeof(double), NULL, &ret);
    n_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, n.size() * sizeof(double), NULL, &ret);
    mu_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, mu.size() * sizeof(double), NULL, &ret);
    rho_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, rho.size() * sizeof(double), NULL, &ret);
    u_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, u.size() * sizeof(double), NULL, &ret);
    nu_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, nu.size() * sizeof(double), NULL, &ret);
    T_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, T.size() * sizeof(double), NULL, &ret);
    Ex_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, Ex.size() * sizeof(double), NULL, &ret);
    
    part_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, particles.size() * sizeof(Particle), NULL, &ret);
    
    
    ret = clEnqueueWriteBuffer(command_queue, x_mem, CL_TRUE, 0, x.size() * sizeof(double), x.data(), 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, n_mem, CL_TRUE, 0, n.size() * sizeof(double), n.data(), 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, mu_mem, CL_TRUE, 0, mu.size() * sizeof(double), mu.data(), 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, rho_mem, CL_TRUE, 0, rho.size() * sizeof(double), rho.data(), 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, u_mem, CL_TRUE, 0, u.size() * sizeof(double), u.data(), 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, nu_mem, CL_TRUE, 0, nu.size() * sizeof(double), nu.data(), 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, T_mem, CL_TRUE, 0, T.size() * sizeof(double), T.data(), 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, Ex_mem, CL_TRUE, 0, Ex.size() * sizeof(double), Ex.data(), 0, NULL, NULL);
    
    ret = clEnqueueWriteBuffer(command_queue, part_mem, CL_TRUE, 0, particles.size() * sizeof(Particle), particles.data(), 0, NULL, NULL);
    
    
    cl_double cl_Lx = Lx;
    cl_int cl_Nx = Nx;
    cl_int cl_Np = nParticle;
    
    ret = clFinish(command_queue);
    
    ret = clSetKernelArg(moments_kernel, 0, sizeof(cl_mem), (void *)&part_mem);
    ret = clSetKernelArg(moments_kernel, 1, sizeof(cl_mem), (void *)&x_mem);
    ret = clSetKernelArg(moments_kernel, 2, sizeof(cl_mem), (void *)&n_mem);
    ret = clSetKernelArg(moments_kernel, 3, sizeof(cl_mem), (void *)&mu_mem);
    ret = clSetKernelArg(moments_kernel, 4, sizeof(cl_mem), (void *)&rho_mem);
    ret = clSetKernelArg(moments_kernel, 5, sizeof(cl_mem), (void *)&u_mem);
    ret = clSetKernelArg(moments_kernel, 6, sizeof(cl_mem), (void *)&nu_mem);
    ret = clSetKernelArg(moments_kernel, 7, sizeof(cl_mem), (void *)&T_mem);
    ret = clSetKernelArg(moments_kernel, 8, sizeof(cl_double), &cl_Lx);
    ret = clSetKernelArg(moments_kernel, 9, sizeof(cl_int), &cl_Nx);
    
    ret = clSetKernelArg(kick_kernel, 0, sizeof(cl_mem), (void *)&part_mem);
    ret = clSetKernelArg(kick_kernel, 1, sizeof(cl_mem), (void *)&Ex_mem);
    ret = clSetKernelArg(kick_kernel, 2, sizeof(cl_double), &cl_Lx);
    ret = clSetKernelArg(kick_kernel, 3, sizeof(cl_int), &cl_Nx);
    
    ret = clSetKernelArg(drift_kernel, 0, sizeof(cl_mem), (void *)&part_mem);
    ret = clSetKernelArg(drift_kernel, 1, sizeof(cl_double), &cl_Lx);
    
    
    ret = clSetKernelArg(setZero_kernel, 0, sizeof(cl_mem), (void *)&n_mem);
    ret = clSetKernelArg(setZero_kernel, 1, sizeof(cl_mem), (void *)&mu_mem);
    ret = clSetKernelArg(setZero_kernel, 2, sizeof(cl_mem), (void *)&rho_mem);
    ret = clSetKernelArg(setZero_kernel, 3, sizeof(cl_mem), (void *)&u_mem);
    ret = clSetKernelArg(setZero_kernel, 4, sizeof(cl_mem), (void *)&nu_mem);
    ret = clSetKernelArg(setZero_kernel, 5, sizeof(cl_mem), (void *)&T_mem);
    
    ret = clSetKernelArg(propMoments_kernel, 0, sizeof(cl_mem), (void *)&n_mem);
    ret = clSetKernelArg(propMoments_kernel, 1, sizeof(cl_mem), (void *)&mu_mem);
    ret = clSetKernelArg(propMoments_kernel, 2, sizeof(cl_mem), (void *)&rho_mem);
    ret = clSetKernelArg(propMoments_kernel, 3, sizeof(cl_mem), (void *)&u_mem);
    ret = clSetKernelArg(propMoments_kernel, 4, sizeof(cl_mem), (void *)&nu_mem);
    ret = clSetKernelArg(propMoments_kernel, 5, sizeof(cl_mem), (void *)&T_mem);
    ret = clSetKernelArg(propMoments_kernel, 6, sizeof(cl_int), &cl_Np);
    ret = clSetKernelArg(propMoments_kernel, 7, sizeof(cl_int), &cl_Nx);
    ret = clSetKernelArg(propMoments_kernel, 8, sizeof(cl_double), &cl_Lx);
    
    ret = clSetKernelArg(calcEx_kernel, 0, sizeof(cl_mem), (void *)&Ex_mem);
    ret = clSetKernelArg(calcEx_kernel, 1, sizeof(cl_mem), (void *)&rho_mem);
    ret = clSetKernelArg(calcEx_kernel, 2, sizeof(cl_double), &cl_Lx);
    ret = clSetKernelArg(calcEx_kernel, 3, sizeof(cl_int), &cl_Nx);
    
    
    
    size_t maxKernWorkGrSize;
    ret = clGetKernelWorkGroupInfo(moments_kernel, device0, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &maxKernWorkGrSize, NULL);
    
    
    local_item_part = maxKernWorkGrSize;
//     std::cout << "mom wg size: " << maxKernWorkGrSize << std::endl;
    if (maxKernWorkGrSize<global_item_part){
        while(global_item_part%local_item_part!=0){
            local_item_part--;
        }
    }
    else{
        local_item_part=global_item_part;
    }
    
    ret = clGetKernelWorkGroupInfo(drift_kernel, device0, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &maxKernWorkGrSize, NULL);
//     std::cout << "drift wg size: " << maxKernWorkGrSize << std::endl;
    
    ret = clGetKernelWorkGroupInfo(propMoments_kernel, device0, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &maxKernWorkGrSize, NULL);
    local_item_x = maxKernWorkGrSize;
    if (maxKernWorkGrSize<global_item_x){
        while(global_item_x%local_item_x!=0){
            local_item_x--;
        }
    }
    else{
        local_item_x=global_item_x;
    }
//     std::cout << maxKernWorkGrSize << std::endl;
    
}



void PIC::finish()
{
      // clear grid and fields
    x.free();
    n.free();
    mu.free();
    rho.free();
    u.free();
    nu.free();
    T.free();
    Ex.free();

    particles.free();
    
//     Clear GPU objects
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(moments_kernel);
    ret = clReleaseKernel(kick_kernel);
    ret = clReleaseKernel(drift_kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(x_mem);
    ret = clReleaseMemObject(u_mem);
    ret = clReleaseMemObject(n_mem);
    ret = clReleaseMemObject(T_mem);
    ret = clReleaseMemObject(Ex_mem);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);  

  
}

