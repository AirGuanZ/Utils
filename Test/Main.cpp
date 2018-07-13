#include <iostream>
#include <Math\Vec2.h>

int main()
{
    using namespace Math;
    using namespace std;

    Vec2<float> v1(1.5f, 2.0f);
    Vec2<float> v2(2.0f, 3.0f);
    Vec2<float> v3 = Clamp((0.5f + v1) * v2, 1.0f, 5.0f);

    cout << "(" << v3.x << ", " << v3.y << ")" << endl;
}
