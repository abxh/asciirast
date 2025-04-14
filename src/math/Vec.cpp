#include "asciirast/math/types.h"

namespace asciirast::math {

extern template Int
dot(const Vec2Int&, const Vec2Int&);
extern template Int
dot(const Vec3Int&, const Vec3Int&);
extern template Int
dot(const Vec4Int&, const Vec4Int&);
extern template std::uint8_t
dot(const RGBA_8bit&, const RGBA_8bit&);
extern template Float
dot(const Vec2&, const Vec2&);
extern template Float
dot(const Vec3&, const Vec3&);
extern template Float
dot(const Vec4&, const Vec4&);

extern template Int
cross(const Vec2Int&, const Vec2Int&);
extern template Vec3Int
cross(const Vec3Int&, const Vec3Int&);
extern template Float
cross(const Vec2&, const Vec2&);
extern template Vec3
cross(const Vec3&, const Vec3&);

extern template Float
angle(const Vec2&, const Vec2&);
extern template Float
angle(const Vec3&, const Vec3&, const Vec3&, bool);

extern template Vec2
lerp(const Vec2&, const Vec2&, const Float t);
extern template Vec3
lerp(const Vec3&, const Vec3&, const Float t);
extern template Vec4
lerp(const Vec4&, const Vec4&, const Float t);

extern template Vec2Int
max(const Vec2Int&, const Vec2Int&);
extern template Vec3Int
max(const Vec3Int&, const Vec3Int&);
extern template Vec4Int
max(const Vec4Int&, const Vec4Int&);
extern template RGBA_8bit
max(const RGBA_8bit&, const RGBA_8bit&);
extern template Vec2
max(const Vec2&, const Vec2&);
extern template Vec3
max(const Vec3&, const Vec3&);
extern template Vec4
max(const Vec4&, const Vec4&);

extern template Vec2Int
min(const Vec2Int&, const Vec2Int&);
extern template Vec3Int
min(const Vec3Int&, const Vec3Int&);
extern template Vec4Int
min(const Vec4Int&, const Vec4Int&);
extern template RGBA_8bit
min(const RGBA_8bit&, const RGBA_8bit&);
extern template Vec2
min(const Vec2&, const Vec2&);
extern template Vec3
min(const Vec3&, const Vec3&);
extern template Vec4
min(const Vec4&, const Vec4&);

extern template Vec2Int
clamp(const Vec2Int&, const Vec2Int&, const Vec2Int&);
extern template Vec3Int
clamp(const Vec3Int&, const Vec3Int&, const Vec3Int&);
extern template Vec4Int
clamp(const Vec4Int&, const Vec4Int&, const Vec4Int&);
extern template RGBA_8bit
clamp(const RGBA_8bit&, const RGBA_8bit&, const RGBA_8bit&);
extern template Vec2
clamp(const Vec2&, const Vec2&, const Vec2&);
extern template Vec3
clamp(const Vec3&, const Vec3&, const Vec3&);
extern template Vec4
clamp(const Vec4&, const Vec4&, const Vec4&);

extern template Vec2Int
abs(const Vec2Int&);
extern template Vec3Int
abs(const Vec3Int&);
extern template Vec4Int
abs(const Vec4Int&);
extern template RGBA_8bit
abs(const RGBA_8bit&);
extern template Vec2
abs(const Vec2&);
extern template Vec3
abs(const Vec3&);
extern template Vec4
abs(const Vec4&);

extern template Vec2
round(const Vec2&);
extern template Vec3
round(const Vec3&);
extern template Vec4
round(const Vec4&);

extern template Vec2
ceil(const Vec2&);
extern template Vec3
ceil(const Vec3&);
extern template Vec4
ceil(const Vec4&);

extern template Vec2
floor(const Vec2&);
extern template Vec3
floor(const Vec3&);
extern template Vec4
floor(const Vec4&);

};
