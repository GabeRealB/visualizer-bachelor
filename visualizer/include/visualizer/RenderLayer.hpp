#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace Visualizer {

struct RenderLayer {
    constexpr RenderLayer() = default;
    constexpr RenderLayer(std::uint64_t layerMask);

    constexpr static RenderLayer all();
    constexpr static RenderLayer layer(std::size_t layer);

    constexpr operator bool() const;
    constexpr bool operator&&(const RenderLayer& layer) const;
    constexpr bool operator||(const RenderLayer& layer) const;

    constexpr RenderLayer operator&(const RenderLayer& layer) const;
    constexpr RenderLayer operator|(const RenderLayer& layer) const;
    constexpr RenderLayer operator^(const RenderLayer& layer) const;
    constexpr RenderLayer operator~() const;

    constexpr RenderLayer& operator&=(const RenderLayer& layer);
    constexpr RenderLayer& operator|=(const RenderLayer& layer);
    constexpr RenderLayer& operator^=(const RenderLayer& layer);

    std::uint64_t m_layerMask;
};

constexpr RenderLayer::RenderLayer(std::uint64_t layerMask)
    : m_layerMask{ layerMask }
{
}

constexpr RenderLayer RenderLayer::all() { return { std::numeric_limits<decltype(m_layerMask)>::max() }; }

constexpr RenderLayer RenderLayer::layer(std::size_t layer)
{
    return layer >= 64 ? RenderLayer{} : RenderLayer{ 1u << layer };
}

constexpr RenderLayer::operator bool() const { return m_layerMask != 0; }

constexpr bool RenderLayer::operator&&(const RenderLayer& layer) const { return m_layerMask && layer.m_layerMask; }

constexpr bool RenderLayer::operator||(const RenderLayer& layer) const { return m_layerMask || layer.m_layerMask; }

constexpr RenderLayer RenderLayer::operator&(const RenderLayer& layer) const
{
    return { m_layerMask & layer.m_layerMask };
}

constexpr RenderLayer RenderLayer::operator|(const RenderLayer& layer) const
{
    return { m_layerMask | layer.m_layerMask };
}

constexpr RenderLayer RenderLayer::operator^(const RenderLayer& layer) const
{
    return { m_layerMask ^ layer.m_layerMask };
}

constexpr RenderLayer RenderLayer::operator~() const { return { ~m_layerMask }; }

constexpr RenderLayer& RenderLayer::operator&=(const RenderLayer& layer)
{
    m_layerMask &= layer.m_layerMask;
    return *this;
}

constexpr RenderLayer& RenderLayer::operator|=(const RenderLayer& layer)
{
    m_layerMask |= layer.m_layerMask;
    return *this;
}

constexpr RenderLayer& RenderLayer::operator^=(const RenderLayer& layer)
{
    m_layerMask ^= layer.m_layerMask;
    return *this;
}

}