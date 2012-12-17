// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sg/ui/dockable.h"

#include "base/logging.h"
#include "sg/ui/docking_split_container.h"
#include "sg/workspace.h"

Dockable::Dockable() : parent_(NULL) {
}

Dockable::~Dockable() {
}

DockingSplitContainer* Dockable::AsDockingSplitContainer() {
  CHECK(IsContainer());
  return reinterpret_cast<DockingSplitContainer*>(this);
}

void Dockable::SetScreenRect(const Rect& rect) {
  rect_ = rect;
}

const Rect& Dockable::GetScreenRect() const {
  return rect_;
}

void Dockable::Invalidate() {
  Workspace::Invalidate();
}

void Dockable::UpdateCursor(const Point& screen_position) {
  if (GetScreenRect().Contains(screen_position))
    SetCursor(LoadCursor(NULL, IDC_ARROW));
}
