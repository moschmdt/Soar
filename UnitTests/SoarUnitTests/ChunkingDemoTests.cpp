//
//  ChunkingDemoTests.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 7/29/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "ChunkingDemoTests.hpp"

#include "SoarHelper.hpp"
#include "sml_ClientAnalyzedXML.h"

/* Note that some test don't get as many successful learned chunks as expected
 * because Soar is not able to detect they're duplicates using the sourcing
 * mechanism these tests use to verify chunk contents.  */

void ChunkingDemoTests::Demo_Arithmetic() {
  check_chunk("Demo_Arithmetic", 2810, 29, false);
}
void ChunkingDemoTests::Demo_Blocks_World_Hierarchical_Look_Ahead() {
  check_chunk("Demo_Blocks_World_Hierarchical_Look_Ahead", 47, 1, false);
}
void ChunkingDemoTests::Demo_Blocks_World_Hierarchical() {
  check_chunk("Demo_Blocks_World_Hierarchical", 24, 20, false);
}
void ChunkingDemoTests::Demo_Blocks_World_Look_Ahead_State_Evaluation() {
  check_chunk("Demo_Blocks_World_Look_Ahead_State_Evaluation", 61, 24, false);
}
void ChunkingDemoTests::Demo_Blocks_World_Look_Ahead() {
  check_chunk("Demo_Blocks_World_Look_Ahead", 64, 8, false);
}
void ChunkingDemoTests::Demo_Blocks_World_Operator_Subgoaling() {
  check_chunk("Demo_Blocks_World_Operator_Subgoaling", 6, 1, false);
}
void ChunkingDemoTests::Demo_Eight_Puzzle() {
  check_chunk("Demo_Eight_Puzzle", 20, 7, false);
}
void ChunkingDemoTests::Demo_Graph_Search() {
  check_chunk("Demo_Graph_Search", 20, 7, false);
}
void ChunkingDemoTests::Demo_MaC_Planning() {
  check_chunk("Demo_MaC_Planning", 122, 34, false);
}
void ChunkingDemoTests::Demo_RL_Unit() {
  check_chunk("Demo_RL_Unit", 26, 15, false);
}
void ChunkingDemoTests::Demo_ToH_Recursive() {
  check_chunk("Demo_ToH_Recursive", 30, 61, false);
}
void ChunkingDemoTests::Demo_Water_Jug_Hierarchy() {
  check_chunk("Demo_Water_Jug_Hierarchy", 99, 3, false);
}
void ChunkingDemoTests::Demo_Water_Jug_Look_Ahead() {
  check_chunk("Demo_Water_Jug_Look_Ahead", 400, 26, false);
}
void ChunkingDemoTests::Demo_Water_Jug_Tie() {
  check_chunk("Demo_Water_Jug_Tie", 21, 5, false);
}
void ChunkingDemoTests::Elio_Agent() {
  check_chunk("Elio_Agent", 795, 135, false);
}
void ChunkingDemoTests::PRIMS_Sanity1() {
  check_chunk("PRIMS_Sanity1", 795, 16, false);
}
void ChunkingDemoTests::PRIMS_Sanity2() {
  check_chunk("PRIMS_Sanity2", 728, 19, false);
}
void ChunkingDemoTests::Teach_Soar_90_Games() {
  check_chunk("Teach_Soar_90_Games", 10000, 15, false);
} /* Should be 28 Probably re-ordering problems.  The rules learned are huge */
void ChunkingDemoTests::Teach_Soar_9_Games() {
  check_chunk("Teach_Soar_9_Games", 23850, 60, false);
} /* Should be 28 Probably re-ordering problems.  The rules learned are huge */

void ChunkingDemoTests::setUp() { FunctionalTestHarness::setUp(); }

void ChunkingDemoTests::tearDown(bool caught) {
  FunctionalTestHarness::tearDown(caught);
}

void ChunkingDemoTests::save_chunks(const char* pTestName) {
  std::string lCmdName;
  if (SoarHelper::save_logs) {
    lCmdName = "output command-to-file ";
    SoarHelper::add_log_dir_if_exists(lCmdName);
    lCmdName += "temp_chunks_";
    lCmdName += pTestName;
    lCmdName += ".soar print -frc";
  } else {
    lCmdName = "output command-to-file temp_chunks.soar print -fcr";
  }
  SoarHelper::agent_command(agent, lCmdName.c_str());
}

void ChunkingDemoTests::save_chunks_internal(const char* pTestName) {
  std::string lCmdName;
  if (SoarHelper::save_logs) {
    lCmdName = "output command-to-file ";
    SoarHelper::add_log_dir_if_exists(lCmdName);
    lCmdName += "temp_chunks_";
    lCmdName += pTestName;
    lCmdName += ".soar print -frci";
  } else {
    lCmdName = "output command-to-file temp_chunks.soar print -fcri";
  }
  SoarHelper::agent_command(agent, lCmdName.c_str());
}

