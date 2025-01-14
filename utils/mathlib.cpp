#include "mathlib.h"

std::ostream& ZMath::operator<<(std::ostream& out, ZMath::t_float2& v)
{
  out << "[" + std::to_string(v.x) + ", " + std::to_string(v.y) + "]";
  return out;
}

std::ostream& ZMath::operator<<(std::ostream& out, ZMath::t_float3& v)
{
  out << "[" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + "]";
  return out;
}

std::ostream& ZMath::operator<<(std::ostream& out, ZMath::t_float4& v)
{
  out << "[" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ", " + std::to_string(v.w) + "]";
  return out;
}

std::ostream& ZMath::operator<<(std::ostream& out, ZMath::Matrix& m)
{
  out << "[";
  for (size_t i = 0; i < 16; i++)
  {
    out << std::to_string(m.mv[i]);

    // Only add "," when not at the last value
    if (i != 15)
      out << ", ";
  }
  out << "]";
  return out;
}
