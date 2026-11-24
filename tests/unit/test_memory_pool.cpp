#include <catch2/catch_test_macros.hpp>
#include <vyra/core/memory_pool.hpp>
#include <stdexcept>

namespace {
struct PoolProbe {
    explicit PoolProbe(int value) : value(value) {}
    ~PoolProbe() noexcept { ++destructions; }
    int value;
    inline static int destructions = 0;
};
}

TEST_CASE("ObjectPool constructs, reuses and destroys objects", "[memory][pool]") {
    PoolProbe::destructions = 0;
    vyra::core::ObjectPool<PoolProbe, 2> pool;

    auto* first = pool.Create(10);
    auto* second = pool.Create(20);
    REQUIRE(first->value == 10);
    REQUIRE(second->value == 20);
    REQUIRE(pool.Size() == 2);
    REQUIRE(pool.Capacity() == 2);

    REQUIRE(pool.Destroy(first));
    REQUIRE_FALSE(pool.Destroy(first));
    auto* reused = pool.Create(30);
    REQUIRE(reused == first);
    REQUIRE(reused->value == 30);
    REQUIRE(pool.Size() == 2);

    pool.Clear();
    REQUIRE(pool.Size() == 0);
    REQUIRE(PoolProbe::destructions == 3);
}

TEST_CASE("ObjectPool preserves existing objects when construction throws", "[memory][pool]") {
    struct Throwing {
        explicit Throwing(bool shouldThrow) {
            if (shouldThrow) throw std::runtime_error("expected");
        }
    };

    vyra::core::ObjectPool<Throwing, 1> pool;
    REQUIRE_THROWS(pool.Create(true));
    REQUIRE(pool.Size() == 0);
    auto* object = pool.Create(false);
    REQUIRE(object != nullptr);
    REQUIRE(pool.Size() == 1);
}
