//
//  FunctionalTestHarness.hpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef FunctionalTestHarness_cpp
#define FunctionalTestHarness_cpp

#include <string>

#include "TestCategory.hpp"
#include "TestRunner.hpp"
#include "agent.h"
#include "portability.h"
#include "rhs_functions.h"
#include "sml_ClientAgent.h"
#include "sml_ClientKernel.h"
#include "sml_KernelSML.h"
#include "sml_RhsFunction.h"

class FunctionalTestHarness : public TestCategory {
 public:
  FunctionalTestHarness();

 protected:
  struct user_data_struct {
    user_data_struct(std::function<::Symbol*()> routine) : function(routine) {}

    std::function<::Symbol*()> function;
  };

  user_data_struct haltData;
  user_data_struct failedData;
  user_data_struct succeededData;

  sml::Agent* agent;
  sml::Kernel* kernel;
  sml::KernelSML* internal_kernel;
  ::agent* internal_agent;

  bool halted = false;
  bool failed = false;

  ::rhs_function_routine halt_routine;

  void afterDecisionCycleHandler();
  void printHandler(const char* msg);

 public:
  /**
   * Sources rules
   * @param testName The test to perform
   * @throws SoarException
   */
  void runTestSetup(std::string testName);

  /**
   * @brief This function assumes some other function has set up the agent (like
   * runTestSetup)
   *
   * @param testName
   * @param expectedDecisions
   */
  void runTestExecute(std::string testName, int expectedDecisions);

 protected:
  void runTest(std::string testName, int expectedDecisions);

 public:
  void before() { setUp(); }
  void setUp();

  void after(bool caught) { tearDown(caught); }
  void tearDown(bool caught);

  ::Symbol* haltHandler();
  ::Symbol* failedHandler();
  ::Symbol* succeededHandler();

  /**
   * @brief Set up the agent with RHS functions common to these
   * FunctionalTests.
   */
  void installRHS();

  /**
   * @brief Remove the RHS functions we added to prevent memory leaks
   */
  void removeRHS();
};

#endif /* FunctionalTestHarness_cpp */
