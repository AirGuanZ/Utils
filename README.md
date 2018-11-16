# AGZ Utils

- [x] Math: common mathematical tools for graphics using
- [x] Range: range objects and transformers. May be deprecated after C++ 20
- [x] String: immutable encoding-aware string
- [x] Regex: regular expression with my favourite features
- [x] Texture: texture loading/saving/sampling
- [x] Arena: fast object pool
- [x] Path: cross-platform file/directory path management
- [x] Config: configuration parsing tool
- [x] WavefrontOBJ: wavefront OBJ file loader
- [x] Others: Endian, COWObject, TypeOpr (for TMP), Singleton, Uncopiable, Platform, Clock...

I'll always keep this library using the latest C++ standard.

## Documentation

Make sure you have [doxygen](http://www.doxygen.nl/) installed and simply enter following command in project directory:

```
doxygen ./Doxyfile
```

## Usage

Add macro

```cpp
#define AGZ_ALL_IMPL
```

in exact one .cpp/.cc file before including `Utils.h` to generate necessary non-public definitions.