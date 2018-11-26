#include <string>

#include <Utils/Serialize.h>
#include <Utils/String.h>

#include "Catch.hpp"

using namespace AGZ;
using namespace std;

template<> struct BinarySerializeImplementator<Str8>
{
    static bool Serialize(BinarySerializer &serializer, const Str8 &s)
    {
        serializer.Serialize(s.Length());
        return serializer.Write(s.Data(), s.Length() * sizeof(Str8::CodeUnit));
    }
};

template<> struct BinaryDeserializeImplementator<Str8>
{
    static bool Deserialize(BinaryDeserializer &deserializer, Str8 &s)
    {
        size_t len;
        deserializer.Deserialize(len);
        std::vector<Str8::CodeUnit> data(len);
        deserializer.Read(data.data(), len * sizeof(Str8::CodeUnit));
        s = Str8(data.data(), len);
        return deserializer.Ok();
    }
};

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
