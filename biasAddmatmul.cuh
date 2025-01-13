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

class ReduceLayoutMatMul
{
public:

    void* workspace;

    cudnnTensorDescriptor_t x_desc;
    cudnnTensorDescriptor_t out_desc;
    cublasLtMatmulDesc_t operationDesc = NULL;
    cublasLtMatrixLayout_t Adesc = NULL, Bdesc = NULL, Cdesc = NULL;
    cublasLtMatmulPreference_t preference = NULL;
    cublasLtMatmulHeuristicResult_t heuristicResult = {};

    cudnnHandle_t handle_;
    cudnnReduceTensorDescriptor_t reduce_desc;

    size_t workspaceSize;

    size_t sizebytes;
    cudaEvent_t start, stop;
    cublasLtHandle_t ltHandle;

    cublasOperation_t transa = CUBLAS_OP_T;
    cublasOperation_t transb = CUBLAS_OP_N;

    float* d_ones;

    float a, b;
    void init(size_t neuronSize, size_t batchSize, size_t workSize)
    {
        cudaEventCreate(&start);
        cudaEventCreate(&stop);

        workspaceSize = workSize;
        a = 1.0f;
        b = 0.0f;
        cudaMalloc(&workspace, workspaceSize);

        float* place_holder = (float*)malloc(neuronSize * batchSize * sizeof(float));

        for (int i = 0; i < neuronSize * batchSize; i++)
        {
            place_holder[i] = 1.0f;
        }

        cudaMalloc((void**)&d_ones, neuronSize * batchSize * sizeof(float));
        cudaMemcpy(d_ones, place_holder, neuronSize * batchSize * sizeof(float), cudaMemcpyHostToDevice);



        int returnedResults = 0;


        // create operation desciriptor; see cublasLtMatmulDescAttributes_t for details about defaults; here we just need to
        // set the transforms for A and B

        cublasLtCreate(&ltHandle);

        cublasLtMatmulDescCreate(&operationDesc, CUBLAS_COMPUTE_32F, CUDA_R_32F);
        cublasLtMatmulDescSetAttribute(operationDesc, CUBLASLT_MATMUL_DESC_TRANSA, &transa, sizeof(transa));
        cublasLtMatmulDescSetAttribute(operationDesc, CUBLASLT_MATMUL_DESC_TRANSB, &transb, sizeof(transb));

        // create matrix descriptors, we are good with the details here so no need to set any extra attributes
        cublasLtMatrixLayoutCreate(&Adesc, CUDA_R_32F, neuronSize, batchSize, neuronSize);
        cublasLtMatrixLayoutCreate(&Bdesc, CUDA_R_32F, neuronSize, batchSize, neuronSize);
        cublasLtMatrixLayoutCreate(&Cdesc, CUDA_R_32F, neuronSize, 1, neuronSize);

        // create the tensor descriptor
        cudnnDataType_t dtype = CUDNN_DATA_FLOAT;
        cudnnTensorFormat_t format = CUDNN_TENSOR_NCHW;



        // create activation function descriptor



        
    }

    void run(float* biases, float* dx)
    {
        cudaEventRecord(start, 0);


        cublasLtMatmul(ltHandle,
            operationDesc,
            &a,
            d_ones,
            Adesc,
            dx,
            Bdesc,
            &b,
            biases,
            Cdesc,
            biases,
            Cdesc,
            &heuristicResult.algo,
            workspace,
            workspaceSize,
            0);
    }
};
//hi