for x in `ls tests/testCases/*.txt`; do
echo Running $x ...
echo load ${x} > input.txt
echo run 1 >> input.txt
echo run 2 >> input.txt
echo exit >> input.txt
./assignment1.exe < input.txt
done
