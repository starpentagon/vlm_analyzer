# -*- coding: utf-8 -*-
import sys
import csv
import texttable

if __name__ == "__main__":
  argvs = sys.argv
  argc = len(argvs)

  if(argc != 2):
    print 'Usage %s (result csv file)' % argvs[0]
    quit()
  
  # data load
  result_filename = argvs[1]
  result_file = open(result_filename, 'rb')
  csv.field_size_limit(1000000)
  csv_file = csv.reader(result_file)

  csv_dict = {}
  header_dict = {}

  first_line = True

  for row in csv_file:
    if first_line:
      for i in range(len(row)):
        header_dict[i] = row[i]
        csv_dict[row[i]] = []
      
      first_line = False
      continue
    
    for i in range(len(row)):
      col_name = header_dict[i]
      csv_dict[col_name].append(row[i])

  # 集計(結果内訳)
  problem_count = len(csv_dict['Result'])

  solved_count = csv_dict['Result'].count("Solved")
  terminated_count = csv_dict['Result'].count("Terminated")
  disproved_count = csv_dict['Result'].count("Disproved")

  table = texttable.Texttable()
  table.add_row(['Result', 'Count', 'Rate(%)'])
  table.set_cols_dtype(['t', 't', 't'])
  table.set_cols_align(['l', 'r', 'r'])
  table.add_row(['Solved', "{:,.0f}".format(solved_count), "{:,.1f}".format(100.0 * solved_count / problem_count)])
  table.add_row(['Terminated', "{:,.0f}".format(terminated_count), "{:,.1f}".format(100.0 * terminated_count / problem_count)])
  table.add_row(['Disproved', "{:,.0f}".format(disproved_count), "{:,.1f}".format(100.0 * disproved_count / problem_count)])
  table.add_row(['Total', "{:,.0f}".format(problem_count), 100.0])

  print table.draw()
  print

  # 解図時間, ノード数, NPS
  col_list = ['Time(sec)', 'Nodes', 'NPS']

  result_mean = {}
  result_max = {}
  result_min = {}

  for col in col_list:
    while "INF" in csv_dict[col]:
      csv_dict[col].remove("INF")

    result_data = map(float, csv_dict[col])
    result_mean[col] = sum(result_data) / len(result_data)
    result_max[col] = max(result_data)
    result_min[col] = min(result_data)

  table = texttable.Texttable()
  table.add_row(['Stat', 'Mean', 'Max', 'Min'])
  table.set_cols_dtype(['t', 't', 't', 't'])
  table.set_cols_align(['l', 'r', 'r', 'r'])
  
  for col in col_list:
    if col == 'Time(sec)':
      table.add_row([col, "{:,.2f}".format(result_mean[col]), "{:,.2f}".format(result_max[col]), "{:,.2f}".format(result_min[col])])
    else:
      table.add_row([col, "{:,.0f}".format(result_mean[col]), "{:,.0f}".format(result_max[col]), "{:,.0f}".format(result_min[col])])

  print table.draw()
