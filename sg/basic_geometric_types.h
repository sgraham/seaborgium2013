// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SG_BASIC_GEOMETRIC_TYPES_H_
#define SG_BASIC_GEOMETRIC_TYPES_H_

class Point;

class Rect {
 public:
  Rect() : x(0), y(0), w(-1), h(-1) {}
  Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
  Rect Expand(const Rect& by) const {
    return Rect(x - by.x, y - by.y, w + by.x + by.w, h + by.y + by.w);
  }
  Rect Contract(const Rect& by) const {
    return Rect(x + by.x, y + by.y, w - by.x - by.w, h - by.y - by.h);
  }
  bool Contains(const Point& point) const;
  Rect RelativeTo(const Rect& other) const {
    return Rect(x - other.x, y - other.y, w, h);
  }
  int x, y, w, h;
};

class Point {
 public:
  Point() : x(0), y(0) {}
  Point(int x, int y) : x(x), y(y) {}
  Point Subtract(const Point& other) const {
    return Point(x - other.x, y - other.y);
  }
  Point Scale(float scale) const {
    return Point(x * scale, y * scale);
  }
  int x, y;
};

class Size {
 public:
  Size() : w(0), h(0) {}
  Size(int w, int h) : w(w), h(h) {}
  int w, h;
};

class Color {
 public:
  Color(unsigned char r, unsigned char g, unsigned char b)
      : r(r), g(g), b(b), a(255) {
  }

  Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
      : r(r), g(g), b(b), a(a) {
  }

  unsigned char r, g, b, a;
};

inline bool Rect::Contains(const Point& point) const {
  return point.x >= x &&
         point.x < x + w &&
         point.y >= y &&
         point.y < y + h;
}

#endif  // SG_BASIC_GEOMETRIC_TYPES_H_
