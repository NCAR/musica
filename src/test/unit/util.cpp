// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
#include <musica/util.hpp>
#include <gtest/gtest.h>

using namespace musica;

TEST(Util, CreateString)
{
  String str = CreateString("Hello, World!");
  EXPECT_EQ(str.size_, 13);
  EXPECT_STREQ(str.value_, "Hello, World!");
  DeleteString(&str);
  EXPECT_EQ(str.size_, 0);
  EXPECT_EQ(str.value_, nullptr);
}

TEST(Util, NoError)
{
  Error error = NoError();
  EXPECT_EQ(error.code_, 0);
  EXPECT_EQ(error.category_.size_, 0);
  EXPECT_STREQ(error.category_.value_, "");
  EXPECT_EQ(error.message_.size_, 7);
  EXPECT_STREQ(error.message_.value_, "Success");
  DeleteError(&error);
  EXPECT_EQ(error.category_.size_, 0);
  EXPECT_EQ(error.category_.value_, nullptr);
  EXPECT_EQ(error.message_.size_, 0);
  EXPECT_EQ(error.message_.value_, nullptr);
}

TEST(Util, ToError)
{
  Error error = ToError("Test", 1, "Test Error");
  EXPECT_EQ(error.code_, 1);
  EXPECT_EQ(error.category_.size_, 4);
  EXPECT_STREQ(error.category_.value_, "Test");
  EXPECT_EQ(error.message_.size_, 10);
  EXPECT_STREQ(error.message_.value_, "Test Error");
  DeleteError(&error);
  EXPECT_EQ(error.category_.size_, 0);
  EXPECT_EQ(error.category_.value_, nullptr);
  EXPECT_EQ(error.message_.size_, 0);
  EXPECT_EQ(error.message_.value_, nullptr);
}

TEST(Util, IsSuccess)
{
  Error error = NoError();
  EXPECT_TRUE(IsSuccess(error));
  DeleteError(&error);
}

TEST(Util, IsError)
{
  Error error = ToError("Test", 1, "Test Error");
  EXPECT_TRUE(IsError(error, "Test", 1));
  DeleteError(&error);
}

TEST(Util, ToMapping)
{
  Mapping mapping = ToMapping("Test", 1);
  EXPECT_EQ(mapping.name_.size_, 4);
  EXPECT_STREQ(mapping.name_.value_, "Test");
  EXPECT_EQ(mapping.index_, 1);
  DeleteMapping(&mapping);
  EXPECT_EQ(mapping.name_.size_, 0);
  EXPECT_EQ(mapping.name_.value_, nullptr);
}

TEST(Util, FindMappingIndex)
{
  Mapping mappings[] = {ToMapping("Test", 1), ToMapping("Test2", 4), ToMapping("Test3", 9)};
  Error error = NoError();
  EXPECT_EQ(FindMappingIndex(mappings, 3, "Test", &error), 1);
  EXPECT_TRUE(IsSuccess(error));
  EXPECT_EQ(FindMappingIndex(mappings, 3, "Test3", &error), 9);
  EXPECT_TRUE(IsSuccess(error));
  EXPECT_EQ(FindMappingIndex(mappings, 3, "Test2", &error), 4);
  EXPECT_TRUE(IsSuccess(error));
  DeleteError(&error);
  DeleteMapping(&(mappings[0]));
  DeleteMapping(&(mappings[1]));
  DeleteMapping(&(mappings[2]));
}
