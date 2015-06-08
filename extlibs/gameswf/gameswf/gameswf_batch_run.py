#!/usr/bin/python

# gameswf_batch_test.py -Thatcher Ulrich <tu@tulrich.com> 2005

# This source code has been donated to the Public Domain.  Do
# whatever you want with it.

# Script to interactively run all the .swf's in samples/ one-by-one.
#
# Pass a path argument to start with a particular test file.

import glob
import commands
import re
import sys

GAMESWF = "./gameswf_test_ogl"
BATCH_ARGS = " -v "


def run_swf(testfile):
  '''Run gameswf on a test file.

  Returns: True if gameswf exited with OK status, False otherwise.
  '''

  print "Running: " + testfile

  success = True;

  [status, output] = commands.getstatusoutput(GAMESWF + BATCH_ARGS + testfile)
  print output

  if (status != 0):
    success = False

  return success


# main
def main(argv):
  files = glob.glob("samples/*.swf")

  # If the user gave a filename, skip ahead until we find it in the list.
  if (len(argv) > 1):
    while (len(files) and argv[1] != files[0]):
      files.pop(0)

  for f in files:
    run_swf(f)


if __name__ == "__main__":
  main(sys.argv)
