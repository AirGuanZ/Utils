#include <string>

#include <Utils/Misc.h>
#include <Utils/Serialize.h>
#include <Utils/String.h>

#include "Catch.hpp"

using namespace AGZ;
using namespace std;

struct A
{
    int x = 0;

    bool Serialize(BinarySerializer &s) const
    {
        return s.Serialize(x + 1);
    }
};

TEST_CASE("Serialize")
{
    SECTION("0")
    {
        int x = 16375;
        A a;
        
        BinaryMemorySerializer serializer;
        serializer.Serialize(x);
        serializer.Serialize(Str8("Minecraft"));
        serializer.Serialize(a);

        BinaryMemoryDeserializer deserializer(
            serializer.GetData(), serializer.GetByteSize());
        int dx;
        deserializer.Deserialize(dx);
        Str8 ds;
        deserializer.Deserialize(ds);
        deserializer.Deserialize(a);

        REQUIRE(dx == 16375);
        REQUIRE(ds == "Minecraft");
        REQUIRE(a.x == 1);
    }

    SECTION("1")
    {
        using namespace TypeOpr;

        Variant<int, float, Str8> v0 = Str8("abc");
        Variant<int, float, Str8> v1 = 2;

        std::vector<float> vec = { 1.0f, 2.0f, 3.0f, 4.0f };

        BinaryMemorySerializer serializer;
        serializer.Serialize(v0);
        serializer.Serialize(v1);
        serializer.Serialize(vec);
        serializer.Serialize(vec);

        Variant<int, float, Str8> v2;
        BinaryMemoryDeserializer deserializer(
            serializer.GetData(), serializer.GetByteSize());

        deserializer.Deserialize(v2);
        REQUIRE(std::get<Str8>(v2) == "abc");

        deserializer.Deserialize(v2);
        REQUIRE(std::get<int>(v2) == 2);

        vec.clear();
        deserializer.Deserialize(vec);
        REQUIRE(vec == std::vector<float>{ 1.0f, 2.0f, 3.0f, 4.0f });

        vec.clear();
        deserializer.Deserialize(vec);
        REQUIRE(vec == std::vector<float>{ 1.0f, 2.0f, 3.0f, 4.0f });
    }

    SECTION("2")
    {
        using namespace TypeOpr;

        using V = Variant<int, Str8>;

        std::vector<V> vec = { 0, "abc", 4, "minecraft" };

        BinaryMemorySerializer serializer;
        serializer.Serialize(vec);
        serializer.Serialize(vec);

        BinaryMemoryDeserializer deserializer(
            serializer.GetData(), serializer.GetByteSize());
        vec.clear();

        vec.clear();
        deserializer.Deserialize(vec);
        REQUIRE(vec == std::vector<V>{ 0, "abc", 4, "minecraft" });

        vec.clear();
        deserializer.Deserialize(vec);
        REQUIRE(vec == std::vector<V>{ 0, "abc", 4, "minecraft" });
    }
}
