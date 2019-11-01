/*
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

/*
    Parallel reduction kernels
*/

#ifndef _REDUCE_KERNEL_H_
#define _REDUCE_KERNEL_H_

#include "../common.h"
#include "reduction_kernel.h"
#include <stdio.h>
#include <cooperative_groups.h>

namespace cg = cooperative_groups;

// Utility class used to avoid linker errors with extern
// unsized shared memory arrays with templated type
template<class T>
struct SharedMemory
{
    __device__ inline operator       T *()
    {
        extern __shared__ int __smem[];
        return (T *)__smem;
    }

    __device__ inline operator const T *() const
    {
        extern __shared__ int __smem[];
        return (T *)__smem;
    }
};

// specialize for double to avoid unaligned memory
// access compile errors
template<>
struct SharedMemory<double>
{
    __device__ inline operator       double *()
    {
        extern __shared__ double __smem_d[];
        return (double *)__smem_d;
    }

    __device__ inline operator const double *() const
    {
        extern __shared__ double __smem_d[];
        return (double *)__smem_d;
    }
};

/*
    Parallel sum reduction using shared memory
    - takes log(n) steps for n input elements
    - uses n threads
    - only works for power-of-2 arrays
*/

/* This reduction interleaves which threads are active by using the modulo
   operator.  This operator is very expensive on GPUs, and the interleaved
   inactivity means that no whole warps are active, which is also very
   inefficient */
template <class T> __global__ void reduce0(int what, T *g_idata, T *g_odata, unsigned int n)
{
    // Handle to thread block group
    cg::thread_block cta = cg::this_thread_block();
    T *sdata = SharedMemory<T>();

    // load shared mem
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;

	switch (what) {
	case SUM:
		sdata[tid] = (i < n) ? g_idata[i] : 0;
		break;
	case SQUARED_SUM:
		sdata[tid] = (i < n) ? g_idata[i]*g_idata[i] : 0;
		break;
	case NORM:
		sdata[tid] = (i < n) ? g_idata[i]*g_idata[i] : 0;
		break;
	case AVERAGE:
		sdata[tid] = (i < n) ? g_idata[i] : 0;
		break;
	default:
		break;
	}

    cg::sync(cta);

    // do reduction in shared mem
    for (unsigned int s=1; s < blockDim.x; s *= 2)
    {
        // modulo arithmetic is slow!
        if ((tid % (2*s)) == 0)
        {
            sdata[tid] += sdata[tid + s];
        }

        cg::sync(cta);
    }

    // write result for this block to global mem
    if (tid == 0) g_odata[blockIdx.x] = sdata[0];
}

/* This version uses contiguous threads, but its interleaved
   addressing results in many shared memory bank conflicts.
*/
template <class T> __global__ void reduce1(int what, T *g_idata, T *g_odata, unsigned int n)
{
    // Handle to thread block group
    cg::thread_block cta = cg::this_thread_block();
    T *sdata = SharedMemory<T>();

    // load shared mem
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;

	switch (what) {
	case SUM:
		sdata[tid] = (i < n) ? g_idata[i] : 0;
		break;
	case SQUARED_SUM:
		sdata[tid] = (i < n) ? g_idata[i]*g_idata[i] : 0;
		break;
	case NORM:
		sdata[tid] = (i < n) ? g_idata[i]*g_idata[i] : 0;
		break;
	case AVERAGE:
		sdata[tid] = (i < n) ? g_idata[i] : 0;
		break;
	default:
		break;
	}

    cg::sync(cta);

    // do reduction in shared mem
    for (unsigned int s=1; s < blockDim.x; s *= 2)
    {
        int index = 2 * s * tid;

        if (index < blockDim.x)
        {
			switch (what) {
			case SUM:
				sdata[index] += sdata[index+s];
				break;
			case SQUARED_SUM:
				sdata[index] += sdata[index+s]*sdata[index+s];
				break;
			case NORM:
				sdata[index] += sdata[index+s]*sdata[index+s];
				break;
			case AVERAGE:
				sdata[index] += sdata[index+s];
				break;
			default:
				break;
			}
        }

        cg::sync(cta);
    }

    // write result for this block to global mem
	switch (what) {
	case SUM:
		if (tid==0) g_odata[blockIdx.x] = sdata[0];
		break;
	case SQUARED_SUM:
		if (tid==0) g_odata[blockIdx.x] = sdata[0];
		break;
	case NORM:
		if (tid==0) g_odata[blockIdx.x] = sqrtf(sdata[0]);
		break;
	case AVERAGE:
		if (tid==0) g_odata[blockIdx.x] = sdata[0]/n;
		break;
	default:
		break;
	}
}

