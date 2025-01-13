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

class ReduceLayoutab
{
public:

    void* workspace;

    cudnnTensorDescriptor_t x_desc;
    cudnnTensorDescriptor_t out_desc;

    cudnnHandle_t handle_;
    cudnnReduceTensorDescriptor_t reduce_desc;

    size_t workspaceSize;

    size_t sizebytes;

    float a, b;
    void init(size_t neuronSize, size_t batchSize, size_t workSize, float alpha, float beta)
    {
        a = 1.0f;
        b = 0.0f;

        cudnnCreate(&handle_);



        // create the tensor descriptor
        cudnnDataType_t dtype = CUDNN_DATA_FLOAT;
        cudnnTensorFormat_t format = CUDNN_TENSOR_NCHW;

        workspaceSize = workSize;

        cudaMalloc(&workspace, workspaceSize);

        cudnnCreateTensorDescriptor(&x_desc);
        cudnnSetTensor4dDescriptor(x_desc, format, dtype, batchSize, 1, 1, neuronSize);

        cudnnCreateTensorDescriptor(&out_desc);
        cudnnSetTensor4dDescriptor(out_desc, format, dtype, 1, 1, 1, neuronSize);

        // create activation function descriptor



        cudnnNanPropagation_t prop = CUDNN_NOT_PROPAGATE_NAN;
        cudnnCreateReduceTensorDescriptor(&reduce_desc);
        cudnnSetReduceTensorDescriptor(reduce_desc, CUDNN_REDUCE_TENSOR_ADD, dtype, CUDNN_NOT_PROPAGATE_NAN, CUDNN_REDUCE_TENSOR_NO_INDICES, CUDNN_32BIT_INDICES);
        cudnnGetReductionIndicesSize(handle_, reduce_desc, x_desc, out_desc, &sizebytes);
    }

    void run(float* biases, float* dx)
    {
        cudnnReduceTensor(
            handle_,
            reduce_desc,
            dx,
            sizebytes,
            workspace,
            workspaceSize,
            &a,
            x_desc,
            dx,
            &b,
            out_desc,
            biases
        );
    }
};
//hi