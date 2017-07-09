#include <iostream>
#include <chrono>

#include <boost/program_options.hpp>

#include "VLMAnalyzer.h"
#include "SGFParser.h"

using namespace std;
using namespace boost::program_options;
using namespace realcore;

string VLMResultString(const variables_map &arg_map, const MoveList &board_move_list, const VLMAnalyzer &vlm_analyzer, const VLMSearch &vlm_search, const VLMResult &vlm_result);
string GetTreeSGFText(const variables_map &arg_map, const MoveList &board_move_list, const MoveTree &tree);

int main(int argc, char* argv[])
{
  // オプション設定
  options_description option;

  option.add_options()
    ("string,s", value<string>(), "開始局面: [a-o]形式のテキスト")
    ("sgf-input", value<string>(), "開始局面: SGF形式のテキスト")
    ("pos", value<string>(), "POS形式ファイル名")
    ("depth,d", value<VLMSearchDepth>()->default_value(7), "探索深さ(四手五連:5, 五手五連:7)")
    ("dual", "余詰/最善応手を探索する")
    ("sgf-output", "探索結果をSGF結果で出力する")
    ("help,h", "ヘルプを表示");
  
  variables_map arg_map;
  store(parse_command_line(argc, argv, option), arg_map);

  size_t input_count = (arg_map.count("string") ? 1 : 0) + (arg_map.count("pos") ? 1 : 0) + (arg_map.count("sgf-input") ? 1 : 0);
  bool is_help = arg_map.count("help") || input_count != 1;

  if(is_help){
    cout << "Usage: " << argv[0] << " [options]" << endl;
    cout << option;
    cout << endl;
    return 0;
  }

  MoveList board_sequence;

  if(arg_map.count("string")){
    const auto board_string = arg_map["string"].as<string>();
    board_sequence = MoveList(board_string);
  }else if(arg_map.count("sgf-input")){
    const auto sgf_data = arg_map["sgf-input"].as<string>();
    GetMoveListFromSGFData(kSGFCheckNone, sgf_data, &board_sequence);
  }else{
    const auto pos_file = arg_map["pos"].as<string>();
    ReadPOSFile(pos_file, &board_sequence);
  }

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

  cout << VLMResultString(arg_map, board_sequence, vlm_analyzer, vlm_search, vlm_result);

  return 0;
}

string VLMResultString(const variables_map &arg_map, const MoveList &board_move_list, const VLMAnalyzer &vlm_analyzer, const VLMSearch &vlm_search, const VLMResult &vlm_result)
{
  stringstream ss;
  
  // 探索結果
  ss << "Result: ";

  if(vlm_result.solved){
    ss << "Solved" << endl;

    ss << "First Move: " << MoveString(vlm_result.proof_tree.GetTopNodeMove()) << endl;

    if(vlm_search.detect_dual_solution){
      ss << "Best response: " << vlm_result.best_response.str() << endl;
    }else{
        ss << "Best response: Unknown" << endl;
    }
    
    const bool is_sgf = arg_map.count("sgf-output");

    if(is_sgf){
      ss << "Proof Tree: " << endl;
      ss << "\t" << GetTreeSGFText(arg_map, board_move_list, vlm_result.proof_tree) << endl;
    }else{
      ss << "Proof Tree: " << vlm_result.proof_tree.str() << endl;
    }

    if(vlm_search.detect_dual_solution){
      if(vlm_result.detect_dual_solution){
        if(is_sgf){
          ss << "Dual Solution: " << endl;
          ss << "\t" << GetTreeSGFText(arg_map, board_move_list, vlm_result.dual_solution_tree) << endl;
        }else{
          ss << "Dual Solution: " << vlm_result.dual_solution_tree.str();
        }
      }else{
        ss << "Dual Solution: -";
      }
    }else{
        ss << "Dual Solution: Unknown";
    }
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

string GetTreeSGFText(const variables_map &arg_map, const MoveList &board_move_list, const MoveTree &tree)
{
  stringstream ss;

  ss << "(;GM[4]FF[4]SZ[15]";
  ss << board_move_list.GetSGFPositionText();
  ss << (board_move_list.IsBlackTurn() ? "PL[B]" : "PL[W]");
  ss << tree.GetSGFLabeledText(board_move_list.IsBlackTurn());
  ss << ")";
  
  return ss.str();
}
