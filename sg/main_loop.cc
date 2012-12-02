// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sg/main_loop.h"

#include "base/logging.h"
#include "base/run_loop.h"
#include "sg/app_thread.h"

namespace {
MainLoop* g_current_main_loop = NULL;
}  // namespace

MainLoop::MainLoop() : result_code_(0) {
  DCHECK(!g_current_main_loop);
  g_current_main_loop = this;
}

MainLoop::~MainLoop() {
  DCHECK(g_current_main_loop == this);
  g_current_main_loop = NULL;
}

void MainLoop::Init() {
}

void MainLoop::MainMessageLoopStart() {
  if (!MessageLoop::current())
    main_message_loop_.reset(new MessageLoop(MessageLoop::TYPE_UI));

  const char* kThreadName = "SeaborgiumMain";
  base::PlatformThread::SetName(kThreadName);
  if (main_message_loop_.get())
    main_message_loop_->set_thread_name(kThreadName);

  // Register the main thread by instantiating it, but don't call any methods.
  main_thread_.reset(new AppThread(AppThread::UI, MessageLoop::current()));
}

void MainLoop::CreateThreads() {
  // Create other threads.

  // Disallow IO on UI thread.
}

void MainLoop::MainMessageLoopRun() {
  DCHECK_EQ(MessageLoop::TYPE_UI, MessageLoop::current()->type());
  base::RunLoop run_loop;
  run_loop.Run();
}

void MainLoop::ShutdownThreadsAndCleanUp() {
  // Allow IO

  // Join other threads.
}

int MainLoop::GetResultCode() {
  return result_code_;
}
