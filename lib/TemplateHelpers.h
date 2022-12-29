#ifndef IBROKER_TEMPLATEHELPERS_H
#define IBROKER_TEMPLATEHELPERS_H

#include <type_traits>
#include <tuple>

namespace thisptr
{
  namespace broker
  {
    template <typename T>
    size_t serializeSize(const T& data)
    {
      return sizeof(T);
    }

    template <typename T>
    void serializeBody(const T& data, void* ptr)
    {}

    template <typename T>
    constexpr bool is_pod = std::is_pod<T>::value;

    template <typename T>
    using enable_if_pod = std::enable_if_t<is_pod<T>, bool>;

    template <typename T>
    constexpr bool is_string = std::is_same<std::string, T>::value;

    template <typename T>
    using enable_if_string = std::enable_if_t<is_string<T>, bool>;

    template <typename T>
    constexpr bool has_serializer_size_method = !std::is_same<std::tuple<>, decltype(serializeSize<T>)>::value;

    template <typename T>
    constexpr bool has_serializer_method = !std::is_same<std::tuple<>, decltype(serializeBody<T>)>::value;

    template <typename T>
    using enable_if_has_serializer = std::enable_if_t<!is_pod<T> && !is_string<T> && has_serializer_size_method<T> && has_serializer_method<T>, bool>;

  }
}

#endif //IBROKER_TEMPLATEHELPERS_H
