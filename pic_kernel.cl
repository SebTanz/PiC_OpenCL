// #pragma OPENCL EXTENSION cl_amd_printf : enable
// #pragma OPENCL EXTENSION cl_intel_printf : enable
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#ifdef cl_khr_fp64
    #pragma OPENCL EXTENSION cl_khr_fp64 : enable
#elif defined(cl_amd_fp64)
    #pragma OPENCL EXTENSION cl_amd_fp64 : enable
#else
    #error Double precision floating point not supported by OpenCL implementation.
#endif
__constant double klein = 1e-10;


struct Particle
{
  double x; // position
  double v; // velocity
  double m; // mass
  double q; // charge
};

void atom_add_double(__global double *val, double delta)
{
    union {
        double f;
        ulong  i;
    } old, new;

    do
    {
        old.f = *val;
        new.f = old.f + delta;
    }
    while (atom_cmpxchg((volatile __global ulong *)val, old.i, new.i) != old.i);
}

__kernel void moments_kernel(__global struct Particle* particles,
                             __global double* x,
                             __global double* n,
                             __global double* mu,
                             __global double* rho,
                             __global double* u,
                             __global double* nu,
                             __global double* T,
                             double Lx, int Nx)
{
    double dx = Lx / (double)Nx;
    int j = get_global_id(0);
    double xj = particles[j].x;
    int iCell = ((int)(round( xj/dx + 0.5 ) - 1)) % Nx; // cell index of particle center
    xj -= floor(xj/Lx)*Lx;


    
    // weight for neighbor cell: Distance of partcle center from cell center / dx
    double wNeighb = fabs( xj - x[iCell] ) / dx;
    // right or left neighbor cell with periodic conditions
    int iNeighb = ( xj > x[iCell] ) ? (iCell+1) % Nx : (iCell+Nx-1) % Nx;

    // add particle velocity moment in neighbor and own cell
    
    
    atom_add_double(n+iNeighb, wNeighb);
    atom_add_double(n+iCell, (1.-wNeighb));
    double m = particles[j].m;
    atom_add_double(mu+iNeighb, m * wNeighb);
    atom_add_double(mu+iCell, m * (1.-wNeighb));
    double q = particles[j].q;
    atom_add_double(rho+iNeighb, q * wNeighb);
    atom_add_double(rho+iCell, q * (1.-wNeighb));
    double v = particles[j].v;
    atom_add_double(u+iNeighb, wNeighb * v);
    atom_add_double(u+iCell, (1.-wNeighb) * v);
    atom_add_double(nu+iNeighb,  m * wNeighb * v);
    atom_add_double(nu+iCell, m * (1.-wNeighb) * v);
    atom_add_double(T+iNeighb, m * wNeighb * v*v);
    atom_add_double(T+iCell, m * (1.-wNeighb) * v*v);
}

__kernel void kick_kernel(__global struct Particle* particles,
                          __global double* Ex,
                          double Lx, int Nx, double dt) 
{
    double dx = Lx / (double)Nx;
    
    int j = get_global_id(0);
    double xj = particles[j].x;
    
    int iBnd = (int)round( xj/dx ); // index of left cell boundary closest to particle
    double dist = xj - iBnd*dx; // particle's distance to cell boundary
    
    // periodic conditions for index
    iBnd = ( iBnd + Nx ) % Nx; // % is modulo-operator in C
    
    // weight for neighbor boundary: Distance of particle center from cell bndry / dx
    double wNeighb = fabs( dist ) / dx;
    // right or left neighbor bndry with periodicity
    int iNeighb = ( dist > 0. ) ? (iBnd+1) % Nx : (iBnd+Nx-1) % Nx;
    
    // compute average electric field in particle cloud and advance v
    particles[j].v += particles[j].q * dt * ( (1.-wNeighb)*Ex[iBnd] + wNeighb*Ex[iNeighb])/particles[j].m;
    
}

__kernel void drift_kernel(__global struct Particle* particles,
                           double Lx, double dt) 
{
    
    int j = get_global_id(0);
    double newX = particles[j].x + dt*particles[j].v;
    // periodic conditions
    newX -= floor(newX/Lx)*Lx;
    particles[j].x = newX;
}

__kernel void setZero_kernel(__global double* n,
                             __global double* mu,
                             __global double* rho,
                             __global double* u,
                             __global double* nu,
                             __global double* T)
{
    int j = get_global_id(0);
    n[j] = 0.;
    mu[j] = 0.;
    rho[j] = 0.;
    u[j] = 0.;
    nu[j] = 0.;
    T[j] = 0.;
}

__kernel void propMoments_kernel(__global double* n,
                                 __global double* mu,
                                 __global double* rho,
                                 __global double* u,
                                 __global double* nu,
                                 __global double* T,
                                 int Np, int Nx, double Lx)
{
    int j = get_global_id(0);
    double factor = ((double)Nx)/(double)Np;
    double dx = Lx / (double)Nx;
    
    n[j] /= dx;
    mu[j] /= dx;
    rho[j] /= dx;
    u[j] /= dx;
    nu[j] /= dx;
    T[j] = T[j]/(dx * (n[j] + klein)) - nu[j]*nu[j]/(2. * (n[j]+klein) * (mu[j]+klein));
    /*
    n[j] *= factor;
    rho[j] *= factor;
    u[j] *= factor/(n[j] + klein);
    T[j] = factor*T[j]/(n[j] + klein) - u[j]*u[j];
    */
}

__kernel void calcEx_kernel(__global double* Ex,
                            __global double* rho,
                            double Lx, int Nx)
{
    double dx = Lx / (double)Nx;
    int j = get_global_id(0);
    if(j==0){
        Ex[j] = 0.;
    }
    else{
        Ex[j] = Ex[j-1] + dx * rho[j-1];
    }
}
