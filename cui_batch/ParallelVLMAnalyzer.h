#ifndef PARALELLE_VLM_ANALYZER_H
#define PARALELLE_VLM_ANALYZER_H

#include <deque>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>

#include "VLMAnalyzer.h"
#include "CSVReader.h"

class ParallelVLMAnalyzer
{
public:
  ParallelVLMAnalyzer(const std::string &problem_db_path);

  //! @brief 解図を並行実行する
  //! @param thread_num 同時実行数
  //! @param vlm_search 解図設定
  void Run(const size_t thread_num, const realcore::VLMSearch &vlm_search);

  //! @brief 解図結果のHeaderを出力する
  static void OutputHeader();
private:
  //! @param thread_id thread id
  //! @param vlm_search 解図設定
  void VLMAnalyze(const size_t thread_id, const realcore::VLMSearch &vlm_search);

  //! @brief 問題indexを取得する
  //! @retval true 問題indexを取得, false 問題indexのリストが空
  const bool GetProblemIndex(size_t * const problem_index);

  //! @brief 解図結果を出力する
  void Output(const size_t problem_id, const realcore::VLMAnalyzer &vlm_analyzer, const realcore::VLMResult &vlm_result) const;
  

  std::deque<size_t> problem_index_list_;    //!< 問題indexのリスト
  std::map<std::string, realcore::StringVector> problem_db_;   //!< VLM問題DB

  mutable boost::mutex mutex_cout_;   //!< 標準出力のmutex
  mutable boost::mutex mutex_cerr_;   //!< 標準エラー出力のmutex
  mutable boost::mutex mutex_problem_index_list_;   //!< problem_index_list_のmutex
};

#endif    // PARALELLE_VLM_ANALYZER_H
