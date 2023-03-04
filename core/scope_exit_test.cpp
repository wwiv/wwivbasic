/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*               Copyright (C)2014-2022, WWIV Software Services           */
/*                                                                        */
/*    Licensed  under the  Apache License, Version  2.0 (the "License");  */
/*    you may not use this  file  except in compliance with the License.  */
/*    You may obtain a copy of the License at                             */
/*                                                                        */
/*                http://www.apache.org/licenses/LICENSE-2.0              */
/*                                                                        */
/*    Unless  required  by  applicable  law  or agreed to  in  writing,   */
/*    software  distributed  under  the  License  is  distributed on an   */
/*    "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,   */
/*    either  express  or implied.  See  the  License for  the specific   */
/*    language governing permissions and limitations under the License.   */
/*                                                                        */
/**************************************************************************/
#include "gtest/gtest.h"
#include "core/scope_exit.h"

using wwiv::core::ScopeExit;
using wwiv::core::finally;

TEST(ScopeExitTest, Basic) {
  auto committed = false;
  auto f = [&] { committed = true; };
  {
    ScopeExit e(f);
    ASSERT_FALSE(committed);
  }
  ASSERT_TRUE(committed);
}

TEST(ScopeExitTest, BasicFinally) {
  auto committed = false;
  auto f = [&] { committed = true; };
  {
    auto e = finally(f);
    ASSERT_FALSE(committed);
  }
  ASSERT_TRUE(committed);
}

TEST(ScopeExitTest, Empty) {
  // Should not crash on exit.
  ScopeExit e;
}

TEST(ScopeExitTest, Swap) {
  auto committed = false;
  auto f = [&] { committed = true; };
  {
    ScopeExit e;
    e.swap(f);
    ASSERT_FALSE(committed);
  }
  ASSERT_TRUE(committed);
}
