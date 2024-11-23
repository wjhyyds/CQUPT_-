#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>

// CUDA 核函数：向量逐元素相加
__global__ void vectorAddKernel(const int* A, const int* B, int* C, int N) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) {
        C[i] = A[i] + B[i];
    }
}

// CPU 函数：向量逐元素相加
void vectorAddition(const int* A, const int* B, int* C, int N) {
    for (int i = 0; i < N; ++i) {
        C[i] = A[i] + B[i];
    }
}

int main() {
    const int N = 1000000; // 向量长度
    int *h_A, *h_B, *h_C_CPU, *h_C_GPU;
    int *d_A, *d_B, *d_C;
    const int size = N * sizeof(int);

    // 分配 CPU 内存
    h_A = (int*)malloc(size);
    h_B = (int*)malloc(size);
    h_C_CPU = (int*)malloc(size);
    h_C_GPU = (int*)malloc(size);

    // 初始化输入向量 A 和 B
    for (int i = 0; i < N; ++i) {
        h_A[i] = i;
        h_B[i] = 2 * i;
    }

    // 分配 GPU 内存
    cudaMalloc((void**)&d_A, size);
    cudaMalloc((void**)&d_B, size);
    cudaMalloc((void**)&d_C, size);

    // 将输入数据从主机内存复制到 GPU 内存
    cudaMemcpy(d_A, h_A, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, size, cudaMemcpyHostToDevice);

    // 调用 GPU 核函数
    const int threadsPerBlock = 256;
    const int blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;
    vectorAddKernel<<<blocksPerGrid, threadsPerBlock>>>(d_A, d_B, d_C, N);

    // 将结果从 GPU 内存复制到主机内存
    cudaMemcpy(h_C_GPU, d_C, size, cudaMemcpyDeviceToHost);

    // 调用 CPU 函数
    vectorAddition(h_A, h_B, h_C_CPU, N);

    // 检查 GPU 和 CPU 结果是否一致
    for (int i = 0; i < N; ++i) {
        if (h_C_GPU[i] != h_C_CPU[i]) {
            printf("Error: GPU and CPU results differ at index %d\n", i);
            break;
        }
    }

    // 释放内存
    free(h_A);
    free(h_B);
    free(h_C_CPU);
    free(h_C_GPU);
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    return 0;
}