/*
    This version uses sequential addressing -- no divergence or bank conflicts.
*/
template <class T> __global__ void reduce2(int what, T *g_idata, T *g_odata, unsigned int n)
{
    // Handle to thread block group
    cg::thread_block cta = cg::this_thread_block();
    T *sdata = SharedMemory<T>();

    // load shared mem
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;

	switch (what) {
	case SUM:
		sdata[tid] = (i < n) ? g_idata[i] : 0;
		break;
	case SQUARED_SUM:
		sdata[tid] = (i < n) ? g_idata[i]*g_idata[i] : 0;
		break;
	case NORM:
		sdata[tid] = (i < n) ? g_idata[i]*g_idata[i] : 0;
		break;
	case AVERAGE:
		sdata[tid] = (i < n) ? g_idata[i] : 0;
		break;
	default:
		break;
	}

    cg::sync(cta);

    // do reduction in shared mem
    for (unsigned int s=blockDim.x/2; s>0; s>>=1)
    {
        if (tid < s)
        {
			switch (what) {
			case SUM:
				sdata[tid] += sdata[tid+s];
				break;
			case SQUARED_SUM:
				sdata[tid] += sdata[tid+s]*sdata[tid+s];
				break;
			case NORM:
				sdata[tid] += sdata[tid+s]*sdata[tid+s];
				break;
			case AVERAGE:
				sdata[tid] += sdata[tid+s];
				break;
			default:
				break;
			}
        }

        cg::sync(cta);
    }

    // write result for this block to global mem
	switch (what) {
	case SUM:
		if (tid==0) g_odata[blockIdx.x] = sdata[0];
		break;
	case SQUARED_SUM:
		if (tid==0) g_odata[blockIdx.x] = sdata[0];
		break;
	case NORM:
		if (tid==0) g_odata[blockIdx.x] = sqrtf(sdata[0]);
		break;
	case AVERAGE:
		if (tid==0) g_odata[blockIdx.x] = sdata[0]/n;
		break;
	default:
		break;
	}
}

/*
    This version uses n/2 threads --
    it performs the first level of reduction when reading from global memory.
*/
template <class T> __global__ void reduce3(int what, T *g_idata, T *g_odata, unsigned int n)
{
    // Handle to thread block group
    cg::thread_block cta = cg::this_thread_block();
    T *sdata = SharedMemory<T>();

    // perform first level of reduction,
    // reading from global memory, writing to shared memory
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*(blockDim.x*2) + threadIdx.x;

	T mySum;
	switch (what) {
	case SUM:
		mySum = (i<n) ? g_idata[i] : 0;
		break;
	case SQUARED_SUM:
		mySum = (i<n) ? g_idata[i]*g_idata[i] : 0;
		break;
	case NORM:
		mySum = (i<n) ? g_idata[i]*g_idata[i] : 0;
		break;
	case AVERAGE:
		mySum = (i<n) ? g_idata[i] : 0;
		break;
	default:
		break;
	}

	if (i+blockDim.x<n) {
		switch (what) {
		case SUM:
			mySum += g_idata[i+blockDim.x];
			break;
		case SQUARED_SUM:
			mySum += g_idata[i+blockDim.x]*g_idata[i+blockDim.x];
			break;
		case NORM:
			mySum += g_idata[i+blockDim.x]*g_idata[i+blockDim.x];
			break;
		case AVERAGE:
			mySum += g_idata[i+blockDim.x];
			break;
		default:
			break;
		}
	}

    sdata[tid] = mySum;
    cg::sync(cta);

    // do reduction in shared mem
    for (unsigned int s=blockDim.x/2; s>0; s>>=1)
    {
        if (tid < s)
        {
			switch (what) {
			case SUM:
				sdata[tid] = mySum = mySum+sdata[tid+s];
				break;
			case SQUARED_SUM:
				sdata[tid] = mySum = mySum+sdata[tid+s]*sdata[tid+s];
				break;
			case NORM:
				sdata[tid] = mySum = mySum+sdata[tid+s]*sdata[tid+s];
				break;
			case AVERAGE:
				sdata[tid] = mySum = mySum+sdata[tid+s];
				break;
			default:
				break;
			}
        }

        cg::sync(cta);
    }

    // write result for this block to global mem
	switch (what) {
	case SUM:
		if (tid==0) g_odata[blockIdx.x] = mySum;
		break;
	case SQUARED_SUM:
		if (tid==0) g_odata[blockIdx.x] = mySum;
		break;
	case NORM:
		if (tid==0) g_odata[blockIdx.x] = sqrtf(mySum);
		break;
	case AVERAGE:
		if (tid==0) g_odata[blockIdx.x] = mySum/n;
		break;
	default:
		break;
	}
}

