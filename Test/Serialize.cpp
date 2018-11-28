#include <string>

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
