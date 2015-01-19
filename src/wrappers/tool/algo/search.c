/// -------------------------------------------------
/// search.c : 查找算法实现文件
/// -------------------------------------------------
/// Copyright (c) 2015, Wang Yujia <combo.xy@163.com>
/// All rights reserved.
/// -------------------------------------------------


/// 二分法搜索 - 非递归
int binary_search(int array[], int n, int v)
{
    int left, right, middle;

    left = 0, right = n - 1;

    while (left <= right)
    {
        middle = (left + right) / 2;
        if (array[middle] > v)
        {
            right = middle - 1;
        }
        else if (array[middle] < v)
        {
            left = middle + 1;
        }
        else
        {
            return middle;
        }
    }

    return -1;
}


/// 二分法搜索 - 递归
int binary_search_recurse(int array[], int low, int high, int v)
{
    int middle;

    middle = (low + high) / 2;

    if (low < high)
    {
        if (array[middle] > v)
        {
            return binary_search_recurse(array, low, middle, v);
        }
        else if (array[middle] < v)
        {
            return binary_search_recurse(array, middle + 1, high, v);
        }
        else
        {
            return middle;
        }
    }
    else if (low == high)
    {
        if (array[middle] == v)
        {
            return middle;
        }
        else
        {
            return -1;
        }

    }
    else
    {
        return -1;
    }

    return -1;
}