/*
    This version uses the warp shuffle operation if available to reduce 
    warp synchronization. When shuffle is not available the final warp's
    worth of work is unrolled to reduce looping overhead.

    See http://devblogs.nvidia.com/parallelforall/faster-parallel-reductions-kepler/
    for additional information about using shuffle to perform a reduction
    within a warp.

    Note, this kernel needs a minimum of 64*sizeof(T) bytes of shared memory.
    In other words if blockSize <= 32, allocate 64*sizeof(T) bytes.
    If blockSize > 32, allocate blockSize*sizeof(T) bytes.
*/
template <class T, unsigned int blockSize> __global__ void reduce4(int what, T *g_idata, T *g_odata, unsigned int n) {
    // Handle to thread block group
    cg::thread_block cta = cg::this_thread_block();
    T *sdata = SharedMemory<T>();

    // perform first level of reduction,
    // reading from global memory, writing to shared memory
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*(blockDim.x*2) + threadIdx.x;

	T mySum;
	switch (what) {
	case SUM:
		mySum = (i<n) ? g_idata[i] : 0;
		break;
	case SQUARED_SUM:
		mySum = (i<n) ? g_idata[i]*g_idata[i] : 0;
		break;
	case NORM:
		mySum = (i<n) ? g_idata[i]*g_idata[i] : 0;
		break;
	case AVERAGE:
		mySum = (i<n) ? g_idata[i] : 0;
		break;
	default:
		break;
	}
	

	if (i+blockSize<n) {
		switch (what) {
		case SUM:
			mySum += g_idata[i+blockSize];
			break;
		case SQUARED_SUM:
			mySum += g_idata[i+blockSize]*g_idata[i+blockSize];
			break;
		case NORM:
			mySum += g_idata[i+blockSize]*g_idata[i+blockSize];
			break;
		case AVERAGE:
			mySum += g_idata[i+blockSize];
			break;
		default:
			break;
		}
	}

    sdata[tid] = mySum;
    cg::sync(cta);

    // do reduction in shared mem
    for (unsigned int s=blockDim.x/2; s>32; s>>=1)
    {
        if (tid < s)
        {
			switch (what) {
			case SUM:
				sdata[tid] = mySum = mySum+sdata[tid+s];
				break;
			case SQUARED_SUM:
				sdata[tid] = mySum = mySum+sdata[tid+s]*sdata[tid+s];
				break;
			case NORM:
				sdata[tid] = mySum = mySum+sdata[tid+s]*sdata[tid+s];
				break;
			case AVERAGE:
				sdata[tid] = mySum = mySum+sdata[tid+s];
				break;
			default:
				break;
			}
        }

        cg::sync(cta);
    }

    cg::thread_block_tile<32> tile32 = cg::tiled_partition<32>(cta);

    if (cta.thread_rank() < 32)
    {
        // Fetch final intermediate sum from 2nd warp
		if (blockSize>=64) {
			switch (what) {
			case SUM:
				mySum += sdata[tid+32];
				break;
			case SQUARED_SUM:
				mySum += sdata[tid+32]*sdata[tid+32];
				break;
			case NORM:
				mySum += sdata[tid+32]*sdata[tid+32];
				break;
			case AVERAGE:
				mySum += sdata[tid+32];
				break;
			default:
				break;
			}
		}
        // Reduce final warp using shuffle
        for (int offset = tile32.size()/2; offset > 0; offset /= 2) 
        {
			switch (what) {
			case SUM:
				mySum += tile32.shfl_down(mySum, offset); 
				break;
			case SQUARED_SUM:
				mySum += tile32.shfl_down(mySum, offset)*tile32.shfl_down(mySum, offset);
				break;
			case NORM:
				mySum += tile32.shfl_down(mySum, offset)*tile32.shfl_down(mySum, offset);
				break;
			case AVERAGE:
				mySum += tile32.shfl_down(mySum, offset);
				break;
			default:
				break;
			}			
        }
    }

    // write result for this block to global mem
	switch (what) {
	case SUM:
		if (cta.thread_rank()==0) g_odata[blockIdx.x] = mySum;
		break;
	case SQUARED_SUM:
		if (cta.thread_rank()==0) g_odata[blockIdx.x] = mySum;
		break;
	case NORM:
		if (cta.thread_rank()==0) g_odata[blockIdx.x] = sqrtf(mySum);
		break;
	case AVERAGE:
		if (cta.thread_rank()==0) g_odata[blockIdx.x] = mySum/n;
		break;
	default:
		break;
	}
}

