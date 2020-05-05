#!/bin/sh

set -e

date
uname -a
echo "-----------------------------------------------------------------"
"$BLD/ltcencode" "$BLD/output.raw"
"$BLD/ltcdecode" "$BLD/output.raw" | diff -q "$SRC/expect_48k_2sec.txt" -
echo "-----------------------------------------------------------------"
"$BLD/ltcencode" "$BLD/output.raw" 192000
"$BLD/ltcdecode" "$BLD/output.raw" 7680 | diff -q "$SRC/expect_96k_2sec.txt" -
echo "-----------------------------------------------------------------"
"$BLD/ltcdecode" "$SRC/timecode.raw" 882 | diff -q "$SRC/timecode.txt" -
echo "-----------------------------------------------------------------"
echo "  $PRJ-$VER passed all tests."
echo "-----------------------------------------------------------------"
