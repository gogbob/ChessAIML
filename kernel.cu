
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

#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <curand_kernel.h>
#include <iostream>
#include <cuda/std/cmath>
#include <cuda.h>
#include <fstream>
#include <string>
#include <Windows.h>
#include <cstring>
#include <SFML/Graphics.hpp>
#include "ChessPositionUtil.h"
#include "MatMulFWDLayout.cuh"
#include "MatMulBWDLayout.cuh"
#include "FWDSigmoidLayout.cuh"
#include "BWDSigmoidLayout.cuh"
#include "reduceLayoutalphabeta.cuh"
#include "BiasMatMul.cuh"
#include "biasAdd.cuh"
#include "biasAddmatmul.cuh"
#include "AddTensorLayout.cuh"
#include "addtab.cuh"
#include "weightAdd.cuh"
#include <stdio.h>  /* defines FILENAME_MAX */
#include <string>
#include <windows.h>
#include <vector>
#include <chrono>
#include <cudnn.h>
#include <cublas_v2.h>
#include <cublasLt.h>
#include <curand.h>



#define inputSize 270
#define Hlayersizes 64
#define HlayerNumber 4
#define outputSize 1
#define batchSize 100



#define MAX_THREAD_COUNT 1024
#define learningRate 0.0007
#define e 2.71828


