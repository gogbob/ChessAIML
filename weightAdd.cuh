#pragma once

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

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

class weightAddLayout
{

public:
    void* workspace;
    cublasLtMatmulDesc_t operationDesc = NULL;
    cublasLtMatrixLayout_t Adesc = NULL, Bdesc = NULL, Cdesc = NULL;
    cublasLtMatmulPreference_t preference = NULL;
    cublasLtMatmulHeuristicResult_t heuristicResult = {};
    cublasOperation_t transa = CUBLAS_OP_T;
    cublasOperation_t transb = CUBLAS_OP_N;
    size_t worksSize;

    cudaEvent_t start, stop;
    cublasLtHandle_t ltHandle;
    float a, b;
    void init(size_t workspaceSize, size_t outSize, size_t batchSize, size_t inSize, float learningRate)
    {

        cudaEventCreate(&start);
        cudaEventCreate(&stop);

        worksSize = workspaceSize;
        a = learningRate;
        b = 1.0f;
        cudaMalloc(&workspace, workspaceSize);


        int returnedResults = 0;


        // create operation desciriptor; see cublasLtMatmulDescAttributes_t for details about defaults; here we just need to
        // set the transforms for A and B

        cublasLtCreate(&ltHandle);

        cublasLtMatmulDescCreate(&operationDesc, CUBLAS_COMPUTE_32F, CUDA_R_32F);
        cublasLtMatmulDescSetAttribute(operationDesc, CUBLASLT_MATMUL_DESC_TRANSA, &transa, sizeof(transa));
        cublasLtMatmulDescSetAttribute(operationDesc, CUBLASLT_MATMUL_DESC_TRANSB, &transb, sizeof(transb));

        // create matrix descriptors, we are good with the details here so no need to set any extra attributes
        cublasLtMatrixLayoutCreate(&Adesc, CUDA_R_32F, inSize, batchSize, inSize);
        cublasLtMatrixLayoutCreate(&Bdesc, CUDA_R_32F, outSize, batchSize, outSize);
        cublasLtMatrixLayoutCreate(&Cdesc, CUDA_R_32F, outSize, inSize, outSize);

        cublasLtMatmulPreferenceCreate(&preference);

        cublasLtMatmulPreferenceSetAttribute(preference, CUBLASLT_MATMUL_PREF_MAX_WORKSPACE_BYTES, &workspaceSize, sizeof(workspaceSize));
        //cublasLtMatmulPreferenceSetAttribute(preference, CUBLASLT_MATMUL_PREF_MAX_WORKSPACE_BYTES, &workspaceSize, sizeof(workspaceSize));

        cublasLtMatmulAlgoGetHeuristic(ltHandle, operationDesc, Adesc, Bdesc, Cdesc, Cdesc, preference, 1, &heuristicResult, &returnedResults);
    }

    void run(float* Weights, float* inNeurons, float* outNeurons)
    {

        //std::cout<<cublasLtGetVersion()<<std::endl;
        cudaEventRecord(start, 0);


        cout << cublasLtMatmul(ltHandle,
            operationDesc,
            &a,
            inNeurons,
            Adesc,
            outNeurons,
            Bdesc,
            &b,
            Weights,
            Cdesc,
            Weights,
            Cdesc,
            &heuristicResult.algo,
            workspace,
            worksSize,
            0);
    }
};
//hi