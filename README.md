## Math

Basic mathematica operations. Mainly used for graphics programming.

### Scalar

```c++
Abs(n)                  // |n|
Sqrt(n)                 // sqrt(n)
Clamp(n, minv, maxv)    // min(maxv, max(minv, n))
ApproxEq(lhs, rhs, eps) // |lhs - rhs| <= eps

Radf pi_rad = PI<Radf>();     // Pi in (float) radian
Degd pi_deg = PI<Degd>();     // Pi in (double) degree
Degf hpi_rad = pi_rad / 2.f;  // Pi/2 in (float) radian
float sin_hpi = Sin(hpi_rad); // Common trigonometric functions
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
Mat4f inv_rotxy = Inverse(rotate_xy);                      // Inverse matrix
//... Common graphics-used matrix
```

### Random

Simplify using of random components in standard library.

```c++
int x    = Uniform(1, 10);       // Sample an integer uniformly in [1, 10]
float y  = Uniform(-10.f, 10.f); // Sample a float uniformly in [-10, 10]
double z = Normal(1.0, 4.0);     // Sample a double with normal distribution
                                 // mean = 1, stddev = 4.0
```

### SIMD

Alternative to `Vec4f` with SSE/SSE2 acceleration.

```c++
f32x4 a = f32x4(1.0f, 2.0f, 3.0f, 4.0f);        // From floats
f32x4 b = f32x4(Vec4f(2.0f, 3.0f, 4.0f, 5.0f)); // From Vec4f
f32x4 c = Sqrt(a + b);
Vec4f d = c.AsVec(); // d: (sqrt(3), sqrt(5), sqrt(7), sqrt(9))
```

## Buffer

```c++
// Initialize buf with { 0, 1, 2, 3, ..., 7, 8, 9 }
auto buf0 = Buffer<int>::FromFn(10, [](size_t i) { return i; });
// Map buf to buf1 with x -> sqrt(x)
auto buf1 = buf0.Map<float>([](int s) { return Sqrt(float(s)); });
// Random access with element index
float elem_at_2 = buf1(2);
// Foldl
float sum = buf1.Foldl(0.0f, [](float a, float b) { return a + b; });

// Buffer2D is similar
auto buf20 = Buffer2D<int>::New(100, 100);
auto buf21 = Buffer2D<int>::FromFn(
    100, 100, [](size_t x, size_t y){ return int(x * y); });
```

## Endian

```c++
if constexpr(IS_LITTLE_ENDIAN)
    ...
else
    ...

// Assume using little endian
uint32_t v1 = Native2Big((uint32_t)0x12345678);
assert(v1 == 0x78563412);
assert(v1 == Native2Little(v1)); // Native-to-native calling does nothing
assert(Big2Little(v1) == 0x12345678);
```

## Range

Between, Collect, Drop, Filter, Map, PartialFoldl, Reverse, Sequence, Take...

