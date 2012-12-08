// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "sg/backend/gdb_mi_parse.h"
#include "sg/test.h"

class GdbMiParse : public LeakCheckTest {
};

TEST_F(GdbMiParse, Welcome) {
  GdbMiParser p;

  scoped_ptr<GdbRecord> welcome(p.Parse("~\"GNU gdb (GDB) 7.5\\n\"\r", NULL));
  EXPECT_EQ(GdbRecord::RT_CONSOLE_STREAM_OUTPUT, welcome->record_type());
  EXPECT_EQ("GNU gdb (GDB) 7.5\n", welcome->OutputString());
}

TEST_F(GdbMiParse, LogData) {
  GdbMiParser p;
  scoped_ptr<GdbRecord> diag(p.Parse("&\"set disassembly-flavor intel\\n\"\r",
                                     NULL));
  EXPECT_EQ(GdbRecord::RT_LOG_STREAM_OUTPUT, diag->record_type());
  EXPECT_EQ("set disassembly-flavor intel\n", diag->OutputString());
}

TEST_F(GdbMiParse, ResultDone) {
  GdbMiParser p;
  scoped_ptr<GdbRecord> done(p.Parse("^done\r", NULL));
  EXPECT_EQ(GdbRecord::RT_RESULT_RECORD, done->record_type());
  EXPECT_EQ("done", done->ResultClass());
  EXPECT_EQ(0, done->results().size());
}

TEST_F(GdbMiParse, ResultDoneSimple) {
  GdbMiParser p;
  scoped_ptr<GdbRecord> done(p.Parse("^done,value=\"42.432000000000002\"\r",
                                     NULL));
  EXPECT_EQ(GdbRecord::RT_RESULT_RECORD, done->record_type());
  EXPECT_EQ("done", done->ResultClass());
  EXPECT_EQ(1, done->results().size());
  EXPECT_EQ("value", done->results()[0]->variable());
  std::string value;
  EXPECT_TRUE(done->results()[0]->value()->GetAsString(&value));
  EXPECT_EQ("42.432000000000002", value);
}

TEST_F(GdbMiParse, ResultErrorSimple) {
  GdbMiParser p;
  scoped_ptr<GdbRecord> done(p.Parse(
      "^error,msg=\"Undefined info command: \\\"regs\\\"."
      "  Try \\\"help info\\\".\"\r",
      NULL));
  EXPECT_EQ(GdbRecord::RT_RESULT_RECORD, done->record_type());
  EXPECT_EQ("error", done->ResultClass());
  EXPECT_EQ(1, done->results().size());
  EXPECT_EQ("msg", done->results()[0]->variable());
  std::string value;
  EXPECT_TRUE(done->results()[0]->value()->GetAsString(&value));
  EXPECT_EQ("Undefined info command: \"regs\".  Try \"help info\".", value);
}

TEST_F(GdbMiParse, ResultDoneTuple) {
  GdbMiParser p;
  scoped_ptr<GdbRecord> done(p.Parse(
      "^done,stuff={a=\"stuff\",b=\"things\"}\r", NULL));
  EXPECT_EQ(GdbRecord::RT_RESULT_RECORD, done->record_type());
  EXPECT_EQ("done", done->ResultClass());
  EXPECT_EQ(1, done->results().size());
  EXPECT_EQ("stuff", done->results()[0]->variable());
  base::Value* stuff = done->results()[0]->value();
  base::DictionaryValue* as_dict;
  EXPECT_TRUE(stuff->GetAsDictionary(&as_dict));
  std::string a_value, b_value;
  EXPECT_TRUE(as_dict->GetString("a", &a_value));
  EXPECT_TRUE(as_dict->GetString("b", &b_value));
  EXPECT_EQ("stuff", a_value);
  EXPECT_EQ("things", b_value);
}

TEST_F(GdbMiParse, ResultDoneListOfTuple) {
  GdbMiParser p;
  scoped_ptr<GdbRecord> done(p.Parse(
      "^done,asm_insns=["
      "{address=\"0x004013cb\","
       "func-name=\"main(int, char**)\","
       "offset=\"63\","
       "inst=\"mov    DWORD PTR [esp+0x1c],0x0\""
      "},"
       "{address=\"0x004013d3\","
        "func-name=\"main(int, char**)\","
        "offset=\"71\","
        "inst=\"jmp    0x4013fd <main(int, char**)+113>\""
      "}]\r", NULL));
  EXPECT_EQ(GdbRecord::RT_RESULT_RECORD, done->record_type());
}

TEST_F(GdbMiParse, FullOutput) {
  GdbMiReader reader;
  scoped_ptr<GdbOutput> output(reader.Parse(
      "=thread-group-added,id=\"i1\"\r"
      "~\"GNU gdb (GDB) 7.5\\n\"\r"
      "~\"<http blahblah\"\r"
      "(gdb)\r"));
  EXPECT_EQ(3, output->size());
  EXPECT_EQ(GdbRecord::RT_NOTIFY_ASYNC_OUTPUT, output->at(0)->record_type());
  EXPECT_EQ("thread-group-added", output->at(0)->AsyncClass());
  EXPECT_EQ(1, output->at(0)->results().size());
  EXPECT_EQ("id", output->at(0)->results()[0]->variable());
  EXPECT_EQ(GdbRecord::RT_CONSOLE_STREAM_OUTPUT, output->at(1)->record_type());
  EXPECT_EQ("GNU gdb (GDB) 7.5\n", output->at(1)->OutputString());
  EXPECT_EQ(GdbRecord::RT_CONSOLE_STREAM_OUTPUT, output->at(2)->record_type());
  EXPECT_EQ("<http blahblah", output->at(2)->OutputString());
}

// TODO(testing): Bad/unexpected outputs.