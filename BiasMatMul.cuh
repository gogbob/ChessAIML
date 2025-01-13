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

using namespace std;

class BiasmatmulAdd
{
private:
    //hi
public:
    void* workspace;
    cublasLtMatmulDesc_t operationDesc = NULL;
    cublasLtMatrixLayout_t Adesc = NULL, Bdesc = NULL, Cdesc = NULL;
    cublasLtMatmulPreference_t preference = NULL;
    cublasLtMatmulHeuristicResult_t heuristicResult = {};
    cublasOperation_t transa = CUBLAS_OP_T;
    cublasOperation_t transb = CUBLAS_OP_N;
    size_t worksSize;

    float* placeHolder;

    cudaEvent_t start, stop;
    cublasLtHandle_t ltHandle;
    float a, b;
    void init(size_t workspaceSize, size_t neuronSize, size_t batchSize)
    {

        cudaEventCreate(&start);
        cudaEventCreate(&stop);

        worksSize = workspaceSize;
        a = 1.0f;
        b = 1.0f;
        cudaMalloc(&workspace, workspaceSize);

        cudaMalloc((void**)placeHolder, batchSize * neuronSize * sizeof(float));

        float* ones;

        cudaMallocHost((void**)&ones, neuronSize * batchSize * sizeof(float));

        for (int i = 0; i < neuronSize * batchSize; i++)
        {
            ones[i] = 1.0f;
        }

        cudaMemcpy(placeHolder, ones, neuronSize * batchSize * sizeof(float), cudaMemcpyHostToDevice);

        int returnedResults = 0;
        // create operation desciriptor; see cublasLtMatmulDescAttributes_t for details about defaults; here we just need to
        // set the transforms for A and B

        cublasLtCreate(&ltHandle);

        cublasLtMatmulDescCreate(&operationDesc, CUBLAS_COMPUTE_32F, CUDA_R_32F);
        cublasLtMatmulDescSetAttribute(operationDesc, CUBLASLT_MATMUL_DESC_TRANSA, &transa, sizeof(transa));
        cublasLtMatmulDescSetAttribute(operationDesc, CUBLASLT_MATMUL_DESC_TRANSB, &transb, sizeof(transb));

        // create matrix descriptors, we are good with the details here so no need to set any extra attributes
        cublasLtMatrixLayoutCreate(&Adesc, CUDA_R_32F, neuronSize, 1, neuronSize);
        cublasLtMatrixLayoutCreate(&Bdesc, CUDA_R_32F, neuronSize, batchSize, neuronSize);
        cublasLtMatrixLayoutCreate(&Cdesc, CUDA_R_32F, neuronSize, batchSize, neuronSize);

        cublasLtMatmulPreferenceCreate(&preference);

        cublasLtMatmulPreferenceSetAttribute(preference, CUBLASLT_MATMUL_PREF_MAX_WORKSPACE_BYTES, &workspaceSize, sizeof(workspaceSize));
        //cublasLtMatmulPreferenceSetAttribute(preference, CUBLASLT_MATMUL_PREF_MAX_WORKSPACE_BYTES, &workspaceSize, sizeof(workspaceSize));

        cublasLtMatmulAlgoGetHeuristic(ltHandle, operationDesc, Adesc, Bdesc, Cdesc, Cdesc, preference, 1, &heuristicResult, &returnedResults);
    }

    void run(float* biases, float* outNeurons)
    {

        //std::cout<<cublasLtGetVersion()<<std::endl;
        cudaEventRecord(start, 0);


        cublasLtMatmul(ltHandle,
            operationDesc,
            &a,
            placeHolder,
            Adesc,
            biases,
            Bdesc,
            &b,
            outNeurons,
            Cdesc,
            outNeurons,
            Cdesc,
            &heuristicResult.algo,
            workspace,
            worksSize,
            0);
    }
};