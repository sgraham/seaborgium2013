// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Copied from ninja and changed cosmetically.

#include "sg/backend/subprocess.h"

#include <stdio.h>
#include <algorithm>

#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/stringprintf.h"
#include "base/utf_string_conversions.h"

Subprocess::Subprocess() : child_(NULL) , overlapped_(), is_reading_(false) {
}

Subprocess::~Subprocess() {
  if (pipe_) {
    CHECK(CloseHandle(pipe_)) << "CloseHandle";
  }
  // Reap child if forgotten.
  if (child_)
    Finish();
}

HANDLE Subprocess::SetupPipe(HANDLE ioport) {
  string16 pipe_name = base::StringPrintf(
      L"\\\\.\\pipe\\sg_pid%lu_sp%p", GetCurrentProcessId(), this);

  pipe_ = ::CreateNamedPipe(pipe_name.c_str(),
                            PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
                            PIPE_TYPE_BYTE,
                            PIPE_UNLIMITED_INSTANCES,
                            0, 0, INFINITE, NULL);
  CHECK(pipe_ != INVALID_HANDLE_VALUE) << "CreateNamedPipe";

  CHECK(CreateIoCompletionPort(pipe_, ioport, (ULONG_PTR)this, 0)) <<
      "CreateIoCompletionPort";

  memset(&overlapped_, 0, sizeof(overlapped_));
  CHECK(ConnectNamedPipe(pipe_, &overlapped_) ||
      GetLastError() == ERROR_IO_PENDING) << "ConnectNamedPipe";

  // Get the write end of the pipe as a handle inheritable across processes.
  HANDLE output_write_handle = CreateFile(pipe_name.c_str(), GENERIC_WRITE, 0,
                                          NULL, OPEN_EXISTING, 0, NULL);
  HANDLE output_write_child;
  CHECK(DuplicateHandle(GetCurrentProcess(), output_write_handle,
                        GetCurrentProcess(), &output_write_child,
                        0, TRUE, DUPLICATE_SAME_ACCESS)) << "DuplicateHandle";

  CloseHandle(output_write_handle);

  return output_write_child;
}

bool Subprocess::Start(SubprocessSet* set, const string16& command) {
  HANDLE child_pipe = SetupPipe(set->ioport_);

  SECURITY_ATTRIBUTES security_attributes;
  memset(&security_attributes, 0, sizeof(SECURITY_ATTRIBUTES));
  security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
  security_attributes.bInheritHandle = TRUE;
  // Must be inheritable so subprocesses can dup to children.
  HANDLE nul = CreateFile(L"NUL", GENERIC_READ,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      &security_attributes, OPEN_EXISTING, 0, NULL);
  CHECK(nul != INVALID_HANDLE_VALUE);

  STARTUPINFO startup_info;
  memset(&startup_info, 0, sizeof(startup_info));
  startup_info.cb = sizeof(STARTUPINFO);
  startup_info.dwFlags = STARTF_USESTDHANDLES;
  startup_info.hStdInput = nul;
  startup_info.hStdOutput = child_pipe;
  startup_info.hStdError = child_pipe;

  PROCESS_INFORMATION process_info;
  memset(&process_info, 0, sizeof(process_info));

  // CreateProcessW can modify input buffer so we have to make a copy here.
  scoped_array<char16> command_copy(new char16[command.size() + 1]);
  command.copy(command_copy.get(), command.size());
  command_copy[command.size()] = 0;

  // Do not prepend 'cmd /c' on Windows, this breaks command
  // lines greater than 8,191 chars.
  if (!CreateProcess(NULL, command_copy.get(), NULL, NULL,
                     /* inherit handles */ TRUE, CREATE_NEW_PROCESS_GROUP,
                     NULL, NULL,
                     &startup_info, &process_info)) {
    DWORD error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND) {
      // File (program) not found error is treated as a normal build action
      // failure.
      if (child_pipe)
        CloseHandle(child_pipe);
      CloseHandle(pipe_);
      CloseHandle(nul);
      pipe_ = NULL;
      // child_ is already NULL;
      buf_ = L"CreateProcess failed: " \
             L"The system cannot find the file specified.\n";
      return true;
    } else {
      NOTREACHED() << "CreateProcess";
    }
  }

  // Close pipe channel only used by the child.
  if (child_pipe)
    CloseHandle(child_pipe);
  CloseHandle(nul);

  CloseHandle(process_info.hThread);
  child_ = process_info.hProcess;

  return true;
}

