// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SG_DEBUG_PRESENTER_DISPLAY_H_
#define SG_DEBUG_PRESENTER_DISPLAY_H_

#include <string>
#include <vector>

#include "sg/basex/string16.h"

class FrameData;
class TypeNameValue;

class DebugPresenterVariable {
 public:
  DebugPresenterVariable();
  DebugPresenterVariable(const string16& type, const string16& name);
  const string16& type() const { return type_; }
  const string16& name() const { return name_; }
  const string16& key() const { return key_; }

  void set_backend_id(const std::string& id) { backend_id_ = id; }
  const std::string& backend_id() const { return backend_id_; }

 private:
  string16 type_;
  string16 name_;
  string16 key_;
  std::string backend_id_;
};

// The interface that the DebugPresenter requires of its view.
class DebugPresenterDisplay {
 public:
  virtual ~DebugPresenterDisplay();

  virtual void SetFileName(const string16& filename) = 0;
  virtual void SetFileData(const std::string& utf8_text) = 0;
  virtual void SetProgramCounterLine(int line_number) = 0;

  virtual void SetStackData(const std::vector<FrameData>& frame_data,
                            int active) = 0;

  virtual void AddLocalsChild(
      const std::string& parent_id, const std::string& child_id) = 0;
  virtual void SetLocalsNodeData(
      const std::string& id,
      const string16* expression,
      const string16* value,
      const string16* type,
      const bool* has_children) = 0;
  virtual void RemoveLocalsNode(const std::string& id) = 0;
  virtual int GetLocalsChildCount(const std::string& id) = 0;
  virtual std::string GetLocalsIdOfChild(
      const std::string& parent_id, int child_index) = 0;

  virtual void AddOutput(const string16& text) = 0;

  virtual void AddLog(const string16& text) = 0;

  virtual void SetRenderTime(double ms_per_frame) = 0;
};

#endif  // SG_DEBUG_PRESENTER_DISPLAY_H_
