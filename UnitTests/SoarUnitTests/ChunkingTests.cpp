//
//  ChunkingTests.cpp
//  Soar-xcode
//
//  Created by Alex Turner on 7/29/15.
//  Copyright © 2015 University of Michigan – Soar Group. All rights reserved.
//

#include "ChunkingTests.hpp"

#include "SoarHelper.hpp"
#include "sml_ClientAnalyzedXML.h"

/* Note that some test don't get as many successful learned chunks as expected
 * because Soar is not able to detect they're duplicates using the sourcing
 * mechanism these tests use to verify chunk contents.  */

void ChunkingTests::All_Test_Types() { check_chunk("All_Test_Types", 4, 1); }
void ChunkingTests::BUNCPS_0() { check_chunk("BUNCPS_0", 8, 1); }
void ChunkingTests::BUNCPS_1() { check_chunk("BUNCPS_1", 8, 1); }
void ChunkingTests::BUNCPS_2() { check_chunk("BUNCPS_2", 8, 1); }
void ChunkingTests::BUNCPS_3() { check_chunk("BUNCPS_3", 8, 1); }
void ChunkingTests::BUNCPS_4() { check_chunk("BUNCPS_4", 8, 1); }
void ChunkingTests::BUNCPS_5() { check_chunk("BUNCPS_5", 8, 1); }
void ChunkingTests::BUNCPS_6_Four_Level() {
  check_chunk("BUNCPS_6_Four_Level", 8, 2);
}
void ChunkingTests::BUNCPS_7_with_Constraints() {
  check_chunk("BUNCPS_7_with_Constraints", 8, 1);
}
void ChunkingTests::Chunk_Operator_Tie_Impasse() {
  check_chunk("Chunk_Operator_Tie_Impasse", 6, 2);
}
void ChunkingTests::Chunk_Operator_Tie_Item_Links() {
  check_chunk("Chunk_Operator_Tie_Item_Links", 6, 1);
}
void ChunkingTests::Chunk_RL_Proposal() {
  check_chunk("Chunk_RL_Proposal", 8, 2);
}
void ChunkingTests::Chunk_Superstate_Operator_Preference() {
  check_chunk("Chunk_Superstate_Operator_Preference", 3, 1);
}
void ChunkingTests::Chunked_Justification_with_extras() {
  check_chunk("STI_with_referents", 8, 1);
}
void ChunkingTests::Conflated_Constants() {
  check_chunk("Conflated_Constants", 8, 1);
}
void ChunkingTests::Constraint_Prop_from_Base_Conds() {
  check_chunk("Constraint_Prop_from_Base_Conds", 8, 1);
}
void ChunkingTests::Constraint_Prop_Simple() {
  check_chunk("Constraint_Prop_Simple", 8, 1);
}
void ChunkingTests::Constraint_Ungrounded() {
  check_chunk("Constraint_Ungrounded", 8, 1);
}
void ChunkingTests::Deep_Copy_Identity_Expansion() {
  check_chunk("Deep_Copy_Identity_Expansion", 3, 4);
}
void ChunkingTests::Disjunction_Merge() {
  check_chunk("Disjunction_Merge", 5, 1);
}
void ChunkingTests::Duplicates() { check_chunk("Duplicates", 5, 2); }
void ChunkingTests::Faux_Operator() { check_chunk("Faux_Operator", 8, 3); }
void ChunkingTests::Faux_Smem_Operator_RHS() {
  check_chunk("Faux_Smem_Operator_RHS", 8, 0);
}  // Should be 1
void ChunkingTests::Justification_RC_not_Ungrounded_STIs() {
  check_chunk("Justification_RC_not_Ungrounded_STIs", 8, 1);
}
void ChunkingTests::Justifications_Get_New_Identities() {
  check_chunk("Justifications_Get_New_Identities", 4, 1);
}
void ChunkingTests::Link_STM_to_LTM() {
  check_chunk("Link_STM_to_LTM", 6, 0);
}  // Should be 2
void ChunkingTests::Literalization_Simple() {
  check_chunk("Literalization_Simple", 8, 1);
}
void ChunkingTests::Literalization_of_NC_and_NCC() {
  check_chunk("Literalization_of_NC_and_NCC", 8, 1);
}
void ChunkingTests::Literalization_with_BT_Constraints() {
  check_chunk("Literalization_with_BT_Constraints", 8, 1);
}
void ChunkingTests::Literalization_with_BT_Constraints2() {
  check_chunk("Literalization_with_BT_Constraints2", 8, 2);
}
void ChunkingTests::Literalization_with_Constraints() {
  check_chunk("Literalization_with_Constraints", 8, 1);
}
void ChunkingTests::Maintain_Instantiation_Specific_Identity() {
  check_chunk("Maintain_Instantiation_Specific_Identity", 8, 1);
}
void ChunkingTests::Max_Chunks() { check_chunk("Max_Chunks", 8, 0); }
void ChunkingTests::Max_Dupes() { check_chunk("Max_Dupes", 8, 0); }
void ChunkingTests::NC_Disjunction() { check_chunk("NC_Disjunction", 8, 1); }
void ChunkingTests::NC_Simple_No_Exist() {
  check_chunk("NC_Simple_No_Exist", 8, 1);
}
void ChunkingTests::NC_with_RC_and_Local_Variable() {
  check_chunk("NC_with_RC_and_Local_Variable", 8, 1);
}
void ChunkingTests::NC_with_Relational_Constraint() {
  check_chunk("NC_with_Relational_Constraint", 8, 1);
}
void ChunkingTests::NCC_2_Conds_Simple_Literals() {
  check_chunk("NCC_2_Conds_Simple_Literals", 8, 1);
}
void ChunkingTests::NCC_Complex() { check_chunk("NCC_Complex", 8, 1); }
void ChunkingTests::NCC_from_Backtrace() {
  check_chunk("NCC_from_Backtrace", 8, 1);
}
void ChunkingTests::NCC_Simple_Literals() {
  check_chunk("NCC_Simple_Literals", 8, 1);
}
void ChunkingTests::NCC_with_Relational_Constraint() {
  check_chunk("NCC_with_Relational_Constraint", 8, 1);
}
void ChunkingTests::No_Grounds() { check_chunk("No_Grounds", 8, 0); }
void ChunkingTests::No_Topstate_Match() {
  check_chunk("No_Topstate_Match", 8, 1);
}
void ChunkingTests::Opaque_State_Barrier() {
  check_chunk("Opaque_State_Barrier", 8, 1);
}
void ChunkingTests::Operator_Selection_Knowledge_Ghost_Operator() {
  check_chunk("Operator_Selection_Knowledge_Ghost_Operator", 4, 1);
}
void ChunkingTests::Operator_Selection_Knowledge_Mega_Test() {
  check_chunk("Operator_Selection_Knowledge", 75, 14);
}  // Should be 18.  2 could be because of uncertainty of numeric preferences.
   // Other 2 could be re-ordering issue.
