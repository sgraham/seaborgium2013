// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SG_BACKEND_GDB_TO_GENERIC_CONVERTER_H_
#define SG_BACKEND_GDB_TO_GENERIC_CONVERTER_H_

#include <string>
#include <vector>

#include "sg/backend/backend.h"
#include "sg/backend/gdb_mi_parse.h"

// This is the boring but somewhat verbose code that converts from the parsed
// gdb output to the backend-agnostic structures that are passed back to UI
// code in notifications.

std::string FindStringValue(
    const std::string& key,
    const std::vector<GdbRecordResult*>& results);

const base::DictionaryValue* FindDictionaryValue(
    const std::string& key,
    const std::vector<GdbRecordResult*>& results);

const base::ListValue* FindListValue(
    const std::string& key,
    const std::vector<GdbRecordResult*>& results);

StoppedAtBreakpointData StoppedAtBreakpointDataFromRecordResults(
    const std::vector<GdbRecordResult*>& results);

StoppedAfterSteppingData StoppedAfterSteppingDataFromRecordResults(
    const std::vector<GdbRecordResult*>& results);

RetrievedStackData RetrievedStackDataFromList(base::Value* list_value);

RetrievedStackData MergeArgumentsIntoStackFrameData(
    const RetrievedStackData& just_stack,
    base::Value* list_value);

RetrievedLocalsData RetrievedLocalsDataFromList(base::Value* list_value);

LibraryLoadedData LibraryLoadedDataFromRecordResults(
    const std::vector<GdbRecordResult*>& results);

WatchCreatedData WatchCreatedDataFromRecordResults(
    const std::vector<GdbRecordResult*>& results);

WatchesUpdatedData WatchesUpdatedDataFromChangesList(base::Value* list_value);

WatchesChildListData WatchesChildListDataFromRecordResults(
    const std::vector<GdbRecordResult*>& results);

#endif  // SG_BACKEND_GDB_TO_GENERIC_CONVERTER_H_
