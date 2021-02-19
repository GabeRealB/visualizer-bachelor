#pragma once

#include <cstddef>

#ifdef _MSC_VER
#include <malloc.h>
#else
#include <cstdlib>
#endif

namespace Visualizer {

template <typename T> struct AlignedDeleter {
    using pointer_type = T*;

    static pointer_type allocate(std::size_t alignment, std::size_t size);
    void operator()(pointer_type ptr);
};

template <typename T>
typename AlignedDeleter<T>::pointer_type AlignedDeleter<T>::allocate(std::size_t alignment, std::size_t size)
{
    // POSIX systems require that the alignment is a multiple of sizeof(void*).
    if (alignment % sizeof(void*) != 0) {
        alignment += sizeof(void*) - (alignment % sizeof(void*));
    }
    // The size must be a multiple of the alignment.
    if (size % alignment != 0) {
        size += alignment - (size % alignment);
    }

#ifdef _MSC_VER
    return static_cast<pointer_type>(_aligned_malloc(size, alignment));
#else
    return static_cast<T*>(std::aligned_alloc(alignment, size));
#endif
}

template <typename T> void AlignedDeleter<T>::operator()(pointer_type ptr)
{
#ifdef _MSC_VER
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}

}