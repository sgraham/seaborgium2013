// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SG_SOURCE_VIEW_H_
#define SG_SOURCE_VIEW_H_

#include <string>
#include <vector>

#include "sg/basex/string16.h"
#include "sg/lexer.h"
#include "sg/ui/dockable.h"
#include "sg/ui/scroll_helper.h"

class Skin;

struct ColoredText {
  Lexer::TokenType type;
  string16 text;
};
typedef std::vector<ColoredText> Line;

class SourceView : public Dockable, public ScrollHelperDataProvider {
 public:
  SourceView();

  virtual void Render(Renderer* renderer) OVERRIDE;

  virtual void SetData(const std::string& utf8_text);
  // TODO(scottmg): Probably some sort of "margin indicator" abstraction.
  virtual void SetProgramCounterLine(int line_number);

  // Implementation of InputHandler:
  virtual bool NotifyMouseMoved(
      int x, int y, int dx, int dy, const InputModifiers& modifiers) OVERRIDE;
  virtual bool NotifyMouseWheel(
      int delta, const InputModifiers& modifiers) OVERRIDE;
  virtual bool NotifyMouseButton(
      int index, bool down, const InputModifiers& modifiers) OVERRIDE;
  virtual bool NotifyKey(
      InputKey key, bool down, const InputModifiers& modifiers) OVERRIDE;
  virtual bool WantMouseEvents() { return true; }
  virtual bool WantKeyEvents() { return true; }

  // Implementation of ScrollHelperDataProvider:
  virtual int GetContentSize() OVERRIDE;
  virtual const Rect& GetScreenRect() const OVERRIDE {
    return Dockable::GetScreenRect();
  }


 private:
  const Color& ColorForTokenType(const Skin& skin, Lexer::TokenType type);
  void CommitAfterHighlight(std::vector<Line> lines);
  bool LineInView(int line_number);
  int GetFirstLineInView();

  float y_pixel_scroll_;
  float y_pixel_scroll_target_;

  std::vector<Line> lines_;

  ScrollHelper scroll_helper_;

  int program_counter_line_;
};

#endif  // SG_SOURCE_VIEW_H_
