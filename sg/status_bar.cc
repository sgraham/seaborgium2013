// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sg/status_bar.h"

#include "base/string_number_conversions.h"
#include "base/utf_string_conversions.h"
#include "Gwen/Font.h"
#include "sg/ui/skin.h"

// TODO(rendering)
// TODO(config)
Gwen::Font kStatusBarFont(L"Segoe UI", 12.f);

StatusBar::StatusBar(const Skin& skin) : Contents(skin) {
  debug_state_ = L"";
  ms_per_frame_ = L"";
}

StatusBar::~StatusBar() {
}

void StatusBar::SetRenderTime(double ms_per_frame) {
  ms_per_frame_ = UTF8ToUTF16(base::DoubleToString(ms_per_frame)) + L" ms";
}

void StatusBar::SetDebugState(const string16& status) {
  debug_state_ = status;
}

void StatusBar::Render(Gwen::Renderer::Base* renderer) {
  const Skin& skin = GetSkin();
  const ColorScheme& color = skin.GetColorScheme();

  renderer->SetDrawColor(color.background());
  // TODO(rendering): Conversion.
  Rect client_rect = GetClientRect();
  renderer->DrawFilledRect(
      Gwen::Rect(client_rect.x, client_rect.y, client_rect.w, client_rect.h));

  const int kEdgeOffset = 3;
  renderer->SetDrawColor(color.text());
  renderer->RenderText(&kStatusBarFont, Gwen::Point(kEdgeOffset, kEdgeOffset),
                       L"Ready.");

  int debug_state_width =
      renderer->MeasureText(&kStatusBarFont, debug_state_).x;
  int ms_per_frame_width =
      renderer->MeasureText(&kStatusBarFont, ms_per_frame_).x;

  const int kItemPadding = 10;
  int x = GetClientRect().w - kItemPadding - ms_per_frame_width;
  renderer->RenderText(&kStatusBarFont, Gwen::Point(x, kEdgeOffset),
                       ms_per_frame_);
  int separator_x = x - kItemPadding;
  x -= kItemPadding + 1 + kItemPadding + debug_state_width;
  renderer->RenderText(&kStatusBarFont, Gwen::Point(x , kEdgeOffset),
                       debug_state_);

  renderer->SetDrawColor(color.border());
  renderer->DrawFilledRect(Gwen::Rect(separator_x, 0, 1, client_rect.h));
  x -= kItemPadding + 1;
  renderer->DrawFilledRect(Gwen::Rect(x, 0, 1, client_rect.h));
}