void Subprocess::OnPipeReady() {
  DWORD bytes;
  if (!GetOverlappedResult(pipe_, &overlapped_, &bytes, TRUE)) {
    if (GetLastError() == ERROR_BROKEN_PIPE) {
      CloseHandle(pipe_);
      pipe_ = NULL;
      return;
    }
    NOTREACHED() << ("GetOverlappedResult");
  }

  // TODO(widechar): Is this ASCII? or ANSI? or UTF8?
  if (is_reading_ && bytes)
    buf_.append(ASCIIToUTF16(base::StringPiece(overlapped_buf_, bytes)));

  memset(&overlapped_, 0, sizeof(overlapped_));
  is_reading_ = true;
  if (!::ReadFile(pipe_, overlapped_buf_, sizeof(overlapped_buf_),
                  &bytes, &overlapped_)) {
    if (GetLastError() == ERROR_BROKEN_PIPE) {
      CloseHandle(pipe_);
      pipe_ = NULL;
      return;
    }
    if (GetLastError() != ERROR_IO_PENDING)
      NOTREACHED() << ("ReadFile");
  }

  // Even if we read any bytes in the readfile call, we'll enter this
  // function again later and get them at that point.
}

ExitStatus Subprocess::Finish() {
  if (!child_)
    return ExitFailure;

  // TODO(scottmg): add error handling for all of these.
  WaitForSingleObject(child_, INFINITE);

  DWORD exit_code = 0;
  GetExitCodeProcess(child_, &exit_code);

  CloseHandle(child_);
  child_ = NULL;

  return exit_code == 0              ? ExitSuccess :
         exit_code == CONTROL_C_EXIT ? ExitInterrupted :
                                       ExitFailure;
}

bool Subprocess::Done() const {
  return pipe_ == NULL;
}

const string16& Subprocess::GetOutput() const {
  return buf_;
}

HANDLE SubprocessSet::ioport_;

SubprocessSet::SubprocessSet() {
  ioport_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
  if (!ioport_)
    NOTREACHED() << ("CreateIoCompletionPort");
  if (!SetConsoleCtrlHandler(NotifyInterrupted, TRUE))
    NOTREACHED() << ("SetConsoleCtrlHandler");
}

SubprocessSet::~SubprocessSet() {
  Clear();

  SetConsoleCtrlHandler(NotifyInterrupted, FALSE);
  CloseHandle(ioport_);
}

BOOL WINAPI SubprocessSet::NotifyInterrupted(DWORD dwCtrlType) {
  if (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT) {
    if (!PostQueuedCompletionStatus(ioport_, 0, 0, NULL))
      NOTREACHED() << ("PostQueuedCompletionStatus");
    return TRUE;
  }

  return FALSE;
}

Subprocess* SubprocessSet::Add(const string16& command) {
  Subprocess* subprocess = new Subprocess;
  if (!subprocess->Start(this, command)) {
    delete subprocess;
    return 0;
  }
  if (subprocess->child_)
    running_.push_back(subprocess);
  else
    finished_.push(subprocess);
  return subprocess;
}

bool SubprocessSet::DoWork() {
  DWORD bytes_read;
  Subprocess* subproc;
  OVERLAPPED* overlapped;

  if (!GetQueuedCompletionStatus(ioport_, &bytes_read, (PULONG_PTR)&subproc,
                                 &overlapped, INFINITE)) {
    if (GetLastError() != ERROR_BROKEN_PIPE)
      NOTREACHED() << ("GetQueuedCompletionStatus");
  }

  if (!subproc) {
    // A NULL subproc indicates that we were interrupted and is
    // delivered by NotifyInterrupted above.
    return true;
  }

  subproc->OnPipeReady();

  if (subproc->Done()) {
    std::vector<Subprocess*>::iterator end =
        std::remove(running_.begin(), running_.end(), subproc);
    if (running_.end() != end) {
      finished_.push(subproc);
      running_.resize(end - running_.begin());
    }
  }

  return false;
}

Subprocess* SubprocessSet::NextFinished() {
  if (finished_.empty())
    return NULL;
  Subprocess* subproc = finished_.front();
  finished_.pop();
  return subproc;
}

void SubprocessSet::Clear() {
  for (std::vector<Subprocess*>::iterator i = running_.begin();
       i != running_.end(); ++i) {
    if ((*i)->child_) {
      if (!GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT,
                                    GetProcessId((*i)->child_))) {
        NOTREACHED() << ("GenerateConsoleCtrlEvent");
      }
    }
  }
  for (std::vector<Subprocess*>::iterator i = running_.begin();
       i != running_.end(); ++i)
    delete *i;
  running_.clear();
}
