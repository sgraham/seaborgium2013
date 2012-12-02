#ifndef WORKSPACE_H_
#define WORKSPACE_H_

#include "Gwen/Gwen.h"
#include "Gwen/Align.h"
#include "Gwen/Utility.h"

class Container;

class Workspace : public Gwen::Controls::Base {
 public:
  GWEN_CONTROL(Workspace, Gwen::Controls::Base);
  void Render(Gwen::Skin::Base* skin);

 private:
  Container* root_;
};

#endif  // WORKSPACE_H_
