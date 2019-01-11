# AGZ Utils

自用C++基础组件库，保持使用最新的C++标准。

- [x] Math：图形学常用的数学工具
- [x] Range：范围对象，预计在C++20后弃用
- [x] String：不可变字符串，支持多种编码
- [x] Regex：正则表达式库，拥有我比较喜欢的特性
- [x] Texture：纹理加载、保存和采样，以及各种环境映射
- [x] Arena：小对象池
- [x] FileSys：路径类，磁盘缓存管理器等
- [x] Config：配置文件解析工具
- [x] WavefrontObj：OBJ文件解析器
- [x] Serialize：二进制序列化和反序列化工具
- [x] Thread：静态任务分派器
- [x] Others：字节序管理，COW对象，常用TMP工具，单件模板，时钟类……

## Usage

AGZ Utils仅包含头文件，使用时需在项目中的某一个.cc/cpp文件中定义以下宏后再包含`Utils.h`：

```cpp
#define AGZ_ALL_IMPL
```

