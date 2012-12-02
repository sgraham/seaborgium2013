#ifndef UI_CONTENTS_H_
#define UI_CONTENTS_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "Gwen/BaseRender.h"
#include "ui/base_types.h"
class Skin;

class Contents {
 public:
  Contents() : parent_(NULL) {}
  virtual ~Contents() {}

  virtual void SetParent(Contents* parent) {
    parent_ = parent;
  }
  virtual Contents* GetParent() const {
    return parent_;
  }

  virtual void Render(const Skin& skin, Gwen::Renderer::Base* renderer) = 0;

  virtual void SetScreenRect(const Rect& rect) {
    rect_ = rect;
  }

  virtual const Rect& GetScreenRect() const { return rect_; }

  virtual bool CanHoldChildren() const { return false; }

 private:
  Contents* parent_;
  Rect rect_;

  DISALLOW_COPY_AND_ASSIGN(Contents);
};

#endif  // UI_CONTENTS_H_
