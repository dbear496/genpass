

namespace genpass {
template<typename T>
std::size_t serialize(unsigned char *dst, const T src);

template<typename T>
std::size_t deserialize(T& dst, const unsigned char *src)

template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
std::size_t serialize<T>(unsigned char *dst, const T src) {
  for(int i = 0; i < sizeof(T); i++)
    dst[i] = (unsigned char)(src >> (i * 8));
  return sizeof(T);
}

template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
std::size_t deserialize<T>(T& dst, const unsigned char *src) {
  dst = 0;
  for(int i = 0; i < sizeof(T); i++)
    dst |= (T)src[i] << (i * 8);
  return sizeof(T);
}
}