/*
    This version is completely unrolled, unless warp shuffle is available, then
    shuffle is used within a loop.  It uses a template parameter to achieve
    optimal code for any (power of 2) number of threads.  This requires a switch
    statement in the host code to handle all the different thread block sizes at
    compile time. When shuffle is available, it is used to reduce warp synchronization.

    Note, this kernel needs a minimum of 64*sizeof(T) bytes of shared memory.
    In other words if blockSize <= 32, allocate 64*sizeof(T) bytes.
    If blockSize > 32, allocate blockSize*sizeof(T) bytes.
*/
template <class T, unsigned int blockSize> __global__ void reduce5(int what, T *g_idata, T *g_odata, unsigned int n)
{
    // Handle to thread block group
    cg::thread_block cta = cg::this_thread_block();
    T *sdata = SharedMemory<T>();

    // perform first level of reduction,
    // reading from global memory, writing to shared memory
    unsigned int tid = threadIdx.x;
    unsigned int i = blockIdx.x*(blockSize*2) + threadIdx.x;

	T mySum;
	switch (what) {
	case SUM:
		mySum = (i<n) ? g_idata[i] : 0; 
		break;
	case SQUARED_SUM:
		mySum = (i<n) ? g_idata[i]*g_idata[i] : 0; break;
		break;
	case NORM:
		mySum = (i<n) ? g_idata[i]*g_idata[i] : 0; break;
	case AVERAGE:
		mySum = (i<n) ? g_idata[i] : 0; 
		break;
	default:
		break;
	}

	

	if (i+blockSize<n) {
		switch (what) {
		case SUM:
			mySum += g_idata[i+blockSize];
			break;
		case SQUARED_SUM:
			mySum += g_idata[i+blockSize]*g_idata[i+blockSize];
			break;
		case NORM:
			mySum += g_idata[i+blockSize]*g_idata[i+blockSize];
			break;
		case AVERAGE:
			mySum += g_idata[i+blockSize];
			break;
		default:
			break;
		}
	}
    sdata[tid] = mySum;
    cg::sync(cta);

    // do reduction in shared mem
    if ((blockSize >= 512) && (tid < 256))
    {
		switch (what) {
		case SUM:
			sdata[tid] = mySum = mySum+sdata[tid+256];
			break;
		case SQUARED_SUM:
			sdata[tid] = mySum = mySum+sdata[tid+256]*sdata[tid+256];
			break;
		case NORM:
			sdata[tid] = mySum = mySum+sdata[tid+256]*sdata[tid+256];
			break;
		case AVERAGE:
			sdata[tid] = mySum = mySum+sdata[tid+256];
			break;
		default:
			break;
		}		
    }

    cg::sync(cta);

    if ((blockSize >= 256) &&(tid < 128))
    {
		switch (what) {
		case SUM:
			sdata[tid] = mySum = mySum+sdata[tid+128];
			break;
		case SQUARED_SUM:
			sdata[tid] = mySum = mySum+sdata[tid+128]*sdata[tid+128];
			break;
		case NORM:
			sdata[tid] = mySum = mySum+sdata[tid+128]*sdata[tid+128];
			break;
		case AVERAGE:
			sdata[tid] = mySum = mySum+sdata[tid+128];
			break;
		default:
			break;
		}
	}

    cg::sync(cta);

    if ((blockSize >= 128) && (tid <  64))
    {
		switch (what) {
		case SUM:
			sdata[tid] = mySum = mySum+sdata[tid+64];
			break;
		case SQUARED_SUM:
			sdata[tid] = mySum = mySum+sdata[tid+64]*sdata[tid+64];
			break;
		case NORM:
			sdata[tid] = mySum = mySum+sdata[tid+64]*sdata[tid+64];
			break;
		case AVERAGE:
			sdata[tid] = mySum = mySum+sdata[tid+64];
			break;
		default:
			break;
		}
	}

    cg::sync(cta);

    cg::thread_block_tile<32> tile32 = cg::tiled_partition<32>(cta);

    if (cta.thread_rank() < 32)
    {
        // Fetch final intermediate sum from 2nd warp
		if (blockSize>=64) {
			switch (what) {
			case SUM:
				sdata[tid] = mySum = mySum+sdata[tid+32];
				break;
			case SQUARED_SUM:
				sdata[tid] = mySum = mySum+sdata[tid+32]*sdata[tid+32];
				break;
			case NORM:
				sdata[tid] = mySum = mySum+sdata[tid+32]*sdata[tid+32];
				break;
			case AVERAGE:
				sdata[tid] = mySum = mySum+sdata[tid+32];
				break;
			default:
				break;
			}
		}
        // Reduce final warp using shuffle
        for (int offset = tile32.size()/2; offset > 0; offset /= 2) 
        {
			switch (what) {
			case SUM:
				mySum += tile32.shfl_down(mySum, offset); 
				break;
			case SQUARED_SUM:
				mySum += tile32.shfl_down(mySum, offset)*tile32.shfl_down(mySum, offset);
				break;
			case NORM:
				mySum += tile32.shfl_down(mySum, offset)*tile32.shfl_down(mySum, offset);
				break;
			case AVERAGE:
				mySum += tile32.shfl_down(mySum, offset); 
				break;
			default:
				break;
			}			
        }
    }

    // write result for this block to global mem
	switch (what) {
	case SUM:
		if (cta.thread_rank()==0) g_odata[blockIdx.x] = mySum;
		break;
	case SQUARED_SUM:
		if (cta.thread_rank()==0) g_odata[blockIdx.x] = mySum;
		break;
	case NORM:
		if (cta.thread_rank()==0) g_odata[blockIdx.x] = sqrtf(mySum);
		break;
	case AVERAGE:
		if (cta.thread_rank()==0) g_odata[blockIdx.x] = mySum/n;
		break;
	default:
		break;
	}
}

