// source: Dillon Huff - dillonhuff@gmail.com - http://www.dillonbhuff.com/?p=30
#include <cassert>
#include <vector>
#include <algorithm>
#include "zenload/zTypes.h"

// specialization for ZenLoad::WorldVertex / ZenLoad::WorldTriangle
template<typename Triangle,typename Vertex=ZenLoad::WorldVertex>
Vertex getVertex(const Triangle& tl, const unsigned& index) {
  assert(index<3);
  return tl.vertices[index];
}

template<typename Triangle,typename Vertex=ZenLoad::WorldVertex>
void setVertex(Triangle& tl, const unsigned& index, const Vertex& vertex) {
  assert(index<3);
  tl.vertices[index] = vertex;
}

template<typename Triangle>
bool shareEdge(const Triangle& tl, const Triangle& tr) {
  int numEqual = 0;
  for (unsigned i = 0; i < 3; i++) {
    for (unsigned j = 0; j < 3; j++) {
      numEqual += (getVertex(tl, i) == getVertex(tr, j)) ? 1 : 0;
    }
  }
  return numEqual > 1;
}

template<typename Triangle>
bool windingConflict(const Triangle& ti, const Triangle& tj) {
  for (unsigned l = 0; l < 3; l++) {
    unsigned lp1 = (l + 1) % 3;
    for (unsigned k = 0; k < 3; k++) {
      unsigned kp1 = (k + 1) % 3;
      if (getVertex(ti, k) == getVertex(tj, l) &&
        getVertex(ti, kp1) == getVertex(tj, lp1)) {
        return true;
      }
    }
  }
  return false;
}
 
template<typename Triangle>
Triangle flipWindingOrder(const Triangle& t) {
  Triangle f;
  setVertex(f, 0, getVertex(t, 1));
  setVertex(f, 1, getVertex(t, 0));
  setVertex(f, 2, getVertex(t, 2));
  return f;
}
 
template<typename Triangle>
std::vector<Triangle> flipWindingOrders(const std::vector<Triangle>& vertexTriangles) {
  std::vector<Triangle> tris;
  for (auto t : vertexTriangles) {
    tris.push_back(flipWindingOrder(t));
  }
  return tris;
}
 
template<typename Triangle>
Triangle correctOrientation(const Triangle& toCorrect, const std::vector<Triangle>& others) {
  auto ti = toCorrect;
  for (auto tj : others) {
    if (windingConflict(ti, tj)) {
      Triangle corrected = flipWindingOrder(ti);
      return corrected;
      }
    }
  return ti;
}

template<typename T, typename F>
std::vector<T> select(const std::vector<T>& v, F f) {
  std::vector<T> selected;
  for (auto e : v) {
    if (f(e))
      selected.push_back(e);
  }
  return selected;
}

template<typename E, typename T>
void remove(E e, T& t) {
  t.erase(std::remove(std::begin(t), std::end(t), e), std::end(t));
}

template<typename Triangle, typename Indices>
std::vector<Triangle> fixWindingOrder(const std::vector<Triangle>& triangles, std::vector<Indices>& indices) {
  std::vector<Triangle> tris;
  std::vector<Indices> remainingIndices = indices;
 
  for(auto& tri : triangles)
  {
    std::vector<Triangle> subTris = select(triangles, [tri](const Triangle t) { return shareEdge(tri, t); });
    if (subTris.size() > 0) {
      Triangle corrected = correctOrientation(tri, subTris);
      tris.push_back(corrected);
    }
    if (tris.size() == 0) // lonely triangle
      tris.push_back(tri);
  }
  return tris;
}
