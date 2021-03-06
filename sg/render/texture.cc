// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sg/render/texture.h"

#include "sg/render/renderer.h"

void Texture::Load(const string16& name, Renderer* renderer) {
  this->name = name;
  renderer->LoadTexture(this);
}

void Texture::Free(Renderer* renderer) {
  renderer->FreeTexture(this);
}