/*
    This version adds multiple elements per thread sequentially.  This reduces the overall
    cost of the algorithm while keeping the work complexity O(n) and the step complexity O(log n).
    (Brent's Theorem optimization)

    Note, this kernel needs a minimum of 64*sizeof(T) bytes of shared memory.
    In other words if blockSize <= 32, allocate 64*sizeof(T) bytes.
    If blockSize > 32, allocate blockSize*sizeof(T) bytes.
*/
template <class T, unsigned int blockSize, bool nIsPow2> __global__ void reduce6(int what, T *g_idata, T *g_odata, unsigned int n) {
	// Handle to thread block group
	cg::thread_block cta = cg::this_thread_block();
	T *sdata = SharedMemory<T>();

	// perform first level of reduction,
	// reading from global memory, writing to shared memory
	unsigned int tid = threadIdx.x;
	unsigned int i = blockIdx.x*blockSize*2+threadIdx.x;
	unsigned int gridSize = blockSize*2*gridDim.x;

	T mySum = 0;

	// we reduce multiple elements per thread.  The number is determined by the
	// number of active thread blocks (via gridDim).  More blocks will result
	// in a larger gridSize and therefore fewer elements per thread
	while (i<n)
	{
		switch (what) {
		case SUM:
			mySum += g_idata[i];
			break;
		case SQUARED_SUM:
			mySum += g_idata[i]*g_idata[i];
			break;
		case NORM:
			mySum += g_idata[i]*g_idata[i];
			break;
		case AVERAGE:
			mySum += g_idata[i];
			break;
		default:
			break;
		}

		// ensure we don't read out of bounds -- this is optimized away for powerOf2 sized arrays
		if (nIsPow2||i+blockSize<n)
			switch (what) {
			case SUM:
				mySum += g_idata[i+blockSize];
				break;
			case SQUARED_SUM:
				mySum += g_idata[i+blockSize]*g_idata[i+blockSize];
				break;
			case NORM:
				mySum += g_idata[i+blockSize]*g_idata[i+blockSize];
				break;
			case AVERAGE:
				mySum += g_idata[i+blockSize];
				break;
			default:
				break;
			}

		i += gridSize;
	}

	// each thread puts its local sum into shared memory
	sdata[tid] = mySum;
	cg::sync(cta);


	// do reduction in shared mem
	if ((blockSize>=512)&&(tid<256)) {
		switch (what) {
		case SUM:
			sdata[tid] = mySum = mySum+sdata[tid+256];
			break;
		case SQUARED_SUM:
			sdata[tid] = mySum = mySum+sdata[tid+256]*sdata[tid+256];
			break;
		case NORM:
			sdata[tid] = mySum = mySum+sdata[tid+256]*sdata[tid+256];
			break;
		case AVERAGE:
			sdata[tid] = mySum = mySum+sdata[tid+256];
			break;
		default:
			break;
		}

	}

	cg::sync(cta);

	if ((blockSize>=256)&&(tid<128)) {
		switch (what) {
		case SUM:
			sdata[tid] = mySum = mySum+sdata[tid+128];
			break;
		case SQUARED_SUM:
			sdata[tid] = mySum = mySum+sdata[tid+128]*sdata[tid+128];
			break;
		case NORM:
			sdata[tid] = mySum = mySum+sdata[tid+128]*sdata[tid+128];
			break;
		case AVERAGE:
			sdata[tid] = mySum = mySum+sdata[tid+128];
			break;
		default:
			break;
		}
	}

	cg::sync(cta);

	if ((blockSize>=128)&&(tid<64)) {
		switch (what) {
		case SUM:
			sdata[tid] = mySum = mySum+sdata[tid+64];
			break;
		case SQUARED_SUM:
			sdata[tid] = mySum = mySum+sdata[tid+64]*sdata[tid+64];
			break;
		case NORM:
			sdata[tid] = mySum = mySum+sdata[tid+64]*sdata[tid+64];
			break;
		case AVERAGE:
			sdata[tid] = mySum = mySum+sdata[tid+64];
			break;
		default:
			break;
		}
	}

	cg::sync(cta);

	cg::thread_block_tile<32> tile32 = cg::tiled_partition<32>(cta);

	if (cta.thread_rank()<32) {
		// Fetch final intermediate sum from 2nd warp
		if (blockSize>=64) {
			switch (what) {
			case SUM:
				sdata[tid] = mySum = mySum+sdata[tid+32];
				break;
			case SQUARED_SUM:
				sdata[tid] = mySum = mySum+sdata[tid+32]*sdata[tid+32];
				break;
			case NORM:
				sdata[tid] = mySum = mySum+sdata[tid+32]*sdata[tid+32];
				break;
			case AVERAGE:
				sdata[tid] = mySum = mySum+sdata[tid+32];
				break;
			default:
				break;
			}
			// Reduce final warp using shuffle
			for (int offset = tile32.size()/2; offset>0; offset /= 2)
			{
				switch (what) {
				case SUM:
					mySum += tile32.shfl_down(mySum, offset);
					break;
				case SQUARED_SUM:
					mySum += tile32.shfl_down(mySum, offset)*tile32.shfl_down(mySum, offset);
					break;
				case NORM:
					mySum += tile32.shfl_down(mySum, offset)*tile32.shfl_down(mySum, offset);
					break;
				case AVERAGE:
					mySum += tile32.shfl_down(mySum, offset);
					break;
				default:
					break;
				}
			}
		}

		// write result for this block to global mem
		switch (what) {
		case SUM:
			if (cta.thread_rank()==0) g_odata[blockIdx.x] = mySum;
			break;
		case SQUARED_SUM:
			if (cta.thread_rank()==0) g_odata[blockIdx.x] = mySum;
			break;
		case NORM:
			if (cta.thread_rank()==0) g_odata[blockIdx.x] = sqrtf(mySum);
			break;
		case AVERAGE:
			if (cta.thread_rank()==0) g_odata[blockIdx.x] = mySum/n;
			break;
		default:
			break;
		}

	}
}

