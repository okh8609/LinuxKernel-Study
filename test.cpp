#include <iostream>
#include <vector>
using namespace std;

int main(void)
{
    vector<int> vec;

    for (size_t i = 0; i < 10; i++)
        vec.push_back(i + 1);

    for (int k : vec)
        cout << k << endl;
}