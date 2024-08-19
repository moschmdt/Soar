//
//  TestRunner.hpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/16/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef TestRunner_cpp
#define TestRunner_cpp

#include <atomic>
#include <condition_variable>
#include <functional>
#include <sstream>

class TestCategory;

class TestRunner {
  TestCategory* category;
  std::function<void()> function;

  std::condition_variable_any* variable;

 public:
  TestRunner(TestCategory* category, std::function<void()> function,
             std::condition_variable_any* variable);

  void run();

  std::atomic<bool> kill;
  std::atomic<bool> ready;
  std::atomic<bool> done;

  bool failed;
  std::string failureMessage;

  std::stringstream output;
};

#endif /* TestRunner_cpp */
