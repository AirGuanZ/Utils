#include <AGZUtils/Utils/Serialize.h>

#include "Catch.hpp"
#include "AGZUtils/Misc/TypeOpr.h"

using namespace AGZ;

TEST_CASE("Serializer & Deserializer")
{
    SECTION("MemorySerializer")
    {
        BinaryMemorySerializer s;
        s.Serialize(4);
        s.Serialize(123.0);

        BinaryMemoryDeserializer ds(s.GetData(), s.GetSize());
        REQUIRE(ds.Deserialize<int>()    == std::make_optional(4));
        REQUIRE(ds.Deserialize<double>() == std::make_optional(123.0));
        REQUIRE(ds.End());
    }

    SECTION("Vector")
    {
        std::variant<int, float, std::string> v0 = std::string("abc");
        std::variant<int, float, std::string> v1 = 2;

        std::vector<float> vec = { 1.0f, 2.0f, 3.0f, 4.0f };

        BinaryMemorySerializer serializer;
        serializer.Serialize(v0);
        serializer.Serialize(v1);
        serializer.Serialize(vec);
        serializer.Serialize(vec);

        std::variant<int, float, std::string> v2;
        BinaryMemoryDeserializer deserializer(
            serializer.GetData(), serializer.GetSize());

        deserializer.Deserialize(v2);
        REQUIRE(std::get<std::string>(v2) == "abc");

        deserializer.Deserialize(v2);
        REQUIRE(std::get<int>(v2) == 2);

        vec.clear();
        deserializer.Deserialize(vec);
        REQUIRE(vec == std::vector<float>{ 1.0f, 2.0f, 3.0f, 4.0f });

        vec.clear();
        deserializer.Deserialize(vec);
        REQUIRE(vec == std::vector<float>{ 1.0f, 2.0f, 3.0f, 4.0f });
    }
}
