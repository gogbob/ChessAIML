#ifndef __CUDACC__
#define __CUDACC__
#endif // !__CUDACC__
#ifndef __CUDACC_EXTENDED_LAMBDA__
#define __CUDACC_EXTENDED_LAMBDA__
#endif // !__CUDACC__
#ifndef __CUDACC_RDC__
#define __CUDACC_RDC__
#endif // !__CUDACC__
#ifndef __NVCC_DIAG_PRAGMA_SUPPORT__
#define __NVCC_DIAG_PRAGMA_SUPPORT__
#endif // !__CUDACC__

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "MatMulFWDLayout.cuh"
#include "MatMulBWDLayout.cuh"
#include "FWDSigmoidLayout.cuh"
#include "BWDSigmoidLayout.cuh"

#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <cuda/std/cmath>
#include <cuda.h>
#include <cudnn.h>
#include <cublas_v2.h>
#include <cublasLt.h>
#include <curand.h>
#include <curand_kernel.h>
#include <chrono>
#include <time.h>

#define e 2.71828
//hi
using namespace std;

void verify_solution(float* a, float* b, float* c, int M, int N, int K);

void verifySigmoidderivative_function(float* a, float* da, float* otherAnswers, size_t size);

void FPBPTensorTrain(const int layerSizes, const int inputSize, const int outputSize)
{
    int numGPUs;
    cudaGetDeviceCount(&numGPUs);
    cout << "Found " << numGPUs << " GPUs." << endl;
    cudaSetDevice(0); // use GPU0
    int device;
    struct cudaDeviceProp devProp;
    cudaGetDevice(&device);
    cudaGetDeviceProperties(&devProp, device);
    cout << "Compute capability:" << devProp.major << "." << devProp.minor << endl;

    SigmoidLayoutBWD sigLayoutBack;

    sigLayoutBack.init(10, 100);

    curandGenerator_t generator;


    curandCreateGenerator(&generator, CURAND_RNG_PSEUDO_DEFAULT);

    curandSetPseudoRandomGeneratorSeed(generator, time(NULL));

    float* d_dx, * d_x;
    float* dx = (float*)malloc(10 * 100 * sizeof(float));
        float* x = (float*)malloc(10 * 100 * sizeof(float));
        float* dxprev = (float*)malloc(10 * 100 * sizeof(float));

    cudaMalloc((void**) & d_dx, 10 * 100 * sizeof(float));
    cudaMalloc((void**) & d_x, 10 * 100 * sizeof(float));

    curandGenerateUniform(generator, d_dx, 10 * 100);
    curandGenerateUniform(generator, d_x, 10 * 100);

    cudaMemcpy(dxprev, d_dx, 10 * 100 * sizeof(float), cudaMemcpyDeviceToHost);
    cudaMemcpy(x, d_x, 10 * 100 * sizeof(float), cudaMemcpyDeviceToHost);

    for (int i = 0; i < 10 * 100; i++)
    {
        dxprev[i] += (float)i / 100.0f;
    }

    cudaMemcpy(d_dx, dxprev, 10 * 100 * sizeof(float), cudaMemcpyHostToDevice);

    sigLayoutBack.run(d_dx, d_x);

    cudaMemcpy(dx, d_dx, 10 * 100 * sizeof(float), cudaMemcpyDeviceToHost);


    verifySigmoidderivative_function(x, dxprev, dx, 10 * 100);

}



void verify_solution(float* a, float* b, float* c, int M, int N, int K)
{
    cout << "\n verifying solution:\n";
    float temp;
    float* error = (float*)malloc(M * N * sizeof(float));

    float* newC = (float*)malloc(M * N * sizeof(float));

    for (int i = 0; i < N * M; i++)
    {
        newC[i] = 0;
        error[i] = 0;
    }

    float epsilon = 0.001;
    for (int m = 0; m < M; ++m)
        for (int n = 0; n < N; ++n)
            for (int k = 0; k < K; ++k)
            {
                newC[n * M + m] += a[m * K + k] * b[n * K + k];
            }
    
    for (int i = 0; i < N * M; i++)
    {
        //cout << newC[i] << endl;
        //cout << c[i] << endl;
        
    }

    cout << "\nprinting errors\n";

    for (int i = 0; i < N * M; i++)
    {
        cout << newC[i] - c[i] << endl;
    }
}

void verifySigmoidderivative_function(float* a, float* da, float* otherAnswers, size_t size)
{
    for (int i = 0; i < size; i++)
    {
        //cout << "prev = " << otherAnswers[i] << endl;
 
        //cout << "next = " << (a[i] * (1 - a[i])) * da[i] << endl;
        //cout << "dx = " << da[i] << endl;
        //cout << "x = " << a[i] << endl;
       // cout << "error = " << fabs(((a[i] * (1 - a[i])) * da[i]) - otherAnswers[i]) << endl;
    }
}

