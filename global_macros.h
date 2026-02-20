#pragma once

#ifndef BOOL_TO_U8
#define BOOL_TO_U8(b) static_cast<std::uint8_t>((b) ? 1u : 0u)
#endif // BOOL_TO_U8

#ifndef TO_BOOL
#define TO_BOOL(val) static_cast<bool>(val)
#endif // TO_BOOL

#ifndef TO_INT
#define TO_INT(val) static_cast<int>(val)
#endif // TO_INT

#ifndef TO_FLOAT
#define TO_FLOAT(val) static_cast<float>(val)
#endif // TO_FLOAT

#ifndef TO_DOUBLE
#define TO_DOUBLE(val) static_cast<double>(val)
#endif // TO_DOUBLE

#ifndef TO_SIZE
#define TO_SIZE(val) static_cast<std::size_t>(val)
#endif // TO_SIZE

#ifndef TO_8
#define TO_8(val) static_cast<std::int8_t>(val)
#endif // TO_8

#ifndef TO_U8
#define TO_U8(val) static_cast<std::uint8_t>(val)
#endif // TO_U8

#ifndef TO_16
#define TO_16(val) static_cast<std::int16_t>(val)
#endif // TO_16

#ifndef TO_U16
#define TO_U16(val) static_cast<std::uint16_t>(val)
#endif // TO_U16

#ifndef TO_32
#define TO_32(val) static_cast<std::int32_t>(val)
#endif // TO_32

#ifndef TO_U32
#define TO_U32(val) static_cast<std::uint32_t>(val)
#endif // TO_U32

#ifndef TO_64
#define TO_64(val) static_cast<std::int64_t>(val)
#endif // TO_64

#ifndef TO_U64
#define TO_U64(val) static_cast<std::uint64_t>(val)
#endif // TO_U64

#ifndef ARRAY_6_FLOAT_SIZE
#define ARRAY_6_FLOAT_SIZE (6 * sizeof(float))
#endif  // ARRAY_6_FLOAT_SIZE

#ifndef ARRAY_16_BOOL_SIZE
#define ARRAY_16_BOOL_SIZE (16 * sizeof(bool))
#endif  // ARRAY_16_BOOL_SIZE