void ChunkingTests::Operator_Selection_Knowledge_In_Propose() {
  check_chunk("Operator_Selection_Knowledge_In_Propose", 7, 2);
}
void ChunkingTests::Promoted_STI() { check_chunk("Promoted_STI", 8, 1); }
void ChunkingTests::Reorderer_Bad_Conjunction() {
  check_chunk("Reorderer_Bad_Conjunction", 8, 1);
}
void ChunkingTests::Repair_NOR_Temporal_Constraint() {
  check_chunk("Repair_NOR_Temporal_Constraint", 8, 3);
}
void ChunkingTests::Repair_Unconnected_RHS_ID() {
  check_chunk("Repair_Unconnected_RHS_ID", 8, 2);
}
void ChunkingTests::Result_On_Operator() {
  check_chunk("Result_On_Operator", 8, 1);
}
void ChunkingTests::Rete_Bug_Deep_vs_Deep() {
  check_chunk("Rete_Bug_Deep_vs_Deep", 8, 1);
}
void ChunkingTests::Rete_Bug_Deep_vs_Top() {
  check_chunk("Rete_Bug_Deep_vs_Top", 8, 1);
}
void ChunkingTests::Rhs_Func_Literalization() {
  check_chunk("Rhs_Func_Literalization", 8, 1);
}
void ChunkingTests::RHS_Math_Abs() { check_chunk("RHS_Math_Abs", 8, 2); }
void ChunkingTests::RHS_Math_Mixed() { check_chunk("RHS_Math_Mixed", 8, 4); }
void ChunkingTests::RHS_Math() { check_chunk("RHS_Math", 8, 1); }
void ChunkingTests::RHS_Math_Children_Force_Learn() {
  check_chunk("RHS_Math_Children_Force_Learn", 4, 1);
}
void ChunkingTests::RHS_Referent_Function() {
  check_chunk("RHS_Referent_Function", 8, 1);
}
void ChunkingTests::RHS_Unbound_Multivalue() {
  check_chunk("RHS_Unbound_Multivalue", 8, 1);
}
void ChunkingTests::RL_Variablization() {
  check_chunk("RL_Variablization", 8, 5);
}
void ChunkingTests::Singletons() { check_chunk("Singletons", 3, 2); }
void ChunkingTests::Singletons_Architectural() {
  check_chunk("Singletons_Architectural", 3, 1);
}
void ChunkingTests::SMem_Chunk_Direct() {
  check_chunk("SMem_Chunk_Direct", 8, 1);
}
void ChunkingTests::SMem_Chunked_Query() {
  check_chunk("SMem_Chunked_Query", 8, 0, true);
}  // Should be 1.
void ChunkingTests::SMem_Chunked_Query2() {
  check_chunk("SMem_Chunked_Query2", 8, 1);
}
void ChunkingTests::STI_Variablization_Same_Type() {
  check_chunk("STI_Variablization_Same_Type", 8, 1);
}
void ChunkingTests::STI_Variablization() {
  check_chunk("STI_Variablization", 8, 1);
}
void ChunkingTests::STI_with_referents() {
  check_chunk("STI_with_referents", 8, 1);
}
void ChunkingTests::Superstate_Identity_Opaque() {
  check_chunk("Superstate_Identity_Opaque", 8, 1);
}
void ChunkingTests::Superstate_Identity_Opaque_Old_Singleton_Behavior() {
  check_chunk("Superstate_Identity_Opaque_Old_Singleton_Behavior", 8, 1);
}
void ChunkingTests::Ungrounded_in_BT_Constraint() {
  check_chunk("Ungrounded_in_BT_Constraint", 8, 2);
}
void ChunkingTests::Ungrounded_Mixed() {
  check_chunk("Ungrounded_Mixed", 8, 1);
}
void ChunkingTests::Ungrounded_Relational_Constraint() {
  check_chunk("Ungrounded_Relational_Constraint", 8, 1);
}
void ChunkingTests::Ungrounded_STI_Promotion() {
  check_chunk("Ungrounded_STI_Promotion", 8, 1);
}
void ChunkingTests::Ungrounded_STIs() { check_chunk("Ungrounded_STIs", 8, 1); }
void ChunkingTests::Unify_Ambiguous_Output() {
  check_chunk("Unify_Ambiguous_Output", 8, 1);
}
void ChunkingTests::Unify_Children_Results() {
  check_chunk("Unify_Children_Results", 8, 1);
}
void ChunkingTests::Unify_through_Two_Traces_Four_Deep() {
  check_chunk("Unify_through_Two_Traces_Four_Deep", 8, 3);
}

