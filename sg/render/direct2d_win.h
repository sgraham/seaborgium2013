// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Originally derived from some rendering code in "GWEN", copyright below:
/*
  GWEN

  Copyright (c) 2010 Facepunch Studios

  MIT License

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef SG_RENDER_DIRECT2D_WIN_H_
#define SG_RENDER_DIRECT2D_WIN_H_

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <list>

#include "base/string16.h"
#include "sg/basic_geometric_types.h"
#include "sg/render/renderer.h"

class Font;
class Texture;

class Direct2DRenderer : public Renderer {
 public:
  Direct2DRenderer(ID2D1RenderTarget* device,
                   IDWriteFactory* dwrite_factory,
                   IWICImagingFactory* wic_factory);
  ~Direct2DRenderer();

  virtual void Release();

  virtual void SetDrawColor(Color color);

  virtual void DrawFilledRect(Rect rect);

  virtual void LoadFont(Font* font);
  virtual void FreeFont(Font* font);
  virtual void RenderText(Font* font, Point pos, const string16& text);
  virtual Point MeasureText(Font* font, const string16& text);

  virtual void DeviceLost();
  virtual void DeviceAcquired(ID2D1RenderTarget* rt);

  void StartClip();
  void EndClip();

  void DrawTexturedRectAlpha(
      Texture* texture,
      Rect target_rect,
      float alpha,
      float u1, float v1, float u2, float v2);
  void LoadTexture(Texture* texture);
  void FreeTexture(Texture* texture);

 private:
  bool InternalCreateDeviceResources();
  void InternalReleaseDeviceResources();

 private:
  bool InternalLoadTexture(Texture* texture);
  bool InternalLoadFont(Font* texture);

  void InternalFreeFont(Font* texture, bool bRemove = true);
  void InternalFreeTexture(Texture* texture, bool bRemove = true);

 private:
  IDWriteFactory* dwrite_factory_;
  IWICImagingFactory* wic_factory_;
  ID2D1RenderTarget* rt_;
  ID2D1Factory* d2d_factory_;
  HWND hwnd_;

  ID2D1SolidColorBrush* solid_color_brush_;
  D2D1::ColorF color_;

  std::list<Texture*> texture_list_;
  std::list<Font*> font_list_;
};

#endif  // SG_RENDER_DIRECT2D_WIN_H_