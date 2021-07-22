#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "./pic.h"

static FILE* openFile(const char* filename)
{
  FILE* fp = fopen(filename, "wb");
  if( nullptr == fp ){
    std::cout << "Can't open output file '" << filename << "'\n";
    exit(1);
  }
  return fp;
}

static int outSeq = 0;

void PIC::output()
{
    // write particle data: Create output file name from seq
    std::ostringstream oss;
    oss << outFileNameStem << "partData." << std::setw(4) << std::setfill('0') << outSeq;


    // restrict #particles to write out
    const int maxOut = 1000;
    const int nOut = nParticle > maxOut ? maxOut : nParticle;

    FILE* fp = openFile( oss.str().c_str() );
    fprintf(fp, "# np = %d time = %g\n*", nOut, outSeq*interval);
    for(int ip=0; ip<nOut; ++ip){
    int rp = nParticle * drand48();
        fwrite( & particles(rp).x, sizeof(double), 1, fp);
        fwrite( & particles(rp).v, sizeof(double), 1, fp);
        fwrite( & particles(rp).m, sizeof(double), 1, fp);
        fwrite( & particles(rp).q, sizeof(double), 1, fp);
    }
    fclose(fp);

    // write field data, compute moments first
    moments();
    ret = clEnqueueReadBuffer(command_queue, n_mem, CL_TRUE, 0, Nx * sizeof(double), n.data(), 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, mu_mem, CL_TRUE, 0, Nx * sizeof(double), mu.data(), 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, rho_mem, CL_TRUE, 0, Nx * sizeof(double), rho.data(), 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, u_mem, CL_TRUE, 0, Nx * sizeof(double), u.data(), 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, nu_mem, CL_TRUE, 0, Nx * sizeof(double), nu.data(), 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, T_mem, CL_TRUE, 0, Nx * sizeof(double), T.data(), 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, part_mem, CL_TRUE, 0, nParticle * sizeof(Particle), particles.data(), 0, NULL, NULL);

    std::ostringstream foss;
    foss << outFileNameStem << "fieldData." << std::setw(4) << std::setfill('0') << outSeq;

    fp = openFile( foss.str().c_str() );
    fprintf(fp, "# nx = %d time = %g\n*", Nx, outSeq*interval);

    // write x, n, rho, u, T, Ex to file
    fwrite( x.data(), sizeof(double), Nx, fp);
    fwrite( n.data(), sizeof(double), Nx, fp);
    fwrite( mu.data(), sizeof(double), Nx, fp);
    fwrite( rho.data(), sizeof(double), Nx, fp);
    fwrite( u.data(), sizeof(double), Nx, fp);
    fwrite( nu.data(), sizeof(double), Nx, fp);
    fwrite( T.data(), sizeof(double), Nx, fp);
    fwrite( Ex.data(), sizeof(double), Nx, fp);

    fclose(fp);

    outSeq++;
}
