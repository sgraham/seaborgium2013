// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SG_UI_DOCKING_TOOL_WINDOW_H_
#define SG_UI_DOCKING_TOOL_WINDOW_H_

#include "base/basictypes.h"
#include "sg/basex/compiler_specific.h"
#include "sg/basex/string16.h"
#include "sg/ui/dockable.h"

// Renders window decoration, handles drag/re-dock interaction.
class DockingToolWindow : public Dockable {
 public:
  DockingToolWindow(Dockable* dockable, const string16& title);
  virtual ~DockingToolWindow();

  virtual void Render(Renderer* renderer) OVERRIDE;
  virtual void SetScreenRect(const Rect& rect) OVERRIDE;
  virtual bool CouldStartDrag(DragSetup* drag_setup) OVERRIDE;
  virtual Dockable* FindTopMostUnderPoint(const Point& point) OVERRIDE;

 private:
  Rect RectForTitleBar();

  Dockable* contents_;
  string16 title_;
};

#endif  // SG_UI_DOCKING_TOOL_WINDOW_H_
