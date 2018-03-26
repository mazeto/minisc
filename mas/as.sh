test $# -eq 1 && test -f $1 &&\

cat $1 | sed -f sed.txt > $1.tmp &&\

while read line; do
      echo -en "\x`echo -n $line`";
      done < $1.tmp > $1.bin &&\

rm $1.tmp &&\
echo done &&\
exit 0;

echo -e "\tusage: $0 src.asm" && exit 1

