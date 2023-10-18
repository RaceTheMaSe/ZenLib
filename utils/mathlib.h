#pragma once
#include <iostream>
#include <string>
#include <cstring>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wpedantic" // ISO C++ prohibits anonymous structs
#endif
#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning(disable: 4201) // nonstandard extension used: nameless struct/union
#endif

namespace ZMath
{
  constexpr int64_t ipow(int64_t base, int exp, int64_t result = 1)
  {
    return exp < 1 ? result : ipow(base * base, exp / 2, (exp % 2) ? result * base : result);
  }

  struct t_float2
  {
    t_float2() = default;
    t_float2(float xIn, float yIn)
    {
      this->x = xIn;
      this->y = yIn;
    }

    union {
      struct
      {
          float x;
          float y;
      };

      float v[2]={};
    };

    std::string toString() const
    {
      std::string out;
      out = "[" + std::to_string(x) + ", " + std::to_string(y) + "]";

      return out;
    }

    friend std::ostream& operator<<(std::ostream& out, t_float2& v);
  };

  std::ostream& operator<<(std::ostream& out, t_float2& v);

  struct t_float3
  {
    t_float3() = default;
    t_float3(float xIn, float yIn, float zIn)
    {
      this->x = xIn;
      this->y = yIn;
      this->z = zIn;
    }

      union {
        struct
        {
          float x;
          float y;
          float z;
        };

        float v[3]={};
      };

      std::string toString() const
      {
        std::string out;
        out = "[" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "]";

        return out;
      }

      friend std::ostream& operator<<(std::ostream& out, t_float3& v);
  };

  std::ostream& operator<<(std::ostream& out, t_float3& v);

  struct t_float4
  {
    t_float4() = default;
    t_float4(float xIn, float yIn, float zIn, float wIn)
    {
      this->x = xIn;
      this->y = yIn;
      this->z = zIn;
      this->w = wIn;
    }

    union {
      struct
      {
        float x;
        float y;
        float z;
        float w;
      };

      float v[4]={};
    };

    /**
      * @brief Converts the given ABGR8-Color to float4
      */
    void fromABGR8(uint32_t argb)
    {
      auto a = uint8_t(argb >> 24);
      auto b = uint8_t(argb >> 16);
      auto g = uint8_t(argb >>  8);
      auto r = uint8_t(argb      );

      x = (float)r / 255.0f;
      y = (float)g / 255.0f;
      z = (float)b / 255.0f;
      w = (float)a / 255.0f;
    }

    /**
      * @brief Converts the stored color to ARGB8
      */
    uint32_t toABGR8()
    {
      auto a = uint32_t(w*255.f);
      auto b = uint32_t(z*255.f);
      auto g = uint32_t(y*255.f);
      auto r = uint32_t(x*255.f);
      return (a << 24) | (b << 16) | (g << 8) | r;
    }

    std::string toString() const
    {
      std::string out;
      out = "[" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w) + "]";

      return out;
    }

