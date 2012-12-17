// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sg/workspace.h"

#include "base/bind.h"
#include "sg/app_thread.h"
#include "sg/debug_presenter_notify.h"
#include "sg/render/application_window.h"
#include "sg/source_view.h"
#include "sg/stack_view.h"
#include "sg/status_bar.h"
#include "sg/ui/docking_split_container.h"
#include "sg/ui/docking_tool_window.h"
#include "sg/ui/focus.h"
#include "sg/ui/skin.h"
#include "sg/ui/solid_color.h"

namespace {

Workspace* g_main_workspace;

Dockable* Placeholder(const Skin& skin, const string16& name) {
  return new DockingToolWindow(
      new SolidColor(skin.GetColorScheme().background()), name);
}

}  // namespace

Workspace::Workspace()
    : delegate_(NULL),
      status_bar_(NULL) /*,
      source_view_(NULL),
      source_view_container_(NULL)*/ {
  // Initialization deferred until Init when we know our window size.

  // This is just for Invalidate. TODO(scottmg): Maybe some sort of broadcast,
  // observer, blablah.
  CHECK(!g_main_workspace);
  g_main_workspace = this;
}

Workspace::~Workspace() {
  CHECK(g_main_workspace == this);
  g_main_workspace = NULL;
}

void Workspace::Init() {
  DCHECK(!main_area_);
  main_area_.reset(new DockingWorkspace);
  source_view_ = new SourceView;
  stack_view_ = new StackView;
  stack_view_window_ = new DockingToolWindow(stack_view_, L"Stack");
  main_area_->SetRoot(source_view_);
  Dockable* watch = Placeholder(skin_, L"Watch");
  Dockable* locals = Placeholder(skin_, L"Locals");
  Dockable* breakpoints = Placeholder(skin_, L"Breakpoints");
  Dockable* output = Placeholder(skin_, L"Output");
  Dockable* log = Placeholder(skin_, L"Log");

  source_view_->parent()->SplitChild(kSplitHorizontal, source_view_, output);
  source_view_->parent()->SetFraction(.7);
  output->parent()->SplitChild(kSplitVertical, output, log);
  output->parent()->SetFraction(.6);

  source_view_->parent()->SplitChild(
      kSplitVertical, source_view_, stack_view_window_);
  source_view_->parent()->SetFraction(.375);
  stack_view_window_->parent()->SplitChild(
      kSplitVertical, stack_view_window_, watch);
  stack_view_window_->parent()->SetFraction(.4);
  watch->parent()->SplitChild(kSplitHorizontal, watch, locals);
  watch->parent()->SetFraction(.65);
  stack_view_window_->parent()->SplitChild(
      kSplitHorizontal, stack_view_window_, breakpoints);
  stack_view_window_->parent()->SetFraction(.6);

  SetFocusedContents(source_view_);

  // TODO(scottmg): Portrait.
}

void Workspace::SetDelegate(ApplicationWindow* delegate) {
  delegate_ = delegate;
}

void Workspace::SetDebugPresenterNotify(DebugPresenterNotify* debug_presenter) {
  debug_presenter_notify_ = debug_presenter;
}

void Workspace::SetScreenRect(const Rect& rect) {
  if (!main_area_)
    Init();
  // TODO(scottmg): Space for status bar, etc.
  main_area_->SetScreenRect(rect);
  /*
  const Skin& skin = GetSkin();
  Rect outer_border_size(
      skin.border_size(), skin.border_size(),
      skin.border_size(), skin.border_size());
  Rect remaining = rect.Contract(outer_border_size);
  Rect main_area_rect = remaining.Contract(
      Rect(0, 0, 0, skin.status_bar_size()));
  main_area_->SetScreenRect(main_area_rect);
  status_bar_->SetScreenRect(
      Rect(remaining.x, remaining.y + main_area_rect.h + skin.border_size(),
           remaining.w, skin.status_bar_size() - skin.border_size()));
           */
}

void Workspace::Invalidate() {
  g_main_workspace->InvalidateImpl();
}

void Workspace::InvalidateImpl() {
  if (delegate_) {
    AppThread::PostDelayedTask(AppThread::UI, FROM_HERE,
        base::Bind(&ApplicationWindow::Paint, base::Unretained(delegate_)),
        base::TimeDelta::FromMilliseconds(10));
  }
}

void Workspace::Render(Renderer* renderer) {
  main_area_->Render(renderer, skin_);
}

void Workspace::SetFileName(const FilePath& filename) {
  /*source_view_container_->SetTitle(source_view_, filename.LossyDisplayName());
   */
}

void Workspace::SetProgramCounterLine(int line_number) {
  source_view_->SetProgramCounterLine(line_number);
}

void Workspace::SetFileData(const std::string& utf8_text) {
  source_view_->SetData(utf8_text);
}

void Workspace::SetDebugState(const string16& debug_state) {
  status_bar_->SetDebugState(debug_state);
}

void Workspace::SetRenderTime(double frame_time_in_ms) {
  status_bar_->SetRenderTime(frame_time_in_ms);
}

void Workspace::SetStackData(
    const std::vector<FrameData>& frame_data, int active) {
  stack_view_->SetData(frame_data, active);
}

void Workspace::SetLocalsData(const std::vector<TypeNameValue>& locals_data) {
  // locals_view_->SetData(locals_data);
}

bool Workspace::NotifyMouseMoved(
    int x, int y, int dx, int dy, const InputModifiers& modifiers) {
  mouse_position_.x = x;
  mouse_position_.y = y;
  main_area_->UpdateCursor(mouse_position_);
  Dockable* focused = GetFocusedContents();
  if (!focused || !focused->WantMouseEvents())
    return false;
  return focused->NotifyMouseMoved(x, y, dx, dy, modifiers);
}

bool Workspace::NotifyMouseWheel(int delta, const InputModifiers& modifiers) {
  Dockable* focused = GetFocusedContents();
  if (!focused || !focused->WantMouseEvents())
    return false;
  return focused->NotifyMouseWheel(delta, modifiers);
}

bool Workspace::NotifyMouseButton(
    int index, bool down, const InputModifiers& modifiers) {
  // TODO(input): Something! Change focus, forward, etc.
  return false;
}

bool Workspace::NotifyKey(
    InputKey key, bool down, const InputModifiers& modifiers) {
  Dockable* focused = GetFocusedContents();
  if (!focused || !focused->WantKeyEvents())
    return false;
  if (focused->NotifyKey(key, down, modifiers))
    return true;
  return debug_presenter_notify_->NotifyKey(key, down, modifiers);
}