using namespace std;
using namespace std::chrono;

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char* file, int line, bool abort = true)
{
    if (code != cudaSuccess)
    {
        fprintf(stderr, "GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
        if (abort) exit(code);
    }
}

__global__ void FullNetworkFPBP(float* inputs, float* Biases, float* Weights, float* outputs, float* NetworkOutputCost)
{
    __shared__ float weightedValues[Hlayersizes * Hlayersizes];
    __shared__ float valuesAtNeurons[Hlayersizes * HlayerNumber];
    __syncthreads();

    //inputInd = the index of the specific neuron the previous layer
    //localInd = the index of the specific neuron on the next layer
    __shared__ float sums[Hlayersizes];

    if (threadIdx.y == 0)
    {
        for (int i = 0; i < inputSize; i++)
        {
            int hLayerNeuron = threadIdx.x;

            int inputNeuron = i;

            sums[hLayerNeuron] += inputs[inputNeuron] * Weights[inputNeuron + hLayerNeuron * inputSize];
        }
    }
    
        

    __syncthreads();


    if (threadIdx.y == 0)
    {
        //printf("sum of the #%d neuron = %f\n", threadIdx.x, sums[threadIdx.x]);

        //adding biases
        sums[threadIdx.x] += Biases[threadIdx.x];

        valuesAtNeurons[threadIdx.x] = sums[threadIdx.x];

        //activation function (sigmoid)
        //e = 2.71828f
        sums[threadIdx.x] = 1.0f / (1.0f + cuda::std::powf(2.71828f, sums[threadIdx.x]));
    }

    //initialize all the values that will be the sums of the weights for all inputs to the next neurons


    //going through all layers
    for (int i = 0; i < HlayerNumber - 1; i++)
    {
        __syncthreads();

        for (int j = 0; j < (Hlayersizes * Hlayersizes) / 1024; j++)
        {
            weightedValues[((threadIdx.y + (j * (1024 / Hlayersizes)))) * Hlayersizes + threadIdx.x] = sums[threadIdx.x] * Weights[(threadIdx.y + (j * (1024 / Hlayersizes))) * Hlayersizes + threadIdx.x + Hlayersizes * inputSize + i * Hlayersizes * Hlayersizes];
        }
        __syncthreads();


        if (threadIdx.y == 0)
        {
            //initialize all the values that will be the sums of the weights for all inputs to the next neurons
            sums[threadIdx.x] = 0;
            //sum up
            for (int j = threadIdx.x * Hlayersizes; j < threadIdx.x * Hlayersizes + Hlayersizes; j++)
            {
                sums[threadIdx.x] += weightedValues[j];
            }

            //adding biases
            sums[threadIdx.x] += Biases[threadIdx.x + (i + 1) * Hlayersizes];

            //printf("before activation on the #%d layer, the sum of the #%d neuron = %f\n", i + 1, outputNeuronIndex_0to31 + 1, sums[threadIdx.x]);

            //activation function (sigmoid)
            //e = 2.71828f

            valuesAtNeurons[threadIdx.x + (i + 1) * Hlayersizes] = sums[threadIdx.x];


            sums[threadIdx.x] = 1.0f / (1.0f + cuda::std::powf(2.71828f, sums[threadIdx.x]));


            //printf("on the #%d layer, the sum of the #%d neuron = %f\n", i + 1, outputNeuronIndex_0to31 + 1, sums[outputNeuronIndex_0to31]);
        }
    }

    //initialise variable that will hold the derivative of the sum of the squared residuals
    __shared__ float deriv_SSR[outputSize];

    __shared__ float deriv_next_Neurons[Hlayersizes]; //more like the previous neuron(next on in BACKpropagation)

    __shared__ float deriv_previous_Neurons[Hlayersizes]; //more like the one of the next neuron (previous in BACKpropagation)

    __syncthreads();
    if (threadIdx.y < outputSize)
    {
        weightedValues[threadIdx.y * Hlayersizes + threadIdx.x] = Weights[threadIdx.y * Hlayersizes + threadIdx.x + Hlayersizes * Hlayersizes * (HlayerNumber - 1) + Hlayersizes * inputSize] * sums[threadIdx.x];
        __syncthreads();

        if (threadIdx.x == 0)
        {
            //initialize all the values that will be the sums of the weights for all inputs to the next neurons
            sums[threadIdx.y] = 0;
            //sum up
            for (int i = threadIdx.y * Hlayersizes; i < threadIdx.y * Hlayersizes + Hlayersizes; i++)
            {
                sums[threadIdx.y] += weightedValues[i];
            }

            //adding biases
            sums[threadIdx.y] += Biases[threadIdx.y + HlayerNumber * Hlayersizes];

            //skip part of calculating sum of the squared residuals

            deriv_SSR[threadIdx.y] = 2 * (outputs[threadIdx.y] - sums[threadIdx.y]);


            if (threadIdx.y == 0)
                for (int i = 0; i < outputSize; i++) NetworkOutputCost[i] = (outputs[i] - sums[i]) * (outputs[i] - sums[i]);

            printf("because (%f - %f)^2 = %f ", outputs[0], sums[0], NetworkOutputCost[0]);

            Biases[threadIdx.y + HlayerNumber * Hlayersizes] += (deriv_SSR[threadIdx.y]) * learningRate;

        }

        __syncthreads();

        if (threadIdx.y == 0)
        {
            deriv_next_Neurons[threadIdx.x] = 0;

            for (int i = 0; i < outputSize; i++)
            {
                deriv_next_Neurons[threadIdx.x] += Weights[Hlayersizes * inputSize + (HlayerNumber - 1) * Hlayersizes * Hlayersizes + i * Hlayersizes + threadIdx.x] * deriv_SSR[i];
            }
            //activation function

            deriv_next_Neurons[threadIdx.x] *= cuda::std::powf(2.71828f, valuesAtNeurons[(HlayerNumber - 1) * Hlayersizes + threadIdx.x]) /
                cuda::std::powf((1.0f + cuda::std::powf(2.71828f, valuesAtNeurons[(HlayerNumber - 1) * Hlayersizes + threadIdx.x])), 2);
        }
        Weights[Hlayersizes * inputSize + (HlayerNumber - 1) * Hlayersizes * Hlayersizes + threadIdx.x + threadIdx.y * Hlayersizes] += (deriv_SSR[threadIdx.y]
            / (1.0f + cuda::std::powf(2.71828f, valuesAtNeurons[(HlayerNumber - 1) * Hlayersizes + threadIdx.x]))) * learningRate;
    }

    __syncthreads();

    for (int i = 0; i < HlayerNumber - 1; i++)
    {
        if (threadIdx.y == 0)
        {
            deriv_previous_Neurons[threadIdx.x] = deriv_next_Neurons[threadIdx.x];
            deriv_next_Neurons[threadIdx.x] = 0;

            __syncthreads();
            for (int j = 0; j < Hlayersizes; j++)
            {
                deriv_next_Neurons[threadIdx.x] += Weights[Hlayersizes * inputSize + (HlayerNumber - 2 - i) * Hlayersizes * Hlayersizes + j * Hlayersizes + threadIdx.x] * deriv_previous_Neurons[j];
            }
            //derivative of the activation function
            deriv_next_Neurons[threadIdx.x] *= cuda::std::powf(2.71828f, valuesAtNeurons[(HlayerNumber - 2 - i) * Hlayersizes + threadIdx.x])
                / cuda::std::powf((1.0f + cuda::std::powf(2.71828f, valuesAtNeurons[(HlayerNumber - 2 - i) * Hlayersizes + threadIdx.x])), 2);


            Biases[(HlayerNumber - 1 - i) * Hlayersizes + threadIdx.x] += (deriv_previous_Neurons[threadIdx.x]) * learningRate;
        }

        __syncthreads();

        for (int j = 0; j < (Hlayersizes * Hlayersizes) / 1024; j++)
        {
            Weights[Hlayersizes * inputSize + (HlayerNumber - 2 - i) * Hlayersizes * Hlayersizes + threadIdx.x + (threadIdx.y + j * (1024 / Hlayersizes)) * Hlayersizes] += ((deriv_previous_Neurons[threadIdx.y + j * (1024 / Hlayersizes)] 
                / (1.0f + cuda::std::powf(2.71828f, valuesAtNeurons[(HlayerNumber - 2 - i) * Hlayersizes + threadIdx.x])))) * learningRate;
        }
    }


    if (threadIdx.y == 0)
    {
        Biases[threadIdx.x] += deriv_next_Neurons[threadIdx.x] * learningRate;
    }


    for (int BlockIdx = 0; BlockIdx < cuda::std::ceil((float)inputSize / (float)Hlayersizes); BlockIdx++)
    {


        __syncthreads();

        for (int i = 0; i < (Hlayersizes * Hlayersizes) / 1024; i++)
        {
            int hLayerNeuron = threadIdx.y + i * (1024 / Hlayersizes);

            int inputNeuron = threadIdx.x + BlockIdx * Hlayersizes;


            if (inputNeuron < inputSize)
            {
                Weights[hLayerNeuron * inputSize + inputNeuron] += deriv_next_Neurons[hLayerNeuron] * inputs[inputNeuron] * learningRate;
            }
            __syncthreads();
        }
    }
}


__global__ void MovesbothSides(int** PieceGroups, int* prevMove)
{
    int numgames = blockDim.x * 1024;
    unsigned int startingmoves[8][8]{0};
    unsigned int bitmapBlackPieces = 0;
    unsigned int bitmapWhitePieces = 0;
    int* pieces = PieceGroups[blockIdx.x * 1024 + threadIdx.x];
    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            if (!(pieces[x + y * 8] == 0))
            {
                if (pieces[x + y * 8] >= 8)
                {
                    bitmapBlackPieces |= 1 << (x + y * 8);
                }
                else
                {
                    bitmapWhitePieces |= 1 << (x + y * 8);
                }
            }
        }
    }

    unsigned int bitmapPieces = (bitmapBlackPieces | bitmapWhitePieces);

    for (int x = 0; x < 8; x++)
    {
        for (int y = 0; y < 8; y++)
        {
            int piece = pieces[x + y * 8 ];
            switch (piece)
            {
            case 0:
                // nothing
            case 1:
                unsigned int move1 = (1 << (x + (y - 1) * 8)) & ~bitmapPieces;
                unsigned int move2 = ((y >= 6) << (x + (y - 2) * 8)) & ~bitmapPieces;
                unsigned int moves3 = (((x > 0) << (x - 1) + (x < 7) << (x + 1)) << ((y - 1) * 8)) & bitmapBlackPieces;
                // white pawn
            case 2:
                // white bishop
                unsigned int posBishop = (1 << (x + y * 8));


                unsigned int deviation = x - y;
                unsigned int negDiagonal = 0x4020100804020101;
                unsigned int newnegDiagonal = negDiagonal >> (deviation * 8);

                newnegDiagonal &= ~posBishop;

                unsigned int bottomnegdiagonal = newnegDiagonal % posBishop;
                unsigned int topnegdiagonal = newnegDiagonal ^ bottomnegdiagonal;
                unsigned int bitsonBotNegD = (bottomnegdiagonal & bitmapPieces);
                unsigned int bitsonTopNegD = (topnegdiagonal & bitmapPieces);
                unsigned int closestnegtop = (bitsonTopNegD & (~bitsonTopNegD + 1));
                unsigned int closestnegbot = cuda::std::pow(2, floor(cuda::std::log2((double)bitsonBotNegD)));
                unsigned int botMovesneg = bottomnegdiagonal - (bottomnegdiagonal % closestnegbot);
                botMovesneg &= ~closestnegbot | bitmapBlackPieces;
                unsigned int topMovesneg = (topnegdiagonal % closestnegtop) | (closestnegtop & bitmapBlackPieces);

                unsigned int deviation = 7 - x - y;
                unsigned int posDiagonal = 0x102040810204080;
                unsigned int newposDiagonal = posDiagonal >> (deviation * 8);
                newposDiagonal &= (~(1 << (x + y * 8)));
                unsigned int bottomposdiagonal = newposDiagonal % (1 << (x + y * 8));
                unsigned int topposdiagonal = newposDiagonal ^ bottomposdiagonal;
                unsigned int bitsonBotPosD = (bottomposdiagonal & bitmapPieces);
                unsigned int bitsonTopPosD = (topposdiagonal & bitmapPieces);
                unsigned int closestpostop = (bitsonTopPosD & (~bitsonTopPosD + 1));
                unsigned int closestposbot = cuda::std::pow(2, floor(cuda::std::log2((double)bitsonBotPosD)));
                unsigned int botMovespos = bottomposdiagonal - (bottomposdiagonal % closestposbot);
                botMovespos &= ~closestposbot | bitmapBlackPieces;
                unsigned int topMovespos = (topposdiagonal % closestpostop) | (closestpostop & bitmapBlackPieces);



            case 3:
                unsigned int moves = 0xA1100110A;
                unsigned int rightmostArea = 0x1010101010;
                unsigned int bottommostArea = 0x1F;
                moves &= ~(rightmostArea * (x <= 1));
                moves &= ~((rightmostArea << 1) * (x == 0));
                moves &= ~((rightmostArea << 3) * (x == 7));
                moves &= ~((rightmostArea << 4) * (x >= 6));

                moves &= ~(bottommostArea * (y <= 1));
                moves &= ~((bottommostArea << 8) * (y == 0));
                moves &= ~((bottommostArea << 24) * (y == 7));
                moves &= ~((bottommostArea << 32) * (y >= 6));
                moves = moves << ((x - 2) + (y - 2) * 8);
                startingmoves[x][y] = moves & (~bitmapWhitePieces);

                // white knight
            case 4:
                // white rook castleable
                unsigned int rowbot = 0xFF;
                unsigned int collumnright = 0x101010101010101;
                unsigned int piecepos = 1 << (x + (y * 8));
                collumnright = collumnright << x;
                rowbot = rowbot << (y * 8);
                rowbot = rowbot & ~(piecepos);
                collumnright = collumnright & ~(piecepos);
                unsigned int moves1 = (((rowbot ^ bitmapPieces) + piecepos) ^ (rowbot ^ bitmapPieces)) & rowbot & ~bitmapWhitePieces;
                unsigned int closestposright = cuda::std::pow(2, floor(cuda::std::log2((double)(rowbot % piecepos))));
                unsigned int moves2 = (rowbot % piecepos) & (~closestposright + 1) & ~bitmapWhitePieces;

                unsigned int moves3 = (((collumnright ^ bitmapPieces) | (~collumnright) + piecepos) ^ (rowbot ^ bitmapPieces)) & collumnright & ~bitmapWhitePieces;
                unsigned int closestposbot = cuda::std::pow(2, floor(cuda::std::log2((double)((collumnright % piecepos)))));
                unsigned int moves4 = (collumnright % piecepos) & (~closestposbot + 1) & collumnright & ~bitmapWhitePieces;



            case 5:
                // white rook not castleable
                unsigned int rowbot = 0xFF;
                unsigned int collumnright = 0x101010101010101;
                unsigned int piecepos = 1 << (x + (y * 8));
                collumnright = collumnright << x;
                rowbot = rowbot << (y * 8);
                rowbot = rowbot & ~(piecepos);
                collumnright = collumnright & ~(piecepos);
                unsigned int moves1 = (((rowbot ^ bitmapPieces) + piecepos) ^ (rowbot ^ bitmapPieces)) & rowbot & ~bitmapWhitePieces;
                unsigned int closestposright = cuda::std::pow(2, floor(cuda::std::log2((double)(rowbot % piecepos))));
                unsigned int moves2 = (rowbot % piecepos) & (~closestposright + 1) & ~bitmapWhitePieces;

                unsigned int moves3 = (((collumnright ^ bitmapPieces) | (~collumnright) + piecepos) ^ (rowbot ^ bitmapPieces)) & collumnright & ~bitmapWhitePieces;
                unsigned int closestposbot = cuda::std::pow(2, floor(cuda::std::log2((double)((collumnright % piecepos)))));
                unsigned int moves4 = (collumnright % piecepos) & (~closestposbot + 1) & collumnright & ~bitmapWhitePieces;
            case 6:
                // white king
                unsigned int moves = 0x70507;
                unsigned int rightmostArea = 0x1010101010;
                unsigned int bottommostArea = 0x1F;
                moves &= ~(rightmostArea * (x == 0));
                moves &= ~((rightmostArea << 2) * (x == 7));

                moves &= ~(bottommostArea * (y == 0));
                moves &= ~((bottommostArea << 16) * (y == 7));
                moves = moves << ((x - 1) + (y - 1) * 8);
                startingmoves[x][y] = moves & (~bitmapWhitePieces);

                unsigned int castlekingside = 0x6000000000000000;
                unsigned int castlequeenside = 0xE00000000000000;

                moves |= (1 << 62) * (pieces[63] == 4 && !(castlekingside & bitmapPieces)); //castle king side
                moves |= (1 << 58) * (pieces[56] == 4 && !(castlequeenside & bitmapPieces)); //castle queen side
            case 7:
                // white queen
                // bishop part
                unsigned int piecepos = (1 << (x + y * 8));


                unsigned int deviation = x - y;
                unsigned int negDiagonal = 0x4020100804020101;
                unsigned int newnegDiagonal = negDiagonal >> (deviation * 8);

                newnegDiagonal &= ~piecepos;

                unsigned int bottomnegdiagonal = newnegDiagonal % piecepos;
                unsigned int topnegdiagonal = newnegDiagonal ^ bottomnegdiagonal;
                unsigned int bitsonBotNegD = (bottomnegdiagonal & bitmapPieces);
                unsigned int bitsonTopNegD = (topnegdiagonal & bitmapPieces);
                unsigned int closestnegtop = (bitsonTopNegD & (~bitsonTopNegD + 1));
                unsigned int closestnegbot = cuda::std::pow(2, floor(cuda::std::log2((double)bitsonBotNegD)));
                unsigned int botMovesneg = bottomnegdiagonal - (bottomnegdiagonal % closestnegbot);
                botMovesneg &= ~closestnegbot | bitmapBlackPieces;
                unsigned int topMovesneg = (topnegdiagonal % closestnegtop) | (closestnegtop & bitmapBlackPieces);

                unsigned int deviation = 7 - x - y;
                unsigned int posDiagonal = 0x102040810204080;
                unsigned int newposDiagonal = posDiagonal >> (deviation * 8);
                newposDiagonal &= (~(1 << (x + y * 8)));
                unsigned int bottomposdiagonal = newposDiagonal % (1 << (x + y * 8));
                unsigned int topposdiagonal = newposDiagonal ^ bottomposdiagonal;
                unsigned int bitsonBotPosD = (bottomposdiagonal & bitmapPieces);
                unsigned int bitsonTopPosD = (topposdiagonal & bitmapPieces);
                unsigned int closestpostop = (bitsonTopPosD & (~bitsonTopPosD + 1));
                unsigned int closestposbot = cuda::std::pow(2, floor(cuda::std::log2((double)bitsonBotPosD)));
                unsigned int botMovespos = bottomposdiagonal - (bottomposdiagonal % closestposbot);
                botMovespos &= ~closestposbot | bitmapBlackPieces;
                unsigned int topMovespos = (topposdiagonal % closestpostop) | (closestpostop & bitmapBlackPieces);
                
                //rook part

                unsigned int rowbot = 0xFF;
                unsigned int collumnright = 0x101010101010101;
                collumnright = collumnright << x;
                rowbot = rowbot << (y * 8);
                rowbot = rowbot & ~(piecepos);
                collumnright = collumnright & ~(piecepos);
                unsigned int moves1 = (((rowbot ^ bitmapPieces) + piecepos) ^ (rowbot ^ bitmapPieces)) & rowbot & ~bitmapWhitePieces;
                unsigned int closestposright = cuda::std::pow(2, floor(cuda::std::log2((double)(rowbot % piecepos))));
                unsigned int moves2 = (rowbot % piecepos) & (~closestposright + 1) & ~bitmapWhitePieces;

                unsigned int moves3 = (((collumnright ^ bitmapPieces) | (~collumnright) + piecepos) ^ (rowbot ^ bitmapPieces)) & collumnright & ~bitmapWhitePieces;
                unsigned int closestposbot = cuda::std::pow(2, floor(cuda::std::log2((double)((collumnright % piecepos)))));
                unsigned int moves4 = (collumnright % piecepos) & (~closestposbot + 1) & collumnright & ~bitmapWhitePieces;



            case 8:
                // black pawn
                unsigned int move1 = (1 << (x + (y + 1) * 8)) & ~bitmapPieces;
                unsigned int move2 = ((y <= 1) << (x + (y + 2) * 8)) & ~bitmapPieces;
                unsigned int moves3 = (((x > 0) << (x - 1) + (x < 7) << (x + 1)) << ((y - 1) * 8)) & bitmapWhitePieces;
                // white pawn
            case 9:
                // black bishop
            case 10:
                // black knight
            case 11:
                // black rook castleable
            case 12:
                // black rook not castleable
            case 13:
                // black king
            case 14:
                // black queen
            }
        }
    }

    
}

