#!/usr/bin/python

# gameswf_batch_test.py -Thatcher Ulrich <tu@tulrich.com> 2005

# This source code has been donated to the Public Domain.  Do
# whatever you want with it.

# Script for batch regression tests on gameswf.

import string
import sys
import commands
import difflib
import re

GAMESWF = "../dmb-out/vc9-debug/gameswf/gameswf_test_ogl"
BATCH_ARGS = " -r 0 -1 -t 10 -v "


def run_batch_test(testname, testfile, expected_output):
  '''Run gameswf on a test file, and compare its output to the given expected output.
  Return an error code and a report string summarizing the results'''

  report = "";
  success = True;

  [status, output] = commands.getstatusoutput(GAMESWF + BATCH_ARGS + testfile)

  # Clean up the output.
  output = output.splitlines(1)
  output = map(fix_string, output)  # lose trailing newlines, avoid DOS/Unix confusion

  expected_output = map(fix_string, expected_output)    # lose trailing newlines, avoid DOS/Unix confusion

  if (status != 0):
    success = False;
    report += format_header_line(testname, "[failed]")
    report += "  command returned status code " + str(status) + "\n"
    report += "  command output:\n"
    report += "    " + string.join(output, "    ")
  else:
    # Let's show the difference between expected and actual output
    difference = list(difflib.unified_diff(expected_output, output, "expected", "actual"))
    if (len(difference) == 0):
      report += format_header_line(testfile, "[OK]")
    else:
      success = False;
      report += format_header_line(testfile, "[failed]")
      report += "    " + string.join(difference, "    ")

  return success, report


def format_header_line(test_name, result_tag):
  '''Make a nice aligned summary line for the test'''
  padding = 70 - len(test_name)
  return test_name + ("." * padding) + result_tag + "\n"


def fix_string(s):
  '''strip trailing whitespace, add consistent newline'''
  return (string.rstrip(s) + '\n')

def next_non_comment_line(f):
  '''Read and return the next non-comment line from the given file.  If
  there are no more lines to read, return the empty string.'''
  while 1:
    line = f.readline()
    if len(line) == 0:
      # end of file.
      return line
    if line[0] != '#':
      return line

def parse_testfile(testfile):
  '''Given a test filename, returns the name of the test, the SWF
  filename for the test, and the expected output of the test.

  The name is just the base name of testfile.

  The SWF filename is taken from the first line of testfile.

  The expected output is taken from the remainder of testfile.

  Any lines in the testfile that start with '#' are comments, and are
  ignored.

  Returns [None,None,None] if the testfile couldn't be parsed.'''

  # Pull out the filename part of the testfile, minus any path and
  # extension.
  m = re.match("(.*\/)?([^\/\.]+)(\.[^\.]*)?$", testfile)
  testname = m.group(2)
    
  # Read the test file.
  f = file(testfile, "r")
  if not f:
    return [None, None, None]

  # The first non-comment line gives the swf file to run.
  swf_file = next_non_comment_line(f).rstrip()
  # The rest of the file gives the expected output.
  expected = []
  while 1:
    line = next_non_comment_line(f)
    if len(line) > 0:
      expected.append(line)
    else:
      break
  f.close()

  return testname, swf_file, expected


def do_tests(filenames):
  success_count = 0
  failed_count = 0
  report = ""

  for testfile in filenames:
    [testname, swf_file, expected_output] = parse_testfile(testfile)
    if testname == None:
      success = False
      rep = format_header_line(testfile, "[failed]\n")
      rep += "  Couldn't load test file %s\n" % testfile
    else:
      [success, rep] = run_batch_test(testname, swf_file, expected_output)

    if success:
      success_count += 1
    else:
      failed_count += 1
    report += rep

  sys.stdout.writelines("Test results: " + str(success_count) + "/" + str(success_count + failed_count) + "\n")
  sys.stdout.writelines(report)


# These tests should all pass.  If you break one of these, it's a
# regression.
passing_tests = [
  'tests/frame1.txt',
  'tests/frame2.txt',
  'tests/test_basic_types.txt',
  'tests/test_currentframe.txt',
  'tests/test_delete_references.txt',
  'tests/test_forin_array.txt',
  'tests/test_motion_exec_order.txt',
  'tests/test_string.txt',
  'tests/test_undefined_v6.txt',
  'tests/test_undefined_v7.txt',
  # Add more passing tests here, as gameswf improves.
  ]
  
# main

# Collect the tests.
if len(sys.argv) < 2:
  # No command-line args.  Print usage, and run all the passing tests.
  print "gameswf_batch_test.py:  runs automated tests against gameswf"
  print "usage:"
  print "  %s [list of test files]" % sys.argv[0]
  print "If no files are given, runs the list of known tests that should pass.\n"
  files = passing_tests
else:
  # Run the tests given on the command line.
  files = sys.argv[1:]

do_tests(files)
sys.exit(0)
