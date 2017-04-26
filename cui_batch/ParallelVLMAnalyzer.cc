#include "ParallelVLMAnalyzer.h"

using namespace std;
using namespace boost;
using namespace realcore;

ParallelVLMAnalyzer::ParallelVLMAnalyzer(const std::string &problem_db_path)
{
  const auto is_read = ReadCSV(problem_db_path, &problem_db_);

  if(!is_read){
    cerr << "Failed to read the problem db: " << problem_db_path << endl;
    return;
  }

  const auto problem_count = problem_db_["Board"].size();
  cerr << "Read: " << problem_count << " problems" << endl;

  for(size_t i=0; i<problem_count; i++){
    problem_index_list_.emplace_back(i);
  }
}

void ParallelVLMAnalyzer::Run(const size_t thread_num, const realcore::VLMSearch &vlm_search)
{
  assert(thread_num >= 1);
  thread_group thread_group;

  // 置換表を確保する
  vlm_table_list_.reserve(thread_num);
  
  for(size_t i=0; i<thread_num; i++){
    vlm_table_list_.emplace_back(make_shared<VLMTable>(kDefaultVLMTableSpace, kLockFree));
  }

  for(size_t i=0; i<thread_num; i++){
    thread_group.create_thread(bind(&ParallelVLMAnalyzer::VLMAnalyze, this, i, vlm_search));
  }

  thread_group.join_all();
}

const bool ParallelVLMAnalyzer::GetProblemIndex(size_t * const problem_index)
{
  mutex::scoped_lock lock(mutex_problem_index_list_);
  assert(problem_index != nullptr);

  if(problem_index_list_.empty()){
    return false;
  }

  *problem_index = problem_index_list_.front();
  problem_index_list_.pop_front();

  return true;
}

void ParallelVLMAnalyzer::VLMAnalyze(const size_t thread_id, const realcore::VLMSearch &vlm_search)
{
  const StringVector &board_list = problem_db_.at("Board");
  const StringVector &id_list = problem_db_.at("ID");
  const StringVector &name_list = problem_db_.at("Name");

  size_t problem_id = 0;
  bool exist_problem = GetProblemIndex(&problem_id);

  while(exist_problem){
    {
      const auto problem_info = id_list[problem_id] + "_" + name_list[problem_id];

      mutex::scoped_lock lock(mutex_cerr_);
      cerr << problem_info << endl;
    }

    const auto board_string = board_list[problem_id];
    MoveList board_sequence(board_string);

    if(!IsNonTerminateNormalSequence(board_sequence)){
      mutex::scoped_lock lock(mutex_cerr_);
      cerr << "The move sequence is not a non-terminal normal sequence: " << board_sequence.str() << endl;
      exist_problem = GetProblemIndex(&problem_id);
      continue;
    }

    const auto &vlm_table = vlm_table_list_[thread_id];
    vlm_table->Initialize();

    VLMAnalyzer vlm_analyzer(board_sequence, vlm_table);
    VLMResult vlm_result;

    vlm_analyzer.Solve(vlm_search, &vlm_result);
    Output(problem_id, vlm_analyzer, vlm_result);

    exist_problem = GetProblemIndex(&problem_id);
  }
}

void ParallelVLMAnalyzer::OutputHeader()
{
  cout << "ID,";
  cout << "Name,";
  cout << "Board,";
  cout << "Result,";
  cout << "FirstMove,";
  cout << "ProofTree,";
  cout << "SearchedDepth,";
  cout << "Time(sec),";
  cout << "Nodes,";
  cout << "NPS";
  cout << endl;
}

void ParallelVLMAnalyzer::Output(const size_t problem_id, const realcore::VLMAnalyzer &vlm_analyzer, const realcore::VLMResult &vlm_result) const
{
  const StringVector &id_list = problem_db_.at("ID");
  const StringVector &name_list = problem_db_.at("Name");
  const StringVector &board_list = problem_db_.at("Board");

  stringstream ss;

  // ID, Name, Board
  ss << id_list[problem_id] << ",";
  ss << name_list[problem_id] << ",";
  ss << board_list[problem_id] << ",";

  // Result
  if(vlm_result.solved){
    ss << "Solved";
  }else if(vlm_result.disproved){
    ss << "Disproved";
  }else{
    ss << "Terminated";
  }

  ss << ",";

  // FirstMove
  const string first_move_str = vlm_result.solved ? MoveString(vlm_result.proof_tree.GetTopNodeMove()) : "";
  ss << first_move_str << ",";

  // ProofTree
  const string proof_tree_str = vlm_result.solved ? vlm_result.proof_tree.str() : "";
  ss << proof_tree_str << ",";

  // SearchedDepth
  ss << vlm_result.search_depth << ",";
  
  const auto& search_manager = vlm_analyzer.GetSearchManager();

  // Time(sec)
  const auto search_time = search_manager.GetSearchTime() / 1000.0;
  ss << search_time << ",";

  // 探索ノード数
  const auto node_count = search_manager.GetNode();
  ss << node_count << ",";

  // 探索速度(NPS)
  if(search_time > 0){
    ss << node_count / search_time;
  }else{
    ss << "INF";
  }

  mutex::scoped_lock lock(mutex_cout_);
  cout << ss.str() << endl;
}
