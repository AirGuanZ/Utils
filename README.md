# Utils

自用C++类库，把一些常见操作封装成自己喜欢的风格。

## Math

基本数学操作，主要服务于图形学编程。

### Scalar

```c++
Abs(n)                // |n|
Sqrt(n)               // sqrt(n)
Clamp(n, minv, maxv)  // min(maxv, max(minv, n))
Approx(lhs, rhs, eps) // |lhs - rhs| <= eps
```

### Veci & Mat4

```c++
Vec2<T>, Vec3<T>, Vec4<T> // 2/3/4-demension vector
Mat4<T>                   // 4x4 matrix

Vec3f v1 = ..., v2 = ...;
Vec3f v3 = Cross(2.f * v1, v2);
Vec4f mv = Mat4f(1.f) * Vec4f(1.f, 2.f, 3.f); // Common matrix/vector operations

Vec2f v4 = v3.yz();
Vec3f v5 = v3.xxy(); // Fiexible swizzling

Mat4f identity  = Mat4f(1.f);                              // Identity
Mat4f rotate_xy = Mat4f::Rotate({ 1.f, 2.f }, Degf(90.0)); // Rotation
//... Common graphics-used matrix
```

### Angle

```c++
Radf pi_rad = PI<Radf>();     // Pi in (float) radian
Degd pi_deg = PI<Degd>();     // Pi in (double) degree
Degd hpi_rad = pi_rad / 2.f;  // Pi/2 in (float) radian
float sin_hpi = Sin(hpi_rad); // Common trigonometric functions
```

### Random

我就是单纯地觉得标准库的`<random>`用起来太麻烦……

```c++
int x   = Uniform(1, 10);       // Sample an integer uniformly in [1, 10]
float y = Uniform(-10.f, 10.f); // Sample a float uniformly in [-10, 10]
```

