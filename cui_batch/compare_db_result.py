#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import csv

if __name__ == "__main__":
  argvs = sys.argv
  argc = len(argvs)

  if(argc != 3):
    print 'Usage %s (problem db file) (result csv file)' % argvs[0]
    quit()
  
  # problem dbを読み込む
  problem_db_filename = argvs[1]
  problem_db_file = open(problem_db_filename, 'rb')
  problem_db_csv = csv.reader(problem_db_file)

  # result csvを読み込む
  result_filename = argvs[2]
  result_file = open(result_filename, 'rb')
  result_csv = csv.reader(result_file)

  # 問題数, 解図数のチェック
  problem_count = sum(1 for row in open(problem_db_filename, 'rb')) - 1  # header分を除く
  result_count = sum(1 for row in open(result_filename, 'rb')) - 1

  if(problem_count != result_count):
    print 'The problem db size(%d) is not equal to the result size(%d).' % (problem_count, result_count)
    quit()

  # ProblemDBのID, IntendedMoveNum, IntendedFirstMoveのリストを取得する
  problem_id_list = []
  problem_depth_list = []
  intended_first_move_list = []
  first_line = True
  
  id_index = 0
  depth_index = 0
  best_known_depth_index = 0
  intended_first_move_index = 0

  for row in problem_db_csv:
    if first_line:
      id_index = row.index('ID')
      depth_index = row.index('IntendedMoveNum')
      best_known_depth_index = row.index('BestKnownMoveNum')
      intended_first_move_index = row.index('IntendedFirstMove')
      first_line = False
      continue
    
    problem_id_list.append(row[id_index])
    intended_first_move_list.append(row[intended_first_move_index])

    # Best known move numが設定されている場合は優先的にその深さで判定を行う
    if row[best_known_depth_index] != '':
      problem_depth_list.append(row[best_known_depth_index])
    else:
      problem_depth_list.append(row[depth_index])
  
  # Result CSVのID, SearchedDepthのリストを取得する
  result_id_list = []
  result_depth_list = []
  result_first_move_list = []
  result_list = []
  result_dual_solution_list = []

  first_line = True

  for row in result_csv:
    if first_line:
      id_index = row.index('ID')
      result_index = row.index('Result')
      depth_index = row.index('SearchedDepth')
      first_move_index = row.index('FirstMove')
      dual_solution_index = row.index('DualSolution')

      first_line = False
      continue
    
    result_id_list.append(row[id_index])
    result_list.append(row[result_index])
    result_depth_list.append(row[depth_index])
    result_first_move_list.append(row[first_move_index])
    result_dual_solution_list.append(row[dual_solution_index])

  # 結果が整合性が取れているか確認する
  for i in range(problem_count):
    problem_id = problem_id_list[i]
    result_id = result_id_list[i]

    if problem_id != result_id:
      print "Problem ID(%s) is not equal to Result ID(%s)" % (problem_id, result_id)
      quit()
    
    result = result_list[i]

    # 手数チェック
    is_depth_check = problem_depth_list[i] != ''

    if result == "Solved" and is_depth_check:
      # 手数が一致しているかチェックする
      problem_depth = int(problem_depth_list[i])
      result_depth = int(result_depth_list[i])

      if result_depth < problem_depth:
        print "Problem ID(%s): result depth(%d) is less than intended depth(%d)" % (problem_id, result_depth, problem_depth)

      if result_depth > problem_depth:
        print "Problem ID(%s): result depth(%d) is more than intended depth(%d)" % (problem_id, result_depth, problem_depth)

      # 初手が一致しているかチェックする
      intended_first_move = intended_first_move_list[i]
      result_first_move = result_first_move_list[i]

      if intended_first_move != result_first_move:
        print "Problem ID(%s): result first move(%s) is not intended one(%s)" % (problem_id, result_first_move, intended_first_move)
      
      # 余詰が見つかった場合はメッセージを出す
      dual_solution = result_dual_solution_list[i]

      if dual_solution != '-':
        print "Problem ID(%s): Detect dual solution" % (problem_id)

    if result == "Terminated" and is_depth_check:
      # 意図した手数までに勝ちが見つかっていない場合はメッセージを出す
      problem_depth = int(problem_depth_list[i])
      result_depth = int(result_depth_list[i])

      if result_depth >= problem_depth:
        print "Problem ID(%s): result depth(>%d) is more than intended depth(%d)" % (problem_id, result_depth, problem_depth)

    # 不詰が証明された場合はメッセージを出す
    if result == "Disproved":
      print "Problem ID(%s) is disproved." % problem_id
      continue



