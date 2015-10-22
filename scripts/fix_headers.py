#!/usr/bin/env python3

import sys
import argparse
import traceback
import os
import re
import fnmatch

reQuoteInclude = re.compile("\s*#include\s+\"(.*?)\"");
reAngleInclude = re.compile("\s*#include\s+<(.*?)>");
reIf = re.compile("\s*#if\s+(.*?)");
reIfdef = re.compile("\s*#ifdef\s+(.*?)");
reIfndef = re.compile("\s*#ifndef\s+([^\s])");
reEndif = re.compile("\s*#endif");
reDefine = re.compile("\s*#define\s+([^\s]+)\s+(\w+)");

def guard(path):
  return path.upper().replace(".", "_").replace("/", "_")

def main(argv=None):
  try:
    # setup option parser
    argParser = argparse.ArgumentParser(
        description="""Script to fix header guards in include files.\n
                       Copyright (c) 2015 Talin.\n
                       Licensed under the Apache License 2.0:\n
                           http://www.apache.org/licenses/LICENSE-2.0""")
    argParser.add_argument(
        "input", action="store", help="input files", nargs='+', metavar="FILE or DIR...")
    argParser.add_argument("-x", "--exclude",
        action="append", dest="excludes", help="exclusion patterns", metavar="PATTERN", default=[])

    # process options
    args = argParser.parse_args(argv)

    def is_excluded(path):
      for pattern in args.excludes:
        if fnmatch.fnmatch(path, pattern):
          return True
      return False

    for input in args.input:
      for dirpath, dirnames, filenames in os.walk(input):
        for name in filenames:
          filePath = os.path.join(dirpath, name)
          if name.endswith(".h") and not is_excluded(name):
            relPath = os.path.relpath(filePath, input)
            fh = open(filePath, "r")
            lines = fh.readlines()
            fh.close()
            modifiedLines = list(lines)
            nesting = 0
            guardSym = guard(relPath)
            seenGuard = False
            for i, line in enumerate(lines):
              if nesting == 1 and not seenGuard:
                mDefine = reDefine.match(line)
                if mDefine:
                  seenGuard = True
                  sym, value = mDefine.groups()
                  if guardSym != sym:
                    print("For file", filePath, "guard symbol should be", guardSym, "not", sym)
                    print(" ", sym, value)
                  continue

              m = reIf.match(line)
              if m:
                nesting += 1

              m = reIfdef.match(line)
              if m:
                nesting += 1

              m = reIfndef.match(line)
              if m:
                nesting += 1

              m = reEndif.match(line)
              if m:
                nesting -= 1

              line = line.strip()
              m = reQuoteInclude.match(line)
              if m:
                pass
                #print(" ", line)

              m = reAngleInclude.match(line)
              if m:
                pass
                #print(" ", line)
            if nesting:
              print("Unmatched #if for file", filePath)
    return 0

  except Exception as e:
    sys.stderr.write(sys.argv[0] + ": " + repr(e) + "\n")
    traceback.print_exc()
    return 2

if __name__ == "__main__":
  sys.exit(main())
