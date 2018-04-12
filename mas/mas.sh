# Macro assembler

[[ $# -eq 1 ]] && [[ -f $1 ]] && {
  cat $1 | sed -E -f sed.txt > $1.tmp
  while read line; do
      echo -en "\x`echo -n $line`";
      done < $1.tmp > $1.bin
  rm $1.tmp
  echo done
  exit 0
} || echo -e "\tusage: $0 src.asm"

