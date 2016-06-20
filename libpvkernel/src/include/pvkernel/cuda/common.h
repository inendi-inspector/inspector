/**
 * @file
 *
 * @copyright (C) Picviz Labs 2010-March 2015
 * @copyright (C) ESI Group INENDI April 2015-2015
 */

#ifndef PVCUDA_COMMON_H
#define PVCUDA_COMMON_H

// Ensure CUDA is set if using nvcc
#ifdef __CUDACC__
#ifndef CUDA
#define CUDA
#endif
#endif

#ifdef CUDA
#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#endif

#ifndef __CUDACC__
#include <functional>
#endif

#include <pvkernel/cuda/constexpr.h>

#define inendi_verify_cuda(e) __inendi_verify_cuda(e, __FILE__, __LINE__)
#define __inendi_verify_cuda(e, F, L)                                                              \
	if ((e) != cudaSuccess) {                                                                      \
		fprintf(stderr, "Cuda assert failed in %s:%d with %s.\n", F, L, cudaGetErrorString(e));    \
		abort();                                                                                   \
	}
#define inendi_verify_cuda_kernel() __verify_cuda_kernel(__FILE__, __LINE__)
#define __verify_cuda_kernel(F, L)                                                                 \
	do {                                                                                           \
		cudaError_t last_err = cudaGetLastError();                                                 \
		__inendi_verify_cuda(last_err, F, L);                                                      \
	} while (0);

namespace PVCuda
{

void init_gl_cuda();
int get_number_blocks();
size_t get_shared_mem_size();
size_t get_number_of_devices();
#ifndef __CUDACC__
void visit_usable_cuda_devices(std::function<void(int)> const& f);
#endif
}

#endif