void FPBPTensorTrain(const int layerSizes, const int inputsSize, const int outputsSize);

std::string getexepath()
{
    char result[MAX_PATH];
    return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}

int main()
{
    dim3 UniversalPieceThreads(64, 16);



    size_t weightinSize = Hlayersizes * inputSize, 
        weighthSize = Hlayersizes * Hlayersizes * (HlayerNumber - 1), 
            weightoutSize = Hlayersizes * outputSize, biashSize = HlayerNumber * Hlayersizes, biasoutSize = outputSize;

    float* AllWeights = (float*)malloc((weightinSize + weighthSize + weightoutSize) * sizeof(float));


    float* AllBiases = (float*)malloc((biashSize + biasoutSize) * sizeof(float));

    float** d_ab = (float**)malloc((HlayerNumber + 1) * sizeof(void*));
    float** d_aw = (float**)malloc((HlayerNumber + 1) * sizeof(void*));

    float* Biases = (float*)malloc((HlayerNumber * Hlayersizes + outputSize) * sizeof(float));

    float* Weights = (float*)malloc(((HlayerNumber - 1) * Hlayersizes * Hlayersizes + inputSize * Hlayersizes + outputSize * Hlayersizes) * sizeof(float));

    fstream file("AINNData.bin", ios::binary | ios::in | ios::out);
    if (file.is_open())
    {


        file.read((char*)Biases, (HlayerNumber * Hlayersizes + outputSize) * sizeof(float));
        file.seekg((HlayerNumber * Hlayersizes + outputSize) * sizeof(float));
        file.read((char*)Weights, ((HlayerNumber - 1) * Hlayersizes * Hlayersizes + inputSize * Hlayersizes + outputSize * Hlayersizes) * sizeof(float));
        file.close();
    }




    float* d_bout;

    cudaMalloc((void**)&d_bout, outputSize * sizeof(float));
    

    float* d_win, * d_wout;

    
    cudaMalloc((void**)&d_wout, Hlayersizes * outputSize * sizeof(float));
    cudaMalloc((void**)&d_win, Hlayersizes * inputSize * sizeof(float));
    //weight add
    cudaMemcpy(d_win, Weights, inputSize * Hlayersizes * sizeof(float), cudaMemcpyHostToDevice);
    d_aw[0] = d_win;
    for (int i = 0; i < HlayerNumber - 1; i++)
    {
        float* d_w;
        cudaMalloc((void**)&d_w, Hlayersizes * Hlayersizes * sizeof(float));
        cudaMemcpy(d_w, Weights + (weightinSize + i * Hlayersizes * Hlayersizes) * sizeof(float), Hlayersizes * Hlayersizes * sizeof(float), cudaMemcpyHostToDevice);

        d_aw[i + 1] = d_w;
    }

    
    cudaMemcpy(d_wout, Weights + (weighthSize + weightinSize) * sizeof(float), outputSize * Hlayersizes * sizeof(float), cudaMemcpyHostToDevice);
    d_aw[HlayerNumber] = d_wout;

    for (int i = 0; i < HlayerNumber; i++)
    {
        float* d_b;
        cudaMalloc((void**)&d_b, Hlayersizes * sizeof(float));
        cudaMemcpy(d_b, Biases + i * Hlayersizes * sizeof(float), Hlayersizes * sizeof(float), cudaMemcpyHostToDevice);

        d_ab[i] = d_b;
    }

    cudaMemcpy(d_bout, Biases + weighthSize * sizeof(float), outputSize * sizeof(float), cudaMemcpyHostToDevice);

    d_ab[HlayerNumber + 1] = d_bout;


    cout << "got here\n";
    //setup our sprite with a texture

    SigmoidLayout sigfwd;

    sigfwd.init(Hlayersizes, batchSize);

    MatMulLayout inmatfwd;

    inmatfwd.init(1024 * 1024 * 8, Hlayersizes, batchSize, inputSize);

    cout << "got here\n";

    SigmoidLayoutBWD sigbwdh;

    sigbwdh.init(Hlayersizes, batchSize);

    MatMulLayout hmatfwd;

    hmatfwd.init(1024 * 1024 * 8, Hlayersizes, batchSize, Hlayersizes);

    float* d_tw, * d_tprev, * d_tnext;

    cudaMalloc((void**)&d_tw, Hlayersizes * Hlayersizes * sizeof(float));
    cudaMalloc((void**)&d_tprev, Hlayersizes * batchSize * sizeof(float));
    cudaMalloc((void**)&d_tnext, Hlayersizes * batchSize * sizeof(float));

    curandGenerator_t generator;

    curandCreateGenerator(&generator, CURAND_RNG_PSEUDO_DEFAULT);

    curandSetPseudoRandomGeneratorSeed(generator, time(NULL));

    curandGenerateUniform(generator, d_tw, Hlayersizes * Hlayersizes);
    curandGenerateUniform(generator, d_tnext, Hlayersizes * batchSize);
    curandGenerateUniform(generator, d_tprev, Hlayersizes * batchSize);

    for(int i = 0; i < 100; i++) hmatfwd.run(d_tw, d_tprev, d_tnext);

    float* t_next = (float*)malloc(Hlayersizes * batchSize * sizeof(float));

    cudaMemcpy(t_next, d_tnext, Hlayersizes * batchSize * sizeof(float), cudaMemcpyDeviceToHost);



    for (int i = 0; i < Hlayersizes * batchSize; i++)
    {
        cout << "vals = " << t_next[i] << endl;
    }

    MatMulLayout outmatfwd;

    outmatfwd.init(1024 * 1024 * 8, outputSize, batchSize, Hlayersizes);

    cout << "got here\n";

    MatMulLayoutBWD outmatbwd;

    outmatbwd.init(1024 * 1024 * 8, Hlayersizes, batchSize, outputSize);

    MatMulLayoutBWD hmatbwd;

    hmatbwd.init(1024 * 1024 * 8, Hlayersizes, batchSize, Hlayersizes);

    weightAddLayout outwAdd;

    outwAdd.init(1024 * 1024 * 8, Hlayersizes, batchSize, outputSize, learningRate);

    weightAddLayout hwAdd;

    outwAdd.init(1024 * 1024 * 8, Hlayersizes, batchSize, Hlayersizes, learningRate);

    weightAddLayout inwAdd;

    inwAdd.init(1024 * 1024 * 8, inputSize, batchSize, Hlayersizes, learningRate);

    ReduceLayoutab redh;

    redh.init(Hlayersizes, batchSize, 1024 * 1024 * 8, 1.0f, 1.0f);

    ReduceLayoutab redout;

    redout.init(outputSize, batchSize, 1024 * 1024 * 8, 1.0f, 1.0f);

    ReduceLayout redcost;

    redcost.init(1, Hlayersizes * batchSize, 1024 * 1024 * 8);

    AddLayout addw;

    addw.init(Hlayersizes * Hlayersizes * (HlayerNumber - 1) + Hlayersizes * inputSize + Hlayersizes * outputSize);

    AddLayout addb;

    addb.init(Hlayersizes * HlayerNumber + outputSize);

    BiasmatmulAdd biasAdd;

    biasAdd.init(1024 * 1024 * 8, Hlayersizes,  batchSize);

    BiasmatmulAdd biasAddOutLayer;

    biasAddOutLayer.init(1024 * 1024 * 8, outputSize, batchSize);

    AddLayoutab SSRDerLayout;

    SSRDerLayout.init(outputSize * batchSize, -2, 2);

    AddLayoutab SSRLayout;

    SSRLayout.init(outputSize * batchSize, -1, 1);
    
    float* input = (float*)malloc(inputSize * sizeof(float));

    float* d_in;
    cudaMalloc((void**)&d_in, inputSize * batchSize * sizeof(float));

    float* d_out;
    cudaMalloc((void**)&d_out, outputSize * batchSize * sizeof(float));

    float *d_next;



    float* placeHolder;
    cudaMallocHost((void**)&placeHolder, Hlayersizes * batchSize * sizeof(float), 0);

    for (int i = 0; i < Hlayersizes * batchSize; i++)
    {
        placeHolder[i] = 0;
    }

    float *d_prev, *prevValin, * outVals;
    float* d_costarr, * d_cost;

    float** prevVals = (float**)malloc(HlayerNumber * sizeof(void*));

    cudaMalloc((void**)&d_costarr, outputSize * batchSize * sizeof(float));
    cudaMalloc((void**)&outVals, outputSize * batchSize * sizeof(float));
    cudaMalloc((void**)&d_prev, Hlayersizes * batchSize * sizeof(float));
    cudaMalloc((void**)&d_cost, 1 * sizeof(float));
    cudaMalloc((void**)&d_next, Hlayersizes * batchSize * sizeof(float));

    cudaMalloc((void**)&prevValin, Hlayersizes * batchSize * sizeof(float));

    inmatfwd.run(d_aw[0], d_in, d_next);

    biasAdd.run(d_ab[0], d_next);

    sigfwd.run(d_next);

    cudaMemcpy(prevValin, d_next, Hlayersizes * batchSize * sizeof(float), cudaMemcpyDeviceToDevice);

    prevVals[0] = prevValin;

    for (int i = 0; i < HlayerNumber - 1; i++)
    {
        hmatfwd.run(d_aw[i + 1], d_next, d_next);

        biasAdd.run(d_ab[i + 1], d_next);

        sigfwd.run(d_next);

        float* prevVal;


        cudaMalloc((void**)&prevVal, Hlayersizes * batchSize * sizeof(float));
        cudaMemcpy(prevVal, d_next, Hlayersizes * batchSize * sizeof(float), cudaMemcpyDeviceToDevice);

        prevVals[i + 1] = prevVal;
    }



    outmatfwd.run(d_aw[HlayerNumber], d_next, outVals);



    biasAddOutLayer.run(d_ab[HlayerNumber], outVals);



    cudaMemcpy(d_costarr, outVals, outputSize * batchSize * sizeof(float), cudaMemcpyDeviceToDevice);


    SSRLayout.run(d_out, d_costarr);



    redcost.run(d_cost, d_costarr);




    SSRDerLayout.run(d_out, outVals);

    redout.run(d_ab[HlayerNumber], outVals);


    outmatbwd.run(d_aw[HlayerNumber], outVals, d_next);

    outwAdd.run(d_aw[HlayerNumber], outVals, prevVals[HlayerNumber - 1]);



    for (int i = HlayerNumber - 2; i >= 0; i--)
    {

        sigbwdh.run(d_next, prevVals[i + 1]);

        cudaMemcpy(d_prev, d_next, Hlayersizes* batchSize * sizeof(float), cudaMemcpyDeviceToDevice);

        redh.run(d_ab[i + 1], d_next);

        hmatbwd.run(d_aw[i + 1], d_prev, d_next);

        hwAdd.run(d_aw[i + 1], d_prev, prevVals[i]);

    }

    sigbwdh.run(d_next, prevVals[0]);

    redh.run(d_ab[0], d_next);



    inwAdd.run(d_aw[0], d_next, d_in);




    string parameter;


    sf::Texture white_rook_texture;

    if (!white_rook_texture.loadFromFile("C:\\Users\\chris\\source\\repos\\ChessAI\\ChessAI\\x64\\Debug\\white_rook.png"));
    sf::Texture white_knight_texture;

    if (!white_knight_texture.loadFromFile("C:\\Users\\chris\\source\\repos\\ChessAI\\ChessAI\\x64\\Debug\\white_knight.png"));

    sf::Texture white_bishop_texture;

    if (!white_bishop_texture.loadFromFile("C:\\Users\\chris\\source\\repos\\ChessAI\\ChessAI\\x64\\Debug\\white_bishop.png"));
    sf::Texture white_king_texture;

    if (!white_king_texture.loadFromFile("C:\\Users\\chris\\source\\repos\\ChessAI\\ChessAI\\x64\\Debug\\white_king.png"));
    sf::Texture white_queen_texture;

    if (!white_queen_texture.loadFromFile("C:\\Users\\chris\\source\\repos\\ChessAI\\ChessAI\\x64\\Debug\\white_queen.png"));
    sf::Texture white_pawn_texture;

    if (!white_pawn_texture.loadFromFile("C:\\Users\\chris\\source\\repos\\ChessAI\\ChessAI\\x64\\Debug\\white_pawn.png"));

    sf::Texture black_rook_texture;

    if (!black_rook_texture.loadFromFile("C:\\Users\\chris\\source\\repos\\ChessAI\\ChessAI\\x64\\Debug\\black_rook.png"));
    sf::Texture black_knight_texture;

    if (!black_knight_texture.loadFromFile("C:\\Users\\chris\\source\\repos\\ChessAI\\ChessAI\\x64\\Debug\\black_knight.png"));

    sf::Texture black_bishop_texture;

    if (!black_bishop_texture.loadFromFile("C:\\Users\\chris\\source\\repos\\ChessAI\\ChessAI\\x64\\Debug\\black_bishop.png"));
    sf::Texture black_king_texture;

    if (!black_king_texture.loadFromFile("C:\\Users\\chris\\source\\repos\\ChessAI\\ChessAI\\x64\\Debug\\black_king.png"));
    sf::Texture black_queen_texture;

    if (!black_queen_texture.loadFromFile("C:\\Users\\chris\\source\\repos\\ChessAI\\ChessAI\\x64\\Debug\\black_queen.png"));
    sf::Texture black_pawn_texture;

    if (!black_pawn_texture.loadFromFile("C:\\Users\\chris\\source\\repos\\ChessAI\\ChessAI\\x64\\Debug\\black_pawn.png"));


    ChessPosition pos;
    pos.setDefault();

    vector<vector<vector<int>>> AllMoves = pos.AllPieceMoves();


    cin >> parameter;
    if (parameter == "game")
    {
        bool showAllMoves = true;

        sf::Event event;

        sf::RenderWindow window(sf::VideoMode(1024, 1024), "SFML works!");
        window.setFramerateLimit(60);


        window.setKeyRepeatEnabled(false);

        bool mouseButtonleftHeld = false;
        bool mouseButtonrightHeld = false;
        float mouseInitleftPosx;
        float mouseInitleftPosy;

        vector<int> SelectedPiece;
        SelectedPiece.push_back(-1);
        SelectedPiece.push_back(-1);

        int pieceMovesIdx = -1;

        bool inPromotion = false;
        int PromotionX = -1;
        int PromotionY = -1;

        float mouseInitrightPosx;
        float mouseInitrightPosy;
        while (window.isOpen())
        {
            float squareSize = 128.0f;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();



                if (event.type == sf::Event::MouseButtonPressed)
                {
                    if (event.mouseButton.button == sf::Mouse::Left)
                    {

                        int pieceX = floor((float)(event.mouseButton.x) / (float)squareSize);
                        int pieceY = floor((float)(event.mouseButton.y) / (float)squareSize);

                        SelectedPiece[0] = pieceX;
                        SelectedPiece[1] = pieceY;

                        cout << "got: [" << pieceX << ", " << pieceY << "]\n";
                        showAllMoves = false;
                    }

                    if (event.mouseButton.button == sf::Mouse::Right)
                    {
                        mouseInitrightPosx = event.mouseButton.x;
                        mouseInitrightPosy = event.mouseButton.y;

                        mouseButtonrightHeld = true;
                    }
                }
            }




            window.clear();

            //draw squares

            if (!inPromotion)
            {
                if (pieceMovesIdx < AllMoves.size() && pieceMovesIdx != -1)
                {
                    int j = 1;
                    bool endCheck = false;
                    while (pieceMovesIdx != -1 && !endCheck)
                    {
                        if (j < AllMoves.at(pieceMovesIdx).size())
                        {
                            if (SelectedPiece[0] == AllMoves.at(pieceMovesIdx)[j][0] && SelectedPiece[1] == AllMoves.at(pieceMovesIdx)[j][1])
                            {
                                int Piecetaken = pos.getPieceID(SelectedPiece[0], SelectedPiece[1]);
                                int Piecetomove = pos.getPieceID(AllMoves.at(pieceMovesIdx)[0][0], AllMoves.at(pieceMovesIdx)[0][1]);
                                cout << Piecetomove << endl;
                                pos.RemovePiece(AllMoves.at(pieceMovesIdx)[0][0], AllMoves.at(pieceMovesIdx)[0][1]);
                                if (Piecetomove % 7 == 6)
                                {
                                    pos.AddPiece(SelectedPiece[0], SelectedPiece[1], Piecetomove);
                                    cout << "king move\n";
                                    //if king
                                    if (SelectedPiece[0] - AllMoves.at(pieceMovesIdx)[0][0] >= 2)
                                    {
                                        //short Castle
                                        cout << "short castle move\n";
                                        pos.AddPiece(5, 7 * (!pos.SIDEMOVE), 5 + pos.SIDEMOVE * 7);
                                        pos.RemovePiece(7, 7 * (!pos.SIDEMOVE));
                                        if (pos.getPieceID(0, 7 * (!pos.SIDEMOVE)) == 4 + pos.SIDEMOVE * 7) pos.AddPiece(0, 7 * (!pos.SIDEMOVE), 5 + pos.SIDEMOVE * 7);
                                    }
                                    if (SelectedPiece[0] - AllMoves.at(pieceMovesIdx)[0][0] <= -2)
                                    {
                                        cout << "long castle move\n";
                                        //long Castle
                                        pos.AddPiece(3, 7 * (!pos.SIDEMOVE), 5 + pos.SIDEMOVE * 7);
                                        pos.RemovePiece(0, 7 * (!pos.SIDEMOVE));
                                        if (pos.getPieceID(7, 7 * (!pos.SIDEMOVE)) == 4 + pos.SIDEMOVE * 7) pos.AddPiece(7, 7 * (!pos.SIDEMOVE), 5 + pos.SIDEMOVE * 7);
                                    }
                                    else
                                    {
                                        if (pos.getPieceID(0, 7 * (!pos.SIDEMOVE)) == 4 + pos.SIDEMOVE * 7) pos.AddPiece(0, 7 * (!pos.SIDEMOVE), 5 + pos.SIDEMOVE * 7);
                                        if (pos.getPieceID(7, 7 * (!pos.SIDEMOVE)) == 4 + pos.SIDEMOVE * 7) pos.AddPiece(7, 7 * (!pos.SIDEMOVE), 5 + pos.SIDEMOVE * 7);
                                    }
                                    pos.SIDEMOVE = !pos.SIDEMOVE;
                                    AllMoves = pos.AllPieceMoves();
                                    pos.AddPrevMove(-1, -1);
                                }
                                else if (Piecetomove % 7 == 1)
                                {
                                    if (SelectedPiece[1] == pos.SIDEMOVE * 7)
                                    {
                                        inPromotion = true;
                                        PromotionX = SelectedPiece[0];
                                        PromotionY = SelectedPiece[1];
                                        SelectedPiece[0] = -1;
                                        SelectedPiece[1] = -1;
                                    }
                                    else
                                    {
                                        pos.AddPiece(SelectedPiece[0], SelectedPiece[1], Piecetomove);
                                        if (Piecetaken == 0)
                                        {
                                            //en passant
                                            pos.RemovePiece(SelectedPiece[0], SelectedPiece[1] - 1 + 2 * (!pos.SIDEMOVE));
                                        }

                                        if (abs(SelectedPiece[1] - AllMoves.at(pieceMovesIdx)[0][1]) >= 2)
                                        {
                                            //flying pawn to put in previous move
                                            pos.AddPrevMove(SelectedPiece[0], SelectedPiece[1]);
                                        }
                                        else
                                        {
                                            pos.AddPrevMove(-1, -1);
                                        }

                                        pos.SIDEMOVE = !pos.SIDEMOVE;
                                        AllMoves = pos.AllPieceMoves();
                                    }
                                }
                                else
                                {
                                    pos.AddPiece(SelectedPiece[0], SelectedPiece[1], Piecetomove);
                                    pos.AddPrevMove(-1, -1);
                                    pos.SIDEMOVE = !pos.SIDEMOVE;
                                    AllMoves = pos.AllPieceMoves();
                                }


                                if (AllMoves.size() > 0) if (!inPromotion) cout << pos.giveSimpleEval() << endl;


                                pieceMovesIdx = -1;
                                endCheck = true;
                                showAllMoves = true;
                            }
                        }
                        else {
                            endCheck = true;
                            pieceMovesIdx = -1;
                        }
                        j++;
                    }

                }
            }





            for (int i = 0; i < 8 * 8; i++)
            {
                int h = i % 8;
                int v = floor((float)i / 8.0f);
                sf::CircleShape shape(sqrt(squareSize * squareSize * 2) / 2);
                shape.setPointCount(4);
                if ((i + v) % 2 == 0)shape.setFillColor(sf::Color::Color(100, 25, 25));
                else shape.setFillColor(sf::Color::Color(255, 100, 100));
                shape.setOrigin(sqrt(squareSize * squareSize * 2) / 4, sqrt(squareSize * squareSize * 2) / 4);
                shape.setPosition(h * squareSize, v * squareSize);
                shape.move(squareSize / 2.0f, 0.0f);
                shape.rotate(45.0f);
                window.draw(shape);
            }

            if (AllMoves.size() == 0) cout << "CHECKMATE!";
            else if (AllMoves[0].size() == 1) cout << "STALEMATE!";

            for (int i = 0; i < 8; i++)
            {
                //cout << "at layer: " << i << endl;
                for (int j = 0; j < 8; j++)
                {
                    //cout << " value = " << pos.Pieces[i][j] << " ";
                    if (pos.Pieces[i][j] != 0)
                    {

                        sf::Sprite pieceSprite;



                        if (pos.Pieces[i][j] == 1)
                        {
                            pieceSprite.setTexture(white_pawn_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(i * squareSize + squareSize / 2, j * squareSize + squareSize / 2);
                        }
                        if (pos.Pieces[i][j] == 2)
                        {
                            pieceSprite.setTexture(white_bishop_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(i * squareSize + squareSize / 2, j * squareSize + squareSize / 2);
                        }
                        if (pos.Pieces[i][j] == 3)
                        {
                            pieceSprite.setTexture(white_knight_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(i * squareSize + squareSize / 2, j * squareSize + squareSize / 2);
                        }
                        if (pos.Pieces[i][j] == 4)
                        {
                            pieceSprite.setTexture(white_rook_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(i * squareSize + squareSize / 2, j * squareSize + squareSize / 2);
                        }
                        if (pos.Pieces[i][j] == 5)
                        {
                            pieceSprite.setTexture(white_rook_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(i * squareSize + squareSize / 2, j * squareSize + squareSize / 2);
                        }
                        if (pos.Pieces[i][j] == 6)
                        {
                            pieceSprite.setTexture(white_king_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.35f, 0.35f);
                            pieceSprite.setPosition(i * squareSize + squareSize / 2, j * squareSize + squareSize / 2);
                        }
                        if (pos.Pieces[i][j] == 7)
                        {
                            pieceSprite.setTexture(white_queen_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.35f, 0.35f);
                            pieceSprite.setPosition(i * squareSize + squareSize / 2, j * squareSize + squareSize / 2);
                        }
                        if (pos.Pieces[i][j] == 8)
                        {
                            pieceSprite.setTexture(black_pawn_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(i * squareSize + squareSize / 2, j * squareSize + squareSize / 2);
                        }
                        if (pos.Pieces[i][j] == 9)
                        {
                            pieceSprite.setTexture(black_bishop_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(i * squareSize + squareSize / 2, j * squareSize + squareSize / 2);
                        }
                        if (pos.Pieces[i][j] == 10)
                        {
                            pieceSprite.setTexture(black_knight_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(i * squareSize + squareSize / 2, j * squareSize + squareSize / 2);
                        }
                        if (pos.Pieces[i][j] == 11)
                        {
                            pieceSprite.setTexture(black_rook_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(i * squareSize + squareSize / 2, j * squareSize + squareSize / 2);
                        }
                        if (pos.Pieces[i][j] == 12)
                        {
                            pieceSprite.setTexture(black_rook_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(i * squareSize + squareSize / 2, j * squareSize + squareSize / 2);
                        }
                        if (pos.Pieces[i][j] == 13)
                        {
                            pieceSprite.setTexture(black_king_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.35f, 0.35f);
                            pieceSprite.setPosition(i * squareSize + squareSize / 2, j * squareSize + squareSize / 2);
                        }
                        if (pos.Pieces[i][j] == 14)
                        {
                            pieceSprite.setTexture(black_queen_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.35f, 0.35f);
                            pieceSprite.setPosition(i * squareSize + squareSize / 2, j * squareSize + squareSize / 2);
                        }





                        window.draw(pieceSprite);
                    }
                }

                //cout << endl;
            }



            if (!inPromotion)
            {
                if (!(AllMoves[0].size() == 1))
                {
                    for (int i = 0; i < AllMoves.size(); i++)
                    {
                        vector<vector<int>> pieceMoves = AllMoves.at(i);

                        if (SelectedPiece[0] == pieceMoves[0][0] && SelectedPiece[1] == pieceMoves[0][1])
                        {
                            pieceMovesIdx = i;
                            for (int j = 1; j < pieceMoves.size(); j++)
                            {
                                vector<int> move = pieceMoves.at(j);
                                int size = 20;

                                sf::CircleShape shape(size);
                                shape.setFillColor(sf::Color::Color(100, 100, 100));
                                shape.setOrigin(size / 2, size / 2);
                                shape.setPosition(move[0] * squareSize + squareSize / 2, move[1] * squareSize + squareSize / 2);

                                window.draw(shape);

                            }
                        }
                    }
                }
            }

            if (inPromotion)
            {
                //draw square shapes and pieces
                for (int i = 0; i < 4; i++)
                {
                    int h = PromotionX;
                    int v = PromotionY + i * ((!pos.SIDEMOVE) * 2 - 1);
                    sf::CircleShape shape(sqrt(squareSize * squareSize * 2) / 2);
                    shape.setPointCount(4);


                    shape.setFillColor(sf::Color::Color(255, 255, 255));
                    shape.setOrigin(sqrt(squareSize * squareSize * 2) / 4, sqrt(squareSize * squareSize * 2) / 4);
                    shape.setPosition(h * squareSize, v * squareSize);
                    shape.move(squareSize / 2.0f, 0.0f);
                    shape.rotate(45.0f);
                    window.draw(shape);

                    sf::Sprite pieceSprite;

                    if (pos.SIDEMOVE == 0)
                    {

                        if (i == 0)
                        {
                            pieceSprite.setTexture(white_queen_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.35f, 0.35f);
                            pieceSprite.setPosition(h * squareSize + squareSize / 2, v * squareSize + squareSize / 2);
                        }
                        if (i == 1)
                        {
                            pieceSprite.setTexture(white_rook_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(h * squareSize + squareSize / 2, v * squareSize + squareSize / 2);
                        }
                        if (i == 2)
                        {
                            pieceSprite.setTexture(white_bishop_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(h * squareSize + squareSize / 2, v * squareSize + squareSize / 2);
                        }
                        if (i == 3)
                        {
                            pieceSprite.setTexture(white_knight_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(h * squareSize + squareSize / 2, v * squareSize + squareSize / 2);
                        }
                    }
                    else
                    {
                        if (i == 0)
                        {
                            pieceSprite.setTexture(black_queen_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.35f, 0.35f);
                            pieceSprite.setPosition(h * squareSize + squareSize / 2, v * squareSize + squareSize / 2);
                        }
                        if (i == 1)
                        {
                            pieceSprite.setTexture(black_rook_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(h * squareSize + squareSize / 2, v * squareSize + squareSize / 2);
                        }
                        if (i == 2)
                        {
                            pieceSprite.setTexture(black_bishop_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(h * squareSize + squareSize / 2, v * squareSize + squareSize / 2);
                        }
                        if (i == 3)
                        {
                            pieceSprite.setTexture(black_knight_texture);
                            pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                            pieceSprite.setScale(0.4f, 0.4f);
                            pieceSprite.setPosition(h * squareSize + squareSize / 2, v * squareSize + squareSize / 2);
                        }
                    }

                    window.draw(pieceSprite);

                    if (SelectedPiece[0] == h && SelectedPiece[1] == v)
                    {
                        if (i == 0)
                        {
                            pos.AddPiece(PromotionX, PromotionY, 7 + 7 * pos.SIDEMOVE);
                        }
                        if (i == 1)
                        {
                            pos.AddPiece(PromotionX, PromotionY, 5 + 7 * pos.SIDEMOVE);
                        }
                        if (i == 2)
                        {
                            pos.AddPiece(PromotionX, PromotionY, 2 + 7 * pos.SIDEMOVE);
                        }
                        if (i == 3)
                        {
                            pos.AddPiece(PromotionX, PromotionY, 3 + 7 * pos.SIDEMOVE);
                        }

                        inPromotion = false;
                        pos.SIDEMOVE = !pos.SIDEMOVE;
                        AllMoves = pos.AllPieceMoves();
                        if (AllMoves.size() == 0) cout << "Checkmate";
                        else if (AllMoves[0].size() == 1) cout << "Stalemate";
                        else if (AllMoves.size() > 0) cout << pos.giveSimpleEval() << endl;
                    }

                }
            }

            window.display();
        }
    }
    //
    if (parameter == "test")
    {
        sf::Event event;

        sf::RenderWindow window(sf::VideoMode(1024, 1024), "SFML works!");
        window.setFramerateLimit(60);


        window.setKeyRepeatEnabled(false);

        ChessPosition TestPos;
        TestPos.setDefault();

        fstream Movesfromgame("data_uci.pgn", ios::in);
        fstream StockFishEval("stockfish.csv", ios::in);
        bool MovesfromgameOpen = Movesfromgame.is_open();
        bool StockFishEvalOpen = StockFishEval.is_open();

        bool mouseButtonleftHeld = false;
        bool mouseButtonrightHeld = false;
        float mouseInitleftPosx;
        float mouseInitleftPosy;

        float mouseInitrightPosx;
        float mouseInitrightPosy;
        if (MovesfromgameOpen && StockFishEvalOpen)
        {
            string Moveline;
            getline(Movesfromgame, Moveline);
            string Stockfishline;
            getline(StockFishEval, Stockfishline);

            int mateEval1Side = -1;

            while (window.isOpen())
            {
                bool switchLines = false;
                while (window.pollEvent(event))
                {
                    if (event.type == sf::Event::Closed)
                        window.close();

                    if (event.type == sf::Event::MouseButtonPressed)
                    {
                        if (event.mouseButton.button == sf::Mouse::Left)
                        {
                            mouseInitleftPosx = event.mouseButton.x;
                            mouseInitleftPosy = event.mouseButton.y;

                            mouseButtonleftHeld = true;
                        }

                        if (event.mouseButton.button == sf::Mouse::Right)
                        {
                            mouseInitrightPosx = event.mouseButton.x;
                            mouseInitrightPosy = event.mouseButton.y;

                            mouseButtonrightHeld = true;
                        }
                    }
                    if (event.type == sf::Event::MouseButtonReleased)
                    {
                        if (event.mouseButton.button == sf::Mouse::Left)
                        {
                            mouseButtonleftHeld = false;
                        }
                        if (event.mouseButton.button == sf::Mouse::Right)
                        {
                            mouseButtonrightHeld = false;
                        }
                    }

                    if (event.type == sf::Event::MouseWheelScrolled)
                    {
                        float wheelDelta = event.mouseWheelScroll.delta;
                        if (fabsf(wheelDelta) != wheelDelta)
                        {
                            if (switchLines)
                            {
                                getline(Movesfromgame, Moveline);
                                getline(StockFishEval, Stockfishline);

                                switchLines = false;
                            }
                            int xInit = (int)Moveline[0] - 97;
                            int yInit = Moveline[1] - 49;
                            int xPost = (int)Moveline[2] - 97;
                            int yPost = Moveline[3] - 49;

                            Moveline.erase(Moveline.begin(), Moveline.begin() + 4);

                            int piece = TestPos.getPieceID(xInit, yInit);

                            if (piece - 7 * (TestPos.SIDEMOVE) == 6 && abs(xPost - xInit) >= 2)
                            {
                                //for if the king castled
                                TestPos.RemovePiece(-7 * (xPost < 4) + 7, yInit);
                                TestPos.AddPiece(-4 * (xPost < 4) + 6, yInit, 4 + 7 * TestPos.SIDEMOVE);
                            }
                            else if (piece - 7 * (TestPos.SIDEMOVE) == 1 && abs(yPost - yInit) >= 2)
                            {
                                //if the move was a double pawn move, store the move in the prevmove, because we want to give the info to the ai if we can enpassant
                                bool StoredPrevMove = false;
                                TestPos.PreviousMove[0] = xPost;
                                TestPos.PreviousMove[1] = yPost;
                                StoredPrevMove = true;
                            }
                            else if (piece - 7 * (TestPos.SIDEMOVE) == 1 && xPost - xInit != 0 && yPost - yInit != 0 && TestPos.getPieceID(xPost, yPost) == 0)
                            {
                                //en passant

                            }

                            if (Moveline[0] != ' ' || Moveline.length() == 1)
                            {
                                //if there is a pawn promotion
                                if (Moveline[0] == 'K') TestPos.AddPiece(xPost, yPost, 3 + 7 * TestPos.SIDEMOVE);
                                else if (Moveline[0] == 'B') TestPos.AddPiece(xPost, yPost, 2 + 7 * TestPos.SIDEMOVE);
                                else if (Moveline[0] == 'R') TestPos.AddPiece(xPost, yPost, 4 + 7 * TestPos.SIDEMOVE);
                                else if (Moveline[0] == 'Q') TestPos.AddPiece(xPost, yPost, 7 + 7 * TestPos.SIDEMOVE);

                                Moveline.erase(Moveline.begin(), Moveline.begin() + 1);
                            }
                            else
                            {
                                TestPos.AddPiece(xPost, yPost, piece);
                                Moveline.erase(Moveline.begin(), Moveline.begin());
                            }


                            TestPos.RemovePiece(xInit, yInit);

                            int spaceidx = Stockfishline.find(' ');
                            int eval;

                            if (spaceidx != -1)
                            {
                                if (Stockfishline.substr(0, spaceidx) == "NA")
                                {

                                }
                                else eval = stoi(Stockfishline.substr(0, spaceidx));
                            }
                            else
                            {
                                if (Stockfishline == "NA")
                                {
                                    if (mateEval1Side == -1)
                                    {
                                        vector<vector<vector<int>>> moves = TestPos.AllPieceMoves();

                                        for (int i = 0; i < moves.size(); i++)
                                        {
                                            vector<vector<int>> move = moves[i];
                                            int pieceidx = TestPos.getPieceID(move.at(0)[0], move.at(0)[0]);
                                            for (int j = 1; j < move.size(); j++)
                                            {

                                            }
                                        }


                                        eval = 10000 - 20000 * TestPos.SIDEMOVE;
                                    }


                                }
                                else eval = stoi(Stockfishline);

                            }
                            Stockfishline.erase(0, spaceidx + 1);
                        }

                    }
                }


                window.clear();

                //draw squares

                float squareSize = 128.0f;

                for (int i = 0; i < 8 * 8; i++)
                {
                    int h = i % 8;
                    int v = floor((float)i / 8.0f);
                    sf::CircleShape shape(sqrt(squareSize * squareSize * 2) / 2);
                    shape.setPointCount(4);
                    if ((i + v) % 2 == 0)shape.setFillColor(sf::Color::Color(100, 25, 25));
                    else shape.setFillColor(sf::Color::Color(255, 100, 100));
                    shape.setOrigin(sqrt(squareSize * squareSize * 2) / 4, sqrt(squareSize * squareSize * 2) / 4);
                    shape.setPosition(h * squareSize, v * squareSize);
                    shape.move(squareSize / 2.0f, 0.0f);
                    shape.rotate(45.0f);
                    window.draw(shape);
                }



                for (int i = 0; i < 8; i++)
                {
                    //cout << "at layer: " << i << endl;
                    for (int j = 0; j < 8; j++)
                    {
                        //cout << " value = " << pos.Pieces[i][j] << " ";
                        if (TestPos.Pieces[i][j] != 0)
                        {

                            sf::Sprite pieceSprite;

                            if (TestPos.Pieces[i][j] == 1)
                            {
                                pieceSprite.setTexture(white_pawn_texture);
                                pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                                pieceSprite.setScale(0.4f, 0.4f);
                                pieceSprite.setPosition(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2);
                            }
                            if (TestPos.Pieces[i][j] == 2)
                            {
                                pieceSprite.setTexture(white_bishop_texture);
                                pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                                pieceSprite.setScale(0.4f, 0.4f);
                                pieceSprite.setPosition(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2);
                            }
                            if (TestPos.Pieces[i][j] == 3)
                            {
                                pieceSprite.setTexture(white_knight_texture);
                                pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                                pieceSprite.setScale(0.4f, 0.4f);
                                pieceSprite.setPosition(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2);
                            }
                            if (TestPos.Pieces[i][j] == 4)
                            {
                                pieceSprite.setTexture(white_rook_texture);
                                pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                                pieceSprite.setScale(0.4f, 0.4f);
                                pieceSprite.setPosition(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2);
                            }
                            if (TestPos.Pieces[i][j] == 5)
                            {
                                pieceSprite.setTexture(white_rook_texture);
                                pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                                pieceSprite.setScale(0.4f, 0.4f);
                                pieceSprite.setPosition(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2);
                            }
                            if (TestPos.Pieces[i][j] == 6)
                            {
                                pieceSprite.setTexture(white_king_texture);
                                pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                                pieceSprite.setScale(0.35f, 0.35f);
                                pieceSprite.setPosition(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2);
                            }
                            if (TestPos.Pieces[i][j] == 7)
                            {
                                pieceSprite.setTexture(white_queen_texture);
                                pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                                pieceSprite.setScale(0.35f, 0.35f);
                                pieceSprite.setPosition(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2);
                            }
                            if (TestPos.Pieces[i][j] == 8)
                            {
                                pieceSprite.setTexture(black_pawn_texture);
                                pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                                pieceSprite.setScale(0.4f, 0.4f);
                                pieceSprite.setPosition(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2);
                            }
                            if (TestPos.Pieces[i][j] == 9)
                            {
                                pieceSprite.setTexture(black_bishop_texture);
                                pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                                pieceSprite.setScale(0.4f, 0.4f);
                                pieceSprite.setPosition(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2);
                            }
                            if (TestPos.Pieces[i][j] == 10)
                            {
                                pieceSprite.setTexture(black_knight_texture);
                                pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                                pieceSprite.setScale(0.4f, 0.4f);
                                pieceSprite.setPosition(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2);
                            }
                            if (TestPos.Pieces[i][j] == 11)
                            {
                                pieceSprite.setTexture(black_rook_texture);
                                pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                                pieceSprite.setScale(0.4f, 0.4f);
                                pieceSprite.setPosition(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2);
                            }
                            if (TestPos.Pieces[i][j] == 12)
                            {
                                pieceSprite.setTexture(black_rook_texture);
                                pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                                pieceSprite.setScale(0.4f, 0.4f);
                                pieceSprite.setPosition(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2);
                            }
                            if (TestPos.Pieces[i][j] == 13)
                            {
                                pieceSprite.setTexture(black_king_texture);
                                pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                                pieceSprite.setScale(0.35f, 0.35f);
                                pieceSprite.setPosition(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2);
                            }
                            if (TestPos.Pieces[i][j] == 14)
                            {
                                pieceSprite.setTexture(black_queen_texture);
                                pieceSprite.setOrigin((pieceSprite.getLocalBounds()).getSize().x / 2, (pieceSprite.getLocalBounds()).getSize().y / 2);
                                pieceSprite.setScale(0.35f, 0.35f);
                                pieceSprite.setPosition(j * squareSize + squareSize / 2, i * squareSize + squareSize / 2);
                            }




                            window.draw(pieceSprite);
                        }
                    }

                    //cout << endl;
                }



                window.display();


            }
        }
    }


    float* input1, * output1;
    float* Weights, * Biases;
    float* dev_in, * dev_out;
    float* dev_w, * dev_b;
    float* neuralNetworkOutput, * dev_netout;
    //allocate space for the variables on the device


    gpuErrchk(cudaMalloc((void**)&dev_netout, sizeof(float)));
    gpuErrchk(cudaMalloc((void**)&dev_in, inputSize * sizeof(float)));
    gpuErrchk(cudaMalloc((void**)&dev_out, outputSize * sizeof(float)));
    gpuErrchk(cudaMalloc((void**)&dev_b, (HlayerNumber * Hlayersizes + outputSize) * sizeof(float)));
    gpuErrchk(cudaMalloc((void**)&dev_w, ((HlayerNumber - 1) * Hlayersizes * Hlayersizes + inputSize * Hlayersizes + outputSize * Hlayersizes) * sizeof(float)));

    //allocate space for the variables on the host
    input1 = (float*)malloc(inputSize * sizeof(float));

    Weights = (float*)malloc(((HlayerNumber - 1) * Hlayersizes * Hlayersizes + inputSize * Hlayersizes + outputSize * Hlayersizes) * sizeof(float));
    Biases = (float*)malloc((HlayerNumber * Hlayersizes + outputSize) * sizeof(float));
    neuralNetworkOutput = (float*)malloc(sizeof(float));
    output1 = (float*)malloc(outputSize * sizeof(float));

    fstream file("AINNData.bin", ios::binary | ios::in | ios::out);
    if (file.is_open())
    {
        file.read((char*)Biases, (HlayerNumber * Hlayersizes + outputSize) * sizeof(float));
        file.seekg((HlayerNumber * Hlayersizes + outputSize) * sizeof(float));
        file.read((char*)Weights, ((HlayerNumber - 1) * Hlayersizes * Hlayersizes + inputSize * Hlayersizes + outputSize * Hlayersizes) * sizeof(float));
        file.close();
    }
    else
    {
        cout << "error while opening file\n";

        //generate numbers for all values in the neural network

        for (int i = 0; i < inputSize * Hlayersizes; i++)
        {
            Weights[i] = (float)(rand() % 2001 - 1000) / 10000.0f;
            if (Weights[i] == 0.0f) Weights[i] = (float)(rand() % 2001 - 1000) / 10000.0f;
        }

        for (int i = 0; i < (HlayerNumber - 1) * Hlayersizes * Hlayersizes; i++)
        {
            //
            Weights[i + inputSize * Hlayersizes] = (float)(rand() % 2001 - 1000) / 1000.0f;
            if (Weights[i + inputSize * Hlayersizes] == 0.0f) Weights[i + inputSize * Hlayersizes] = (float)(rand() % 2001 - 1000) / 1000.0f;
        }

        for (int i = 0; i < HlayerNumber * Hlayersizes; i++)
        {
            Biases[i] = 0.0f;
        }
        ///
        for (int i = 0; i < outputSize * Hlayersizes; i++)
        {
            Weights[i + (HlayerNumber - 1) * Hlayersizes * Hlayersizes + inputSize * Hlayersizes] = (float)(rand() % 2001 - 1000) / 1000.0f;
            if (Weights[i + (HlayerNumber - 1) * Hlayersizes * Hlayersizes + inputSize * Hlayersizes] == 0) Weights[i + (HlayerNumber - 1) * Hlayersizes * Hlayersizes + inputSize * Hlayersizes] = (float)(rand() % 2001 - 1000) / 1000.0f;
        }
        for (int i = 0; i < outputSize; i++)
        {
            Biases[i + HlayerNumber * Hlayersizes] = 0.0f;
        }
    }

    gpuErrchk(cudaMemcpy(dev_b, Biases, (HlayerNumber * Hlayersizes + outputSize) * sizeof(float), cudaMemcpyHostToDevice));
    gpuErrchk(cudaMemcpy(dev_w, Weights, (outputSize * Hlayersizes + (HlayerNumber - 1) * Hlayersizes * Hlayersizes + inputSize * Hlayersizes) * sizeof(float), cudaMemcpyHostToDevice));

    dim3 threadsPerBlock(Hlayersizes, 1024 / Hlayersizes);

    int StartGame = 2000;

    int goodGameIDX = 0;

    if (parameter == "train")
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(1, 500); // distribution in range [1, 6]

        ChessPosition TrainPos;
        TrainPos.setDefault();

        fstream Movesfromgame("chessdatabase.pgn", ios::in);
        bool MovesfromgameOpen = Movesfromgame.is_open();
        if (MovesfromgameOpen)
        {
            cout << "opened file\n";
            bool endLines = false;
            int idx = 0;

            int tempIdx = 0;
            while (!endLines && idx < 100000)
            {


                TrainPos.setDefault();

                cout << "Starting new Game #" << idx + 1 << endl;

                string Moveline;
                getline(Movesfromgame, Moveline);
                bool atGoodLine = false;
                while (!atGoodLine)
                {
                    if ((Moveline[0] == '[' || Moveline[0] == ' ') || Moveline.size() < 2)
                    {
                        getline(Movesfromgame, Moveline);
                    }
                    else
                    {
                        if (goodGameIDX >= StartGame)
                        {
                            atGoodLine = true;
                        }
                        else
                        {
                            getline(Movesfromgame, Moveline); 
                            goodGameIDX++;
                        }
                    }
                }

                bool endline = false;

                Moveline.erase(Moveline.begin(), Moveline.begin() + 2);

                

                int moveIIDX = 0;


                moveIIDX = 0;
                while (!endline)
                {
                    int spaceIdx = Moveline.find(' ');

                    if (spaceIdx > 0)
                    {


                        string move = Moveline.substr(0, spaceIdx);


                        TrainPos.DoMoveByString(move);

                        TrainPos.SIDEMOVE = !TrainPos.SIDEMOVE;
                        float expectedEval;

                        if (dist6(rng) == 2 || (((moveIIDX >> 2) + 1) > dist6(rng)))
                        {
                            vector<vector<vector<int>>> moves = TrainPos.AllPieceMoves();
                            if (moves.size() == 0)
                            {
                                expectedEval = -40 + 80 * TrainPos.SIDEMOVE;
                            }
                            else if (moves[0].size() == 1)
                            {
                                expectedEval = 0;
                            }
                            else
                            {
                                expectedEval = TrainPos.giveSimpleEval();
                            }


                            for (int i = 0; i < 64; i++)
                            {
                                unsigned int indexx = i % 8;
                                unsigned int indexy = i >> 3;

                                unsigned int pieceIdx = TrainPos.getPieceID(indexx, indexy);
                                if (pieceIdx > 7) pieceIdx++;
                                input1[i * 4] = (float)(pieceIdx & 1);
                                input1[i * 4 + 1] = (float)((pieceIdx & 2) >> 1);
                                input1[i * 4 + 2] = (float)((pieceIdx & 4) >> 2);
                                input1[i * 4 + 3] = (float)((pieceIdx & 8) >> 3);
                            }

                            input1[256] = (float)(TrainPos.PreviousMove[0] & 1);
                            input1[256 + 1] = (float)((TrainPos.PreviousMove[0] & 2) >> 1);
                            input1[256 + 2] = (float)((TrainPos.PreviousMove[0] & 4) >> 2);
                            input1[256 + 3] = (float)(TrainPos.PreviousMove[1] & 1);
                            input1[256 + 4] = (float)((TrainPos.PreviousMove[1] & 2) >> 1);
                            input1[256 + 5] = (float)((TrainPos.PreviousMove[1] & 4) >> 2);

                            input1[262] = (float)(TrainPos.SIDEMOVE);
                            input1[263] = (float)(TrainPos.SIDEMOVE);
                            input1[264] = (float)(TrainPos.SIDEMOVE);
                            input1[265] = (float)(TrainPos.SIDEMOVE);
                            input1[266] = (float)(TrainPos.SIDEMOVE);
                            input1[267] = (float)(TrainPos.SIDEMOVE);
                            input1[268] = (float)(TrainPos.SIDEMOVE);
                            input1[269] = (float)(TrainPos.SIDEMOVE);

                            output1[0] = expectedEval;


                            gpuErrchk(cudaMemcpy(dev_in, input1, inputSize * sizeof(float), cudaMemcpyHostToDevice));
                            gpuErrchk(cudaMemcpy(dev_out, output1, outputSize * sizeof(float), cudaMemcpyHostToDevice));

                            FullNetworkFPBP << < Hlayersizes * Hlayersizes / MAX_THREAD_COUNT, threadsPerBlock >> > (dev_in, dev_b, dev_w, dev_out, dev_netout);

                            gpuErrchk(cudaMemcpy(neuralNetworkOutput, dev_netout, sizeof(float), cudaMemcpyDeviceToHost));

                            cout << "cost = " << (float)neuralNetworkOutput[0] << endl;
                        }

                        Moveline.erase(0, spaceIdx + 1);




                        int spaceIdx2 = Moveline.find(' ');
                        if (spaceIdx2 > 0)
                        {
                            string move2 = Moveline.substr(0, spaceIdx2);

                            if (move2 == "--")
                            {
                                endline = true;
                            }
                            else
                            {
                                TrainPos.DoMoveByString(move2);

                                TrainPos.SIDEMOVE = !TrainPos.SIDEMOVE;


                                if (dist6(rng) == 2 || (((moveIIDX >> 2) + 1) > dist6(rng)))
                                {
                                    for (int i = 0; i < 64; i++)
                                    {
                                        unsigned int indexx = i % 8;
                                        unsigned int indexy = i >> 3;

                                        unsigned int pieceIdx = TrainPos.getPieceID(indexx, indexy);
                                        if (pieceIdx > 7) pieceIdx++;
                                        input1[i * 4] = (float)(pieceIdx & 1);
                                        input1[i * 4 + 1] = (float)((pieceIdx & 2) >> 1);
                                        input1[i * 4 + 2] = (float)((pieceIdx & 4) >> 2);
                                        input1[i * 4 + 3] = (float)((pieceIdx & 8) >> 3);
                                    }

                                    vector<vector<vector<int>>> moves = TrainPos.AllPieceMoves();
                                    if (moves.size() == 0)
                                    {
                                        expectedEval = -40 + 80 * TrainPos.SIDEMOVE;
                                    }
                                    else if (moves[0].size() == 1)
                                    {
                                        expectedEval = 0;
                                    }
                                    else
                                    {
                                        expectedEval = TrainPos.giveSimpleEval();
                                    }

                                    output1[0] = expectedEval;

                                    input1[256] = (float)(TrainPos.PreviousMove[0] & 1);
                                    input1[256 + 1] = (float)((TrainPos.PreviousMove[0] & 2) >> 1);
                                    input1[256 + 2] = (float)((TrainPos.PreviousMove[0] & 4) >> 2);
                                    input1[256 + 3] = (float)(TrainPos.PreviousMove[1] & 1);
                                    input1[256 + 4] = (float)((TrainPos.PreviousMove[1] & 2) >> 1);
                                    input1[256 + 5] = (float)((TrainPos.PreviousMove[1] & 4) >> 2);

                                    input1[262] = (float)(TrainPos.SIDEMOVE);
                                    input1[263] = (float)(TrainPos.SIDEMOVE);
                                    input1[264] = (float)(TrainPos.SIDEMOVE);
                                    input1[265] = (float)(TrainPos.SIDEMOVE);
                                    input1[266] = (float)(TrainPos.SIDEMOVE);
                                    input1[267] = (float)(TrainPos.SIDEMOVE);
                                    input1[268] = (float)(TrainPos.SIDEMOVE);
                                    input1[269] = (float)(TrainPos.SIDEMOVE);

                                    gpuErrchk(cudaMemcpy(dev_in, input1, inputSize * sizeof(float), cudaMemcpyHostToDevice));
                                    gpuErrchk(cudaMemcpy(dev_out, output1, outputSize * sizeof(float), cudaMemcpyHostToDevice));

                                    FullNetworkFPBP << < 1, threadsPerBlock >> > (dev_in, dev_b, dev_w, dev_out, dev_netout);

                                    gpuErrchk(cudaMemcpy(neuralNetworkOutput, dev_netout, sizeof(float), cudaMemcpyDeviceToHost));

                                    cout << "cost = " << (float)neuralNetworkOutput[0] << endl;
                                }
                            }
                        }
                        else
                        {
                            endline = true;
                        }


                        int founddot = Moveline.find('.');
                        if (founddot > 0) Moveline.erase(0, founddot + 1);
                        else endline = true;
                    }
                    else
                    {
                        endline = true;
                    }
                    moveIIDX++;
                }
                idx++;
                tempIdx++;


                if (tempIdx > 2000)
                {
                    gpuErrchk(cudaMemcpy(Biases, dev_b, (HlayerNumber* Hlayersizes + outputSize) * sizeof(float), cudaMemcpyDeviceToHost));
                    gpuErrchk(cudaMemcpy(Weights, dev_w, ((HlayerNumber - 1)* Hlayersizes* Hlayersizes + inputSize * Hlayersizes + outputSize * Hlayersizes) * sizeof(float), cudaMemcpyDeviceToHost));




                    fstream newFile("AINNData.bin", ios::binary | ios::in | ios::out | ios::trunc);
                    if (newFile.is_open())
                    {
                        cout << "opened file";
                        newFile.write((char*)Biases, (HlayerNumber * Hlayersizes + outputSize) * sizeof(float));
                        newFile.seekg((HlayerNumber * Hlayersizes + outputSize) * sizeof(float));
                        newFile.write((char*)Weights, ((HlayerNumber - 1) * Hlayersizes * Hlayersizes + inputSize * Hlayersizes + outputSize * Hlayersizes) * sizeof(float));
                        newFile.close();
                    }
                    else
                    {
                        cout << "error while opening file";
                    }
                    tempIdx = 0;
                }
            }
            Movesfromgame.close();
        }
        else
        {
            cout << "either file isnt open";


        }
    }

    gpuErrchk(cudaMemcpy(Biases, dev_b, (HlayerNumber * Hlayersizes + outputSize) * sizeof(float), cudaMemcpyDeviceToHost));
    gpuErrchk(cudaMemcpy(Weights, dev_w, ((HlayerNumber - 1) * Hlayersizes * Hlayersizes + inputSize * Hlayersizes + outputSize * Hlayersizes) * sizeof(float), cudaMemcpyDeviceToHost));




    fstream newFile("AINNData.bin", ios::binary | ios::in | ios::out | ios::trunc);
    if (newFile.is_open())
    {
        newFile.write((char*)Biases, (HlayerNumber * Hlayersizes + outputSize) * sizeof(float));
        newFile.seekg((HlayerNumber * Hlayersizes + outputSize) * sizeof(float));
        newFile.write((char*)Weights, ((HlayerNumber - 1) * Hlayersizes * Hlayersizes + inputSize * Hlayersizes + outputSize * Hlayersizes) * sizeof(float));
        newFile.close();
    }
    else
    {
        cout << "error while opening file";
    }

    free(input1);
    free(Weights);
    free(Biases);
    free(output1);

    cudaFree(dev_out);
    cudaFree(dev_b);
    cudaFree(dev_w);
    cudaFree(dev_in);
    cudaFree(dev_netout);
    return 0;
}
//hi