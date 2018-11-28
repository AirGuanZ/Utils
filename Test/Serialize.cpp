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

        BinaryMemorySerializer serializer;
        serializer.Serialize(v0);
        serializer.Serialize(v1);

        Variant<int, float, Str8> v2;
        BinaryMemoryDeserializer deserializer(
            serializer.GetData(), serializer.GetByteSize());
        deserializer.Deserialize(v2);
        REQUIRE(std::get<Str8>(v2) == "abc");
        deserializer.Deserialize(v2);
        REQUIRE(std::get<int>(v2) == 2);
    }
}
