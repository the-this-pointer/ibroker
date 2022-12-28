#ifndef IBROKER_TEMPLATEHELPERS_H
#define IBROKER_TEMPLATEHELPERS_H

#include <type_traits>
#include <tuple>

namespace thisptr
{
  namespace broker
  {

    template <typename T>
    struct serializer {
      size_t serializeSize(const T& data)
      {
        return 0;
      }

      void serialize(const T& data, void* buf)
      {
      }
    };

    template <typename T>
    constexpr bool is_pod = std::is_pod<T>::value;

    template <typename T>
    using enable_if_pod = std::enable_if_t<is_pod<T>, bool>;

    template <typename T>
    constexpr bool is_string = std::is_same<std::string, T>::value;

    template <typename T>
    using enable_if_string = std::enable_if_t<is_string<T>, bool>;

    template <typename T>
    constexpr bool has_serializer = std::is_default_constructible<serializer<T>>::value;

    template <typename T>
    using enable_if_has_serializer = std::enable_if_t<!is_pod<T> && !is_string<T> && has_serializer<T>, bool>;

  }
}

#endif //IBROKER_TEMPLATEHELPERS_H
