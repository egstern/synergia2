#ifndef DISTRIBUTED_FFT2D_H
#define DISTRIBUTED_FFT2D_H

#include <Kokkos_Core.hpp>

#ifdef Kokkos_ENABLE_CUDA

  #include "synergia/utils/distributed_fft2d_cuda.h"

#else

  #include "synergia/utils/distributed_fft2d_fftw.h"

#endif


#endif
