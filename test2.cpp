#include <iostream>
#include <vector>
using namespace std;

int main(void)
{
    int i = 6;
    switch (i)
    {
    default:
        cout << "default!" << endl;
        break;

    case 5:
        cout << "## " << i << endl;
        break;

    case 6:
        cout << "## " << i << endl;
        break;
    }
}