#!/usr/bin/env python3

import sys
import argparse
import traceback
import os
import re
import fnmatch
import difflib

reQuoteInclude = re.compile("\s*#include\s+\"(.*?)\"");
reAngleInclude = re.compile("\s*#include\s+<(.*?)>");
reIf = re.compile("\s*#if\s+(.*?)");
reIfdef = re.compile("\s*#ifdef\s+(.*?)");
reIfndef = re.compile("\s*#ifndef\s+([^\s]+)");
reEndif = re.compile("\s*#endif");
reDefine = re.compile("\s*#define\s+([^\s]+)\s+(\w+)");
reNamespace = re.compile("\s*namespace\s+([^\s]+)\s*\{");

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
            processHeader(filePath, input)
    return 0

  except Exception as e:
    sys.stderr.write(sys.argv[0] + ": " + repr(e) + "\n")
    traceback.print_exc()
    return 2

def processHeader(filePath, rootPath):
  relPath = os.path.relpath(filePath, rootPath)
  fh = open(filePath, "r")
  lines = fh.readlines()
  fh.close()
  modifiedLines = list(lines)
  nesting = 0
  firstHeaderLine = None
  namespaceLine = None
  headers = []
  for i, line in enumerate(lines):
    line = line.rstrip()

    if line.startswith("#if") and firstHeaderLine is None:
      firstHeaderLine = i

    m = reNamespace.match(line)
    if m and namespaceLine is None:
      namespaceLine = i

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

    if namespaceLine is None:
      m = reQuoteInclude.match(line)
      if m:
        headers.append(m.group(1))

      m = reAngleInclude.match(line)
      if m:
        headers.append(m.group(1))

  if nesting:
    print("Unmatched #if for file", filePath)
    return

  if firstHeaderLine and namespaceLine:
    newHeaders = []
    g = guard(relPath)
    newHeaders.append("#ifndef {0}\n".format(g))
    newHeaders.append("#define {0} 1\n".format(g))
    newHeaders.append("\n")
    for h in headers:
      if h.startswith("spark"):
        g = guard(h)
        newHeaders.append("#ifndef {0}\n".format(g))
        newHeaders.append("  #include \"{0}\"\n".format(h))
        newHeaders.append("#endif\n")
        newHeaders.append("\n")
      else:
        g = guard(h)
        newHeaders.append("#if SPARK_HAVE_{0}\n".format(g))
        newHeaders.append("  #include <{0}>\n".format(h))
        newHeaders.append("#endif\n")
        newHeaders.append("\n")

    modifiedLines[firstHeaderLine:namespaceLine] = newHeaders
    if modifiedLines[-1] == "\n":
      modifiedLines.pop()
    if not modifiedLines[-1].endswith("\n"):
      modifiedLines[-1] += "\n"
    if lines != modifiedLines:
      print("Updating:", relPath)
      #for d in difflib.unified_diff(lines, modifiedLines):
        #print(d, end="")
      fh = open(filePath, "w")
      fh.write("".join(modifiedLines))
      fh.close()

if __name__ == "__main__":
  sys.exit(main())
