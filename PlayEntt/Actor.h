#pragma once 

#include <concepts>

//enum class Entity : uint32_t {};


#include <cstdint>
#include <compare>

class [[nodiscard]] Entity {
    uint32_t id;

public:
    // Constants
    static constexpr uint32_t index_bits = 24;
    static constexpr uint32_t version_bits = 8;
    static constexpr uint32_t index_mask = (1u << index_bits) - 1;
    static constexpr uint32_t version_mask = (1u << version_bits) - 1;
    static constexpr uint32_t null_id = ~0u;

    // Constructors
    constexpr Entity() : id(null_id) {}
    explicit constexpr Entity(uint32_t id) : id(id) {}

    // Factory method to create an Entity from index and version
    static constexpr Entity create(uint32_t index, uint32_t version) {
        return Entity((version << index_bits) | index);
    }

    // Accessors
    constexpr uint32_t index() const { return id & index_mask; }
    constexpr uint32_t version() const { return (id >> index_bits) & version_mask; }

    // Comparison operators
    constexpr auto operator<=>(const Entity& other) const = default;

    // Check if the entity is null
    constexpr bool is_null() const { return id == null_id; }

    // Get the raw ID
    constexpr uint32_t raw_id() const { return id; }
};






template<typename T>
concept EntityType = std::same_as<T, Entity>;