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

class SigmoidLayout
{
private:

public:

    cudnnTensorDescriptor_t x_desc;

    cudnnHandle_t handle_;
    cudnnActivationDescriptor_t sigmoid_activation;
    float a, b;
    void init(size_t neuronSize, size_t batchSize)
    {
        a = 1.0f;
        b = 0.0f;

        cudnnCreate(&handle_);

        // create the tensor descriptor
        cudnnDataType_t dtype = CUDNN_DATA_FLOAT;
        cudnnTensorFormat_t format = CUDNN_TENSOR_NCHW;
        int n = batchSize, c = 1, h = 1, w = neuronSize;
        cudnnCreateTensorDescriptor(&x_desc);
        cudnnSetTensor4dDescriptor(x_desc, format, dtype, n, c, h, w);


        // create activation function descriptor

        cudnnActivationMode_t mode = CUDNN_ACTIVATION_SIGMOID;
        cudnnNanPropagation_t prop = CUDNN_NOT_PROPAGATE_NAN;
        cudnnCreateActivationDescriptor(&sigmoid_activation);
        cudnnSetActivationDescriptor(sigmoid_activation, mode, prop, 0.0f);
    }

    void run(float* InOutNeurons)
    {
        cudnnActivationForward(
            handle_,
            sigmoid_activation,
            &a,
            x_desc,
            InOutNeurons,
            &b,
            x_desc,
            InOutNeurons
        );
    }
};
//hi