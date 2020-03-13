#include "xml.h"
#include <iostream>

using namespace std;

int main (void)
{
    Xml xml;
    xml.parse();
    cout << xml;
    return 0;
}
