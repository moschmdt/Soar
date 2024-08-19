//
//  FunctionalTests.hpp
//  Prototype-UnitTesting
//
//  Created by Alex Turner on 6/17/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#ifndef FunctionalTests_cpp
#define FunctionalTests_cpp

#include "FunctionalTestHarness.hpp"

/**
 * @author ray
 */
class FunctionalTests : public FunctionalTestHarness {
 public:
  TEST_CATEGORY(FunctionalTests);

  TEST(testWaterJug, 5000)
  void testWaterJug();

  TEST(testWaterJugHierarchy, 10000)
  void testWaterJugHierarchy();

  TEST(testTowersOfHanoi, -1)
  void testTowersOfHanoi();

  TEST(testTowersOfHanoiFast, -1)
  void testTowersOfHanoiFast();

  TEST(testEightPuzzle, 10000)
  void testEightPuzzle();

  TEST(testBlocksWorld, 10000)
  void testBlocksWorld();

  TEST(testBlocksWorldOperatorSubgoaling, 10000)
  void testBlocksWorldOperatorSubgoaling();

  TEST(testBlocksWorldLookAhead, 10000)
  void testBlocksWorldLookAhead();

  TEST(testArithmetic, 80000)
  void testArithmetic();

  TEST(testCountTest, 80000)
  void testCountTest();
};

#endif /* FunctionalTests_cpp */
