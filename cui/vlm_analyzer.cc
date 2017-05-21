#include <iostream>
#include <chrono>

#include <boost/program_options.hpp>

#include "VLMAnalyzer.h"

using namespace std;
using namespace boost::program_options;
using namespace realcore;

string VLMResultString(const VLMAnalyzer &vlm_analyzer, const VLMResult &vlm_result);

int main(int argc, char* argv[])
{
  // オプション設定
  options_description option;

  option.add_options()
    ("string,s", value<string>(), "開始局面: [a-o]形式のテキスト")
    ("depth,d", value<VLMSearchDepth>()->default_value(7), "探索深さ(default: 7)")
    ("dual", "余詰を探索する")
    ("help,h", "ヘルプを表示");
  
  variables_map arg_map;
  store(parse_command_line(argc, argv, option), arg_map);

  bool is_help = arg_map.count("help") || !arg_map.count("string");

  if(is_help){
    cout << "Usage: " << argv[0] << " [options]" << endl;
    cout << option;
    cout << endl;
    return 0;
  }

  const auto board_string = arg_map["string"].as<string>();
  MoveList board_sequence(board_string);

  if(!IsNonTerminateNormalSequence(board_sequence)){
    cerr << "The move sequence is not a non-terminal normal sequence: " << board_sequence.str() << endl;
  }

  const auto search_depth = arg_map["depth"].as<VLMSearchDepth>();

  VLMAnalyzer vlm_analyzer(board_sequence);
  
  cerr << vlm_analyzer.GetSettingInfo() << endl;

  VLMSearch vlm_search;
  vlm_search.remain_depth = search_depth;
  vlm_search.detect_dual_solution = arg_map.count("dual");

  VLMResult vlm_result;

  vlm_analyzer.Solve(vlm_search, &vlm_result);

  cout << VLMResultString(vlm_analyzer, vlm_result);

  return 0;
}

string VLMResultString(const VLMAnalyzer &vlm_analyzer, const VLMResult &vlm_result)
{
  stringstream ss;
  
  // 探索結果
  ss << "Result: ";

  if(vlm_result.solved){
    ss << "Solved" << endl;

    ss << "First Move: " << MoveString(vlm_result.proof_tree.GetTopNodeMove()) << endl;
    ss << "Proof Tree: " << vlm_result.proof_tree.str();
  }else if(vlm_result.disproved){
    ss << "Disproved";
  }else{
    ss << "Terminated";
  }

  ss << endl;

  // 探索深さ
  ss << "Depth: " << vlm_result.search_depth << endl;

  const auto& search_manager = vlm_analyzer.GetSearchManager();

  // 探索ノード数
  const auto node_count = search_manager.GetNode();
  ss << "Nodes: " << node_count << endl;

  // 証明木
  const auto proof_tree_count = search_manager.GetProofTreeCount();
  const auto proof_tree_success_count = search_manager.GetProofTreeSuccessCount();
  const double proof_tree_success_rate = proof_tree_count == 0 ? 0.0 : round(1000.0 * proof_tree_success_count / proof_tree_count) / 10;

  ss << "ProofTree: " << proof_tree_success_count << " / " << proof_tree_count << " (" << proof_tree_success_rate << " %)" << endl;

  // Simulation
  const auto simulation_count = search_manager.GetSimulationCount();
  const auto simulation_success_count = search_manager.GetSimulationSuccessCount();
  const double simulation_success_rate = simulation_count == 0 ? 0.0 : round(1000.0 * simulation_success_count / simulation_count) / 10;

  ss << "Simulation: " << simulation_success_count << " / " << simulation_count << " (" << simulation_success_rate << " %)" << endl;

  // 探索時間
  const auto search_time = search_manager.GetSearchTime();
  ss << "Time(s): " << search_time / 1000.0 << endl;

  // 探索速度
  if(search_time > 0){
    ss << "NPS: " << 1000 * node_count / search_time << endl;
  }else{
    ss << "NPS: INF" << endl;
  }

  return ss.str();
}
