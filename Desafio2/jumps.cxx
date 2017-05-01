#include <iostream>
#include <climits>
#include <algorithm>    // std::min


int minJumps(int arr[], int n)
{
    int *jumps = new int[n];  // jumps[n-1] will hold the result
    int i, j;

    if (n == 0 || arr[0] == 0)
        return INT_MAX;

    jumps[0] = 0;

    // Find the minimum number of jumps to reach arr[i]
    // from arr[0], and assign this value to jumps[i]
    for (i = 1; i < n; i++)
    {
        jumps[i] = INT_MAX;
        for (j = 0; j < i; j++)
        {
            if (i <= j + arr[j] && jumps[j] != INT_MAX)
            {
                 jumps[i] = std::min(jumps[i], jumps[j] + 1);
                 break;
            }
        }
    }
    return jumps[n-1];
}


// Driver program to test above function
int main()
{
    int arr[] = {1, 2, 2, 0, 12, 1, 0, 9};
    int size = sizeof(arr)/sizeof(int);
    std::cout << ("Minimum number of jumps to reach end is %d \n", minJumps(arr,size));
    return 0;
}
