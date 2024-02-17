#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
PANDA_DIR=$DIR/../../

GREEN="\e[1;32m"
NC='\033[0m'

: "${CPPCHECK_DIR:=$DIR/cppcheck/}"

# install cppcheck if missing
if [ -z "${SKIP_BUILD}" ]; then
  $DIR/install.sh
fi

# ensure checked in coverage table is up to date
cd $DIR
python $CPPCHECK_DIR/addons/misra.py -generate-table > new_table
if ! cmp -s new_table coverage_table; then
  echo "MISRA coverage table doesn't match. Update and commit:"
  echo "mv new_table coverage_table && git add . && git commit -m 'update table'"
  exit 1
fi

cd $PANDA_DIR
if [ -z "${SKIP_BUILD}" ]; then
  scons -j8
fi

cppcheck() {
  # note that cppcheck build cache results in inconsistent results as of v2.13.0
  OUTPUT=$DIR/.output.log
  $CPPCHECK_DIR/cppcheck --enable=all --force --inline-suppr -I $PANDA_DIR/board/ \
          -I $gcc_inc "$(arm-none-eabi-gcc -print-file-name=include)" \
          --suppressions-list=$DIR/suppressions.txt --suppress=*:*inc/* \
          --suppress=*:*include/* --error-exitcode=2 --addon=misra \
          --check-level=exhaustive "$@" |& tee $OUTPUT

  # cppcheck bug: some MISRA errors won't result in the error exit code,
  # so check the output (https://trac.cppcheck.net/ticket/12440#no1)
  if grep "misra violation" $OUTPUT > /dev/null; then
    exit 1
  fi
}

printf "\n${GREEN}** PANDA F4 CODE **${NC}\n"
cppcheck -DPANDA -DSTM32F4 -DUID_BASE $PANDA_DIR/board/main.c

printf "\n${GREEN}** PANDA H7 CODE **${NC}\n"
cppcheck -DPANDA -DSTM32H7 -DUID_BASE $PANDA_DIR/board/main.c

printf "\n${GREEN}Success!${NC} took $SECONDS seconds\n"
