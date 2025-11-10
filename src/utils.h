#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <tuple>
#include <vector>

inline int correctModulo(int i, int n) { return (i % n + n) % n; }

template <typename T>
inline T normalizeRange(T inMin, T inMax, T v)
{
    const T d = inMax - inMin;
    return (d == (T)0) ? (v >= inMax ? (T)1 : (T)0) : (v - inMin) / d;
}

template <typename T>
inline T normalizeRangeClamped(T inMin, T inMax, T v)
{
    T r = normalizeRange(inMin, inMax, v);
    return r < 0 ? 0 : r > 1 ? 1
                             : r;
}

template <typename T>
inline T remapRangeClamped(T inMin, T inMax, T outMin, T outMax, T v)
{
    const T r = normalizeRangeClamped(inMin, inMax, v);
    return outMin * (1 - r) + outMax * r;
}

template <typename StructType, typename FloatType, FloatType StructType::*member>
static std::tuple<int, int> BinarySearchFindBounds(
    const std::vector<StructType>& arr, FloatType target)
{
    // if (!arr.size()) return { -1, -1 };

    int leftIndex = 0;
    int rightIndex = (int)arr.size() - 1;

    if (target <= arr[0].*member)
        return { 0, 0 };
    if (target >= arr[rightIndex].*member)
        return { rightIndex, rightIndex };

    while (leftIndex <= rightIndex) {
        int midIndex = leftIndex + (rightIndex - leftIndex) / 2;

        if (arr[midIndex].*member == target) {
            return { midIndex, midIndex };
        } else if (arr[midIndex].*member < target) {
            leftIndex = midIndex + 1;
        } else {
            rightIndex = midIndex - 1;
        }
    }

    return { std::max(leftIndex, 0), std::min(rightIndex, (int)arr.size() - 1) };
}
template <typename FloatType>
static std::tuple<int, int> BinarySearchFindBounds(
    const FloatType* arr, const int arrNum, const int strideWidth,
    const int initialOffset, FloatType target)
{
    // if (!arr.size()) return { -1, -1 };
#define GET_ARR(x) arr[x * strideWidth + initialOffset]
    int leftIndex = 0;
    int rightIndex = (int)arrNum - 1;

    if (target <= GET_ARR(0))
        return { 0, 0 };
    if (target >= GET_ARR(rightIndex))
        return { rightIndex, rightIndex };

    while (leftIndex <= rightIndex) {
        int midIndex = leftIndex + (rightIndex - leftIndex) / 2;

        if (GET_ARR(midIndex) == target) {
            return { midIndex, midIndex };
        } else if (GET_ARR(midIndex) < target) {
            leftIndex = midIndex + 1;
        } else {
            rightIndex = midIndex - 1;
        }
    }
#undef GET_ARR
    return { std::max(leftIndex, 0), std::min(rightIndex, (int)arrNum - 1) };
}

template <typename StructType, typename FloatType>
struct InterpData {
    const StructType* array;
    int i0, i1;
    FloatType param;

    InterpData(const StructType* _array, int _i0, int _i1, FloatType _param)
        : array(_array)
        , i0(_i0)
        , i1(_i1)
        , param(_param)
    {
    }

    template <FloatType StructType::*MemberPtr>
    FloatType GetValue()
    {
        const auto v0 = array[i0].*MemberPtr, v1 = array[i1].*MemberPtr;
        return v0 + (v1 - v0) * param;
    }

    bool isValid() const { return !!array; }
};

template <typename StructType, typename FloatType, FloatType StructType::*MemberPtr>
static InterpData<StructType, FloatType> GetInterpData(const std::vector<StructType>& array, FloatType val)
{
    static_assert(std::is_floating_point<FloatType>(), "FloatType must be float");
    if (!array.size())
        return InterpData<StructType, FloatType>(nullptr, 0, 0, 0.0);

    auto range = BinarySearchFindBounds<StructType, FloatType, MemberPtr>(array, val);
    const int index0 = std::get<0>(range), index1 = std::get<1>(range);

    FloatType param = normalizeRangeClamped(array[index0].*MemberPtr, array[index1].*MemberPtr, val);
    return InterpData<StructType, FloatType>(array.data(), index0, index1, param);
}

#endif // UTILS_H
