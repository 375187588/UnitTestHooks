/// @file Test_WS2_32_hook.h
/// 
/// Verifies the unit-test hook object for Windows Sockets through the WS2_32_hook library.
/// 
/// The MIT License(MIT)
/// @copyright 2015 Paul M Watt
/// 
/// Verify data with these TEST ASSERTIONS:
/// 
///  TS_FAIL(message):                        Fail unconditionally
///  TS_ASSERT(expr):                         Verify (expr) is true
///  TS_ASSERT_EQUALS(x, y):                  Verify (x==y)
///  TS_ASSERT_SAME_DATA(x, y, size):         Verify two buffers are equal
///  TS_ASSERT_DELTA(x, y, d):                Verify (x==y) up to d
///  TS_ASSERT_DIFFERS(x, y):                 Verify !(x==y)
///  TS_ASSERT_LESS_THAN(x, y):               Verify (x<y)
///  TS_ASSERT_LESS_THAN_EQUALS(x, y):        Verify (x<=y)
///  TS_ASSERT_PREDICATE(P, x):               Verify P(x)
///  TS_ASSERT_RELATION(R, x, y):             Verify x R y, ex. TS_ASSERT_RELATION(std::greater, x, y);
///  TS_ASSERT_THROWS(expr, type):            Verify that (expr) throws a specific type of exception.
///  TS_ASSERT_THROWS_EQUALS(expr, arg, x, y):Verify type and value of what (expr) throws
///  TS_ASSERT_THROWS_ANYTHING(expr):         Verify that (expr) throws an exception
///  TS_ASSERT_THROWS_NOTHING(expr):          Verify that (expr) doesn't throw anything
///  TS_WARN(message):                        Print message as a warning
///  TS_TRACE(message):                       Print message as an information message
/// 
//  ****************************************************************************
#ifndef Test_WS2_32_hook_H_INCLUDED
#define Test_WS2_32_hook_H_INCLUDED
//  Includes *******************************************************************
#include <cxxtest/TestSuite.h>
#include "../../../../src/api/windows/WS2_32/WS2_32.h"

using namespace cxxhook;

//  ****************************************************************************
/// Test_WS2_32_hook Test Suite class.
///
class Test_WS2_32_hook : public CxxTest::TestSuite
{
public:

  Test_WS2_32_hook()
  {
    // TODO: Construct Test Suite Object
  }

  // Fixture Management ********************************************************
  // setUp will be called before each test case in order to setup common fixtures.
  virtual void setUp()
  {
    // TODO: Add common fixture setup code if any exists.
  }
 
  // tearDown will be called after each test case to clean up common resources.
  virtual void tearDown()
  {
    // TODO: Add common fixture teardown code if any exists.
  }

protected:
  // Test Suite Data ***********************************************************
  typedef cxxhook::WS2_32                                   SUT;


  // Creator Methods ***********************************************************
  // TODO: Use creator methods to reduce redundant setup code in test cases.

public:
  // Test Cases ****************************************************************
  // TODO: Add a new function for each unique test to be performed in this suite. 
  void TestCase1(void);

};

//  ****************************************************************************
void Test_WS2_32_hook::TestCase1(void)
{
  // TODO: Implement Single Test Case.
  // TODO: The implementation can also be placed in a cpp file if desired.
}

#endif