bool isPow2(unsigned int x) {
	return ((x&(x-1))==0);
}


////////////////////////////////////////////////////////////////////////////////
// Wrapper function for kernel launch
////////////////////////////////////////////////////////////////////////////////
template <class T> void reduce(int what, int size, int threads, int blocks, int whichKernel, T *d_idata, T *d_odata)
{
    dim3 dimBlock(threads, 1, 1);
    dim3 dimGrid(blocks, 1, 1);

    // when there is only one warp per block, we need to allocate two warps
    // worth of shared memory so that we don't index shared memory out of bounds
    int smemSize = (threads <= 32) ? 2 * threads * sizeof(T) : threads * sizeof(T);

    // choose which of the optimized versions of reduction to launch
    switch (whichKernel)
    {
        case 0:
            reduce0<T><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
            break;

        case 1:
            reduce1<T><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
            break;

        case 2:
            reduce2<T><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
            break;

        case 3:
            reduce3<T><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
            break;

        case 4:
            switch (threads)
            {
                case 512:
                    reduce4<T, 512><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case 256:
                    reduce4<T, 256><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case 128:
                    reduce4<T, 128><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case 64:
                    reduce4<T,  64><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case 32:
                    reduce4<T,  32><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case 16:
                    reduce4<T,  16><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case  8:
                    reduce4<T,   8><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case  4:
                    reduce4<T,   4><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case  2:
                    reduce4<T,   2><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case  1:
                    reduce4<T,   1><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;
            }

            break;

        case 5:
            switch (threads)
            {
                case 512:
                    reduce5<T, 512><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case 256:
                    reduce5<T, 256><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case 128:
                    reduce5<T, 128><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case 64:
                    reduce5<T,  64><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case 32:
                    reduce5<T,  32><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case 16:
                    reduce5<T,  16><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case  8:
                    reduce5<T,   8><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case  4:
                    reduce5<T,   4><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case  2:
                    reduce5<T,   2><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;

                case  1:
                    reduce5<T,   1><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                    break;
            }

            break;

        case 6:
        default:
            if (isPow2(size))
            {
                switch (threads)
                {
                    case 512:
                        reduce6<T, 512, true><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case 256:
                        reduce6<T, 256, true><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case 128:
                        reduce6<T, 128, true><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case 64:
                        reduce6<T,  64, true><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case 32:
                        reduce6<T,  32, true><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case 16:
                        reduce6<T,  16, true><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case  8:
                        reduce6<T,   8, true><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case  4:
                        reduce6<T,   4, true><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case  2:
                        reduce6<T,   2, true><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case  1:
                        reduce6<T,   1, true><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;
                }
            }
            else
            {
                switch (threads)
                {
                    case 512:
                        reduce6<T, 512, false><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case 256:
                        reduce6<T, 256, false><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case 128:
                        reduce6<T, 128, false><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case 64:
                        reduce6<T,  64, false><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case 32:
                        reduce6<T,  32, false><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case 16:
                        reduce6<T,  16, false><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case  8:
                        reduce6<T,   8, false><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case  4:
                        reduce6<T,   4, false><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case  2:
                        reduce6<T,   2, false><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;

                    case  1:
                        reduce6<T,   1, false><<< dimGrid, dimBlock, smemSize >>>(what, d_idata, d_odata, size);
                        break;
                }
            }

            break;
    }
}
// Instantiate the reduction function for 3 types
template void reduce<int>(int what, int size, int threads, int blocks, int whichKernel, int *d_idata, int *d_odata);
template void reduce<float>(int what, int size, int threads, int blocks, int whichKernel, float *d_idata, float *d_odata);
EXPORT void reduce(int what, int size, int threads, int blocks, int whichKernel, float *d_idata, float *d_odata) {
	reduce<float>(what, size, threads, blocks, whichKernel, d_idata, d_odata);
}
template void reduce<double>(int what, int size, int threads, int blocks, int whichKernel, double *d_idata, double *d_odata);

#endif // #ifndef _REDUCE_KERNEL_H_
