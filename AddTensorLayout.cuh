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

class AddLayout
{
public:

    cudnnTensorDescriptor_t x_desc;

    cudnnHandle_t handle_;
    cudnnActivationDescriptor_t add_desc;
    float a, b;
    void init(size_t neuronSize)
    {
        a = 1.0f;
        b = 1.0f;

        cudnnCreate(&handle_);

        // create the tensor descriptor
        cudnnDataType_t dtype = CUDNN_DATA_FLOAT;
        cudnnTensorFormat_t format = CUDNN_TENSOR_NCHW;
        int n = 1, c = 1, h = 1, w = neuronSize;
        cudnnCreateTensorDescriptor(&x_desc);
        cudnnSetTensor4dDescriptor(x_desc, format, dtype, n, c, h, w);
    }

    void run(float* InNeurons, float* OutNeurons)
    {
        cudnnAddTensor(
            handle_,
            &a,
            x_desc,
            InNeurons,
            &b,
            x_desc,
            OutNeurons
        );
        //hi
    }
};