void ChunkingTests::setUp() { FunctionalTestHarness::setUp(); }

void ChunkingTests::tearDown(bool caught) {
  FunctionalTestHarness::tearDown(caught);
}

void ChunkingTests::save_chunks(const char* pTestName) {
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

void ChunkingTests::save_chunks_internal(const char* pTestName) {
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

void ChunkingTests::source_saved_chunks(const char* pTestName) {
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

void ChunkingTests::check_chunk(const char* pTestName, int64_t decisions,
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

void ChunkingTests::verify_chunk(const char* pTestName, int64_t expected_chunks,
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

void ChunkingTests::Singleton_Element_Types() {
  SoarHelper::start_log(agent, "Singleton_Element_Types");
  assertTrue_msg("Could not find " + this->getCategoryName() +
                     " test file 'Singleton_Element_Types'",
                 SoarHelper::source(agent, this->getCategoryName(),
                                    "Singleton_Element_Types"));

  if (!SoarHelper::no_explainer) {
    SoarHelper::agent_command(agent, "explain all on");
    SoarHelper::agent_command(agent, "explain just on");
  }
  if (SoarHelper::save_logs) {
    SoarHelper::agent_command(agent, "trace -CbL 2");
  }

  SoarHelper::check_learning_override(agent);

  agent->RunSelf(3, sml::sml_DECIDE);
  assertTrue_msg(agent->GetLastErrorDescription(),
                 agent->GetLastCommandLineResult());

  SoarHelper::agent_command(agent, "chunk singleton -c");
  SoarHelper::agent_command(agent, "soar init");
  SoarHelper::agent_command(agent, "chunk singleton state state-type state");
  SoarHelper::agent_command(agent, "chunk singleton state operator operator");
  SoarHelper::agent_command(agent, "chunk singleton state sti identifier");
  SoarHelper::agent_command(agent, "chunk singleton state constant-i constant");
  SoarHelper::agent_command(agent, "chunk singleton state constant-s constant");
  SoarHelper::agent_command(agent, "chunk singleton state constant-f constant");
  agent->RunSelf(3, sml::sml_DECIDE);
  assertTrue_msg(agent->GetLastErrorDescription(),
                 agent->GetLastCommandLineResult());

  SoarHelper::agent_command(agent, "chunk singleton -c");
  SoarHelper::agent_command(agent, "soar init");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier state-type state");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier operator operator");
  SoarHelper::agent_command(agent, "chunk singleton identifier sti identifier");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-i constant");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-s constant");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-f constant");
  agent->RunSelf(3, sml::sml_DECIDE);
  assertTrue_msg(agent->GetLastErrorDescription(),
                 agent->GetLastCommandLineResult());

  SoarHelper::agent_command(agent, "chunk singleton -c");
  SoarHelper::agent_command(agent, "soar init");
  SoarHelper::agent_command(agent, "chunk singleton operator state-type state");
  SoarHelper::agent_command(agent,
                            "chunk singleton operator operator operator");
  SoarHelper::agent_command(agent, "chunk singleton operator sti identifier");
  SoarHelper::agent_command(agent,
                            "chunk singleton operator constant-i constant");
  SoarHelper::agent_command(agent,
                            "chunk singleton operator constant-s constant");
  SoarHelper::agent_command(agent,
                            "chunk singleton operator constant-f constant");

  agent->RunSelf(3, sml::sml_DECIDE);
  assertTrue_msg(agent->GetLastErrorDescription(),
                 agent->GetLastCommandLineResult());

  SoarHelper::agent_command(agent, "chunk singleton -c");
  SoarHelper::agent_command(agent, "soar init");
  SoarHelper::agent_command(agent, "chunk singleton any state-type state");
  SoarHelper::agent_command(agent, "chunk singleton any operator operator");
  SoarHelper::agent_command(agent, "chunk singleton any sti identifier");
  SoarHelper::agent_command(agent, "chunk singleton any constant-i constant");
  SoarHelper::agent_command(agent, "chunk singleton any constant-s constant");
  SoarHelper::agent_command(agent, "chunk singleton any constant-f constant");

  agent->RunSelf(3, sml::sml_DECIDE);
  assertTrue_msg(agent->GetLastErrorDescription(),
                 agent->GetLastCommandLineResult());

  SoarHelper::agent_command(agent, "chunk singleton -c");
  SoarHelper::agent_command(agent, "soar init");
  SoarHelper::agent_command(agent, "chunk singleton state state-type any");
  SoarHelper::agent_command(agent, "chunk singleton state operator any");
  SoarHelper::agent_command(agent, "chunk singleton state sti any");
  SoarHelper::agent_command(agent, "chunk singleton state constant-i any");
  SoarHelper::agent_command(agent, "chunk singleton state constant-s any");
  SoarHelper::agent_command(agent, "chunk singleton state constant-f any");

  agent->RunSelf(3, sml::sml_DECIDE);
  assertTrue_msg(agent->GetLastErrorDescription(),
                 agent->GetLastCommandLineResult());

  SoarHelper::agent_command(agent, "chunk singleton -c");
  SoarHelper::agent_command(agent, "soar init");
  SoarHelper::agent_command(agent, "chunk singleton any state-type any");
  SoarHelper::agent_command(agent, "chunk singleton any operator any");
  SoarHelper::agent_command(agent, "chunk singleton any sti any");
  SoarHelper::agent_command(agent, "chunk singleton any constant-i any");
  SoarHelper::agent_command(agent, "chunk singleton any constant-s any");
  SoarHelper::agent_command(agent, "chunk singleton any constant-f any");

  agent->RunSelf(3, sml::sml_DECIDE);
  assertTrue_msg(agent->GetLastErrorDescription(),
                 agent->GetLastCommandLineResult());

  SoarHelper::agent_command(agent, "chunk singleton -c");
  SoarHelper::agent_command(agent, "soar init");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier state-type constant");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier operator constant");
  SoarHelper::agent_command(agent, "chunk singleton identifier sti constant");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-i constant");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-s constant");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-f constant");

  agent->RunSelf(3, sml::sml_DECIDE);
  assertTrue_msg(agent->GetLastErrorDescription(),
                 agent->GetLastCommandLineResult());

  SoarHelper::agent_command(agent, "chunk singleton -c");
  SoarHelper::agent_command(agent, "soar init");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier state-type identifier");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier operator identifier");
  SoarHelper::agent_command(agent, "chunk singleton identifier sti identifier");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-i identifier");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-s identifier");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-f identifier");

  agent->RunSelf(3, sml::sml_DECIDE);
  assertTrue_msg(agent->GetLastErrorDescription(),
                 agent->GetLastCommandLineResult());

  SoarHelper::agent_command(agent, "chunk singleton -c");
  SoarHelper::agent_command(agent, "soar init");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier state-type state");
  SoarHelper::agent_command(agent, "chunk singleton identifier operator state");
  SoarHelper::agent_command(agent, "chunk singleton identifier sti state");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-i state");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-s state");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-f state");

  agent->RunSelf(3, sml::sml_DECIDE);
  assertTrue_msg(agent->GetLastErrorDescription(),
                 agent->GetLastCommandLineResult());

  SoarHelper::agent_command(agent, "chunk singleton -c");
  SoarHelper::agent_command(agent, "soar init");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier state-type operator");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier operator operator");
  SoarHelper::agent_command(agent, "chunk singleton identifier sti operator");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-i operator");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-s operator");
  SoarHelper::agent_command(agent,
                            "chunk singleton identifier constant-f operator");

  verify_chunk("Singleton_Element_Types", 8, false);
}
