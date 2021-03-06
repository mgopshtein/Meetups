#include <random>
#include <vector>
#include <iostream>

#include <boost/chrono.hpp>
#include <boost/timer/timer.hpp>

#if defined(_DEBUG)
    constexpr int DATA_SIZE = 1024 * 1024;
#else
    constexpr int DATA_SIZE = 8 * 1024 * 1024;
#endif

std::vector<float> data1(DATA_SIZE), data2(DATA_SIZE), resData(DATA_SIZE);

void regularCode()
{
    const float factor = 6.4f;
    for (std::size_t i = 0; i < DATA_SIZE; ++i)
    {
        resData[i] += std::round(factor * std::sqrt(data1[i] * data1[i] + data2[i] * data2[i]));
    }
}

void intrinsicsSseCode()
{
    const __m128 cnstVal = _mm_set_ps1(6.4f);
    for (std::size_t i = 0; i < DATA_SIZE; i += 4)
    {
        __m128 i1 = _mm_load_ps(&data1[i]);
        __m128 i2 = _mm_load_ps(&data2[i]);
        __m128 sqrI1 = _mm_mul_ps(i1, i1);
        __m128 sqrI2 = _mm_mul_ps(i2, i2);
        __m128 sumSqr = _mm_add_ps(sqrI1, sqrI2);
        __m128 sqrtSumSqr = _mm_sqrt_ps(sumSqr);
        __m128 compute = _mm_round_ps(_mm_mul_ps(cnstVal, sqrtSumSqr), _MM_ROUND_MODE_NEAREST);
        __m128 prevRes = _mm_load_ps(&resData[i]);
        _mm_store_ps(&resData[i], _mm_add_ps(prevRes, compute));
    }
}

void intrinsicsAvxCode()
{
    const __m256 cnstVal = _mm256_set1_ps(6.4f);
    for (std::size_t i = 0; i < DATA_SIZE; i += 8)
    {
        __m256 i1 = _mm256_load_ps(&data1[i]);
        __m256 i2 = _mm256_load_ps(&data2[i]);
        __m256 sqrI1 = _mm256_mul_ps(i1, i1);
        __m256 sqrI2 = _mm256_mul_ps(i2, i2);
        __m256 sumSqr = _mm256_add_ps(sqrI1, sqrI2);
        __m256 sqrtSumSqr = _mm256_sqrt_ps(sumSqr);
        __m256 compute = _mm256_round_ps(_mm256_mul_ps(cnstVal, sqrtSumSqr), _MM_ROUND_MODE_NEAREST);
        __m256 prevRes = _mm256_load_ps(&resData[i]);
        _mm256_store_ps(&resData[i], _mm256_add_ps(prevRes, compute));
    }
}

void prepareData()
{
    std::mt19937 gen;
    std::uniform_real_distribution<float> dist(-5.3f, 7.2f);
    std::generate(data1.begin(), data1.end(), [&]() { return dist(gen); });
    std::generate(data2.begin(), data2.end(), [&]() { return dist(gen); });
}


int main()
{
    prepareData();
    regularCode(); //warmup...

    {
        std::cout << "Regular code" << std::endl;
        boost::timer::auto_cpu_timer t;
        regularCode();
    }
    {
        std::cout << "SSE code" << std::endl;
        boost::timer::auto_cpu_timer t;
        intrinsicsSseCode();
    }
    {
        std::cout << "AVX code" << std::endl;
        boost::timer::auto_cpu_timer t;
        intrinsicsAvxCode();
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int<> dist(0, DATA_SIZE-1);

    return static_cast<int>(dist(gen));
}