```cpp
vector<int> v;
for(auto i : Seq<int>(1) | Take(5))
    v.push_back(i);
REQUIRE(v == vector<int>{ 1, 2, 3, 4, 5 });

v.clear();
auto Square = [](int v) { return v * v; };
for(auto i : Between(1, 6) | Map(Square))
    v.push_back(i);
REQUIRE(v == vector<int>{ 1, 4, 9, 16, 25 });

auto addInt = [](int a, int b) { return a + b; };
REQUIRE((Between(1, 4) | Reduce(0, addInt)) == 1 + 2 + 3);

v.clear();
auto IsEven = [](int v) { return v % 2 == 0; };
for(auto i : Between(1, 7) | Filter(IsEven))
    v.push_back(i);
REQUIRE(v == vector<int>{ 2, 4, 6 });

REQUIRE((Between(0, 100) | Count()) == 100);
REQUIRE((Between(0, 100) | CountIf(IsEven)) == 50);

v.clear();
Seq(1) | Take(10) | Each([&](int i) { v.push_back(i); });
REQUIRE(v == vector<int>{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 });

auto isLessThan10 = [](int v) { return v < 10; };
REQUIRE((Seq(1) | Drop(2) | Take(5) | Collect<vector<int>>())
     == vector<int>{ 3, 4, 5, 6, 7 });
REQUIRE((Seq(1) | DropWhile(isLessThan10) | Take(5) | Collect<set<int>>())
     == set<int>{ 10, 11, 12, 13, 14 });

REQUIRE((Between(1, 6) | PartialFoldl(0, addInt) | Collect<unordered_set<int>>())
     == unordered_set<int>{ 1, 3, 6, 10, 15 });

REQUIRE((Between(1, 6) | Reverse() | Collect<list<int>>())
     == list<int>{ 5, 4, 3, 2, 1 });

REQUIRE((Seq(1) | TakeWhile(isLessThan10) | Collect<vector<int>>())
     == vector<int>{ 1, 2, 3, 4, 5, 6, 7, 8, 9 });
```

## String

A non-null-terminated immutable string class...Just for fun.

- Small string optimization & reference counting
- Various char encoding

```cpp
// using Str8 = String<UTF8<char>>; using Str32 = String<UTF32<uint32_t>>;

Str8 a = u8"今天天气不错。Hello, world!";
REQUIRE(a == u8"今天天气不错。Hello, world!");

// Fiexiable convertion between different char encodings
Str32 b = CSConv::Convert<UTF32<>>(a);
REQUIRE(b == u8"今天天气不错。Hello, world!");

// Traversal code points easily
for(char32_t codePoint : a)
    ;
// Typeof c is StringView<UTF8<>>.
// Each c contains one single code point (perhaps multiple code units).
for(auto c : a.Chars())
    ;

// Common string operations
REQUIRE(Str8::From(0xFF35B, 16) == u8"FF35B")
REQUIRE(Str32::From(0b1010110, 2) == u8"1010110")
REQUIRE(Str8(u8"  Minecraft  ").Trim() == u8"Minecraft");
REQUIRE(Str8(u8"Minecraft").Slice(0, 3) == u8"Min")
REQUIRE(Str8(u8" + ").Join(vector<Str8>{ u8"a", u8"b", u8"c" }) == u8"a + b + c");
REQUIRE(Str8(u8"Minecraft").Find(u8"necraft") == 2);
//...

// Perfectly compatiable with Range components
REQUIRE((Str8(u8"Mine cr aft ").Split()
        | Map([](const Str8::View &v) { return v.AsString(); })
        | Collect<vector<Str8>>())
     == vector<Str8>{ u8"Mine", u8"cr", u8"aft" });
```

## Regex

- Various char encoding
- Flexible submatches tracking

`abc`: matches `abc`
`abc[def]`: matches `abcd`/`abce`/`abcf`
`abc?`: matches `ab`/`abc`
`abc+`: matches concatenation of `ab` and one or more `c`
`abc*`: matches concatenation of `ab` and zero or more `c`

`&` defines a save point and can be used to indicate a location in matched strings.

```cpp
REQUIRE(Regex8("abc(def)+").Match("abcdefdefdef"));
REQUIRE(Regex16("abc[def]+").Match("abcdddeeefffdef"));
REQUIRE(Regex32("abc").Match("abd") == false);
REQUIRE(Regex8("cde").Search("abcdef"));
REQUIRE(Regex8("cDe").Search("abcdef") == false);

{
    auto m = Regex8("&abc&[def]*&xyz&").Match("abcddeeffxyz");
    REQUIRE((m && m(0, 1) == "abc"
               && m(1, 2) == "ddeeff"
               && m(2, 3) == "xyz"));
}

{
    auto m = Regex8("&[def]+&").Search("abcddeeffxyz");
    REQUIRE((m && m(0, 1) == "ddeeff"));
}
```