void ChunkingDemoTests::source_saved_chunks(const char* pTestName) {
  std::string lCmdName;
  if (SoarHelper::save_logs) {
    lCmdName = "source ";
    SoarHelper::add_log_dir_if_exists(lCmdName);
    lCmdName += "temp_chunks_";
    lCmdName += pTestName;
    lCmdName += ".soar";
  } else {
    lCmdName = "source temp_chunks.soar";
  }
  SoarHelper::agent_command(agent, lCmdName.c_str());
}

void ChunkingDemoTests::check_chunk(const char* pTestName, int64_t decisions,
                                    int64_t expected_chunks,
                                    bool directSourceChunks) {
  SoarHelper::start_log(agent, pTestName);
  assertTrue_msg("Could not find " + this->getCategoryName() + " test file '" +
                     pTestName + "'",
                 SoarHelper::source(agent, this->getCategoryName(), pTestName));
  if (!SoarHelper::no_explainer) {
    SoarHelper::agent_command(agent, "explain all on");
    SoarHelper::agent_command(agent, "explain just on");
  }
  //    if (SoarHelper::save_logs)
  //    {
  //        SoarHelper::agent_command(agent,"trace -CbL 2");
  //    }
  SoarHelper::check_learning_override(agent);
  agent->RunSelf(decisions, sml::sml_DECIDE);
  assertTrue_msg(agent->GetLastErrorDescription(),
                 agent->GetLastCommandLineResult());

  verify_chunk(pTestName, expected_chunks, directSourceChunks);
}

void ChunkingDemoTests::verify_chunk(const char* pTestName,
                                     int64_t expected_chunks,
                                     bool directSourceChunks) {
  if (SoarHelper::save_logs) {
    SoarHelper::agent_command(agent, "chunk ?");
    SoarHelper::agent_command(agent, "production firing-count");
    SoarHelper::agent_command(agent, "print -cf");
  }
  if (!directSourceChunks) {
    SoarHelper::close_log(agent);
    save_chunks(pTestName);
    if (SoarHelper::save_after_action_report) {
      SoarHelper::agent_command(agent, "explain after-action-report on");
    }
    SoarHelper::init_check_to_find_refcount_leaks(agent);
    tearDown(false);
    setUp();
    SoarHelper::continue_log(agent, pTestName);
    source_saved_chunks(pTestName);
  } else {
    SoarHelper::close_log(agent);
    save_chunks_internal(pTestName);
    if (SoarHelper::save_after_action_report) {
      SoarHelper::agent_command(agent, "explain after-action-report on");
    }
    SoarHelper::init_check_to_find_refcount_leaks(agent);
    tearDown(false);
    setUp();
    SoarHelper::continue_log(agent, pTestName);
    source_saved_chunks(pTestName);
  }
  {
    sml::ClientAnalyzedXML response;

    std::string sourceName =
        this->getCategoryName() + "_" + pTestName + "_expected" + ".soar";

    std::string lPath = SoarHelper::GetResource(sourceName);
    assertNonZeroSize_msg("Could not find test file '" + sourceName + "'",
                          lPath);

    agent->ExecuteCommandLineXML(
        std::string("source \"" + lPath + "\"").c_str(), &response);
    int sourced, excised, ignored;
    ignored =
        response.GetArgInt(sml::sml_Names::kParamIgnoredProductionCount, -1);
    sourced =
        response.GetArgInt(sml::sml_Names::kParamSourcedProductionCount, -1);
    excised =
        response.GetArgInt(sml::sml_Names::kParamExcisedProductionCount, -1);
    std::ostringstream outStringStream("");
    if (ignored < expected_chunks) {
      outStringStream << "Only learned " << ignored << " of the expected "
                      << expected_chunks << ".";
      if (SoarHelper::save_logs) {
        agent->ExecuteCommandLine(
            (std::string("output log --add |") + outStringStream.str().c_str() +
             std::string("|"))
                .c_str(),
            false, false);
        SoarHelper::agent_command(agent, "print -cf");
      }
    } else {
      std::cout << " " << ignored << "/" << expected_chunks << " ";
      if (SoarHelper::save_logs) {
        agent->ExecuteCommandLine(
            "output log -a Success!!!  All expected rules were learned!!!",
            false, false);
      }
    }
    assertTrue_msg(outStringStream.str().c_str(), ignored >= expected_chunks);
  }
  SoarHelper::close_log(agent);
}