    friend std::ostream& operator<<(std::ostream& out, t_float4& v);
  };

  std::ostream& operator<<(std::ostream& out, t_float4& v);

  template <typename T, typename... S>
  struct t_vector : public T
  {
    t_vector(S... a)
        : T(a...)
    {
    }
    t_vector() = default;

    // Comparision operators
    bool operator==(const t_vector<T, S...>& w) const
    {
        return memcmp(T::v, w.v, sizeof(w.v)) == 0;
    }

    bool operator!=(const t_vector<T, S...>& vOther) const
    {
        return !(*this == vOther);
    }

    t_vector<T, S...> operator*(float s) const
    {
      t_vector<T, S...> rs;

      for (size_t i = 0; i < sizeof(T) / sizeof(float); i++)  // Fixme: Doesn't work for double-vectors! Also, not very nice solution.
      {
          rs.v[i] = T::v[i] * s;
      }

      return rs;
    }

    t_vector<T, S...>& operator*=(float s)
    {
      *this = (*this) * s;
      return *this;
    }
  };

  using float2 = t_vector<t_float2, float, float>;
  using float3 = t_vector<t_float3, float, float, float>;
  using float4 = t_vector<t_float4, float, float, float, float>;

  //------------------------------------------------------------------------------
  // 4x4 Matrix (assumes right-handed cooordinates)
  struct Matrix
  {
    Matrix() {}

    Matrix(float* pm)
    {
      memcpy((void*)m, pm, sizeof(m));
    }

    Matrix(float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33)
    {
      m[0][0] = m00;
      m[0][1] = m01;
      m[0][2] = m02;
      m[0][3] = m03;

      m[1][0] = m10;
      m[1][1] = m11;
      m[1][2] = m12;
      m[1][3] = m13;

      m[2][0] = m20;
      m[2][1] = m21;
      m[2][2] = m22;
      m[2][3] = m23;

      m[3][0] = m30;
      m[3][1] = m31;
      m[3][2] = m32;
      m[3][3] = m33;
    }

    void Transpose()
    {
      Matrix mm = *this;

      m[0][0] = mm.m[0][0];
      m[0][1] = mm.m[1][0];
      m[0][2] = mm.m[2][0];
      m[0][3] = mm.m[3][0];
      m[1][0] = mm.m[0][1];
      m[1][1] = mm.m[1][1];
      m[1][2] = mm.m[2][1];
      m[1][3] = mm.m[3][1];
      m[2][0] = mm.m[0][2];
      m[2][1] = mm.m[1][2];
      m[2][2] = mm.m[2][2];
      m[2][3] = mm.m[3][2];
      m[3][0] = mm.m[0][3];
      m[3][1] = mm.m[1][3];
      m[3][2] = mm.m[2][3];
      m[3][3] = mm.m[3][3];
    }

    // Properties
    float3 Up() const { return {_21, _22, _23}; }
    void Up(const float3& vIn)
    {
      _21 = vIn.x;
      _22 = vIn.y;
      _23 = vIn.z;
    }

    float3 Down() const { return {-_21, -_22, -_23}; }
    void Down(const float3& vIn)
    {
      _21 = -vIn.x;
      _22 = -vIn.y;
      _23 = -vIn.z;
    }

    float3 Right() const { return {_11, _12, _13}; }
    void Right(const float3& vIn)
    {
      _11 = vIn.x;
      _12 = vIn.y;
      _13 = vIn.z;
    }

    float3 Left() const { return {-_11, -_12, -_13}; }
    void Left(const float3& vIn)
    {
      _11 = -vIn.x;
      _12 = -vIn.y;
      _13 = -vIn.z;
    }

    float3 Forward() const { return {-_31, -_32, -_33}; }
    void Forward(const float3& vIn)
    {
      _31 = -vIn.x;
      _32 = -vIn.y;
      _33 = -vIn.z;
    }

    float3 Backward() const { return {_31, _32, _33}; }
    void Backward(const float3& vIn)
    {
      _31 = vIn.x;
      _32 = vIn.y;
      _33 = vIn.z;
    }

    float3 Translation() const { return {_41, _42, _43}; }
    float3 TranslationT() const { return {_14, _24, _34}; }
    void Translation(const float3& vIn)
    {
      _41 = vIn.x;
      _42 = vIn.y;
      _43 = vIn.z;
    }

    static Matrix CreateIdentity()
    {
      return {1, 0, 0, 0,
              0, 1, 0, 0,
              0, 0, 1, 0,
              0, 0, 0, 1};
    }

    union {
      struct
      {
          float _11, _12, _13, _14;
          float _21, _22, _23, _24;
          float _31, _32, _33, _34;
          float _41, _42, _43, _44;
      };
      float m[4][4];
      float4 v[4];
      float mv[16];
    };

    std::string toString()
    {
      std::string out;
      out = "[";
      for (size_t i = 0; i < 16; i++)
      {
        out += std::to_string(mv[i]);

        // Only add "," when not at the last value
        if (i != 15)
            out += ", ";
      }
      out += "]";

      return out;
    }

    friend std::ostream& operator<<(std::ostream& out, Matrix& v);
  };

  std::ostream& operator<<(std::ostream& out, Matrix& m);
}  // namespace ZMath

#if defined(_MSC_VER)
#pragma warning( pop )
#endif
