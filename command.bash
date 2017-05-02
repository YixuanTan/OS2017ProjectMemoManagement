g++ -Wall -Werror main.cpp

./a.out p2-test-input01.txt >> p2-test-output1.txt
./a.out p2-test-input02.txt >> p2-test-output2.txt
./a.out p2-test-input03.txt >> p2-test-output3.txt

diff p2-test-output1.txt p2-test-output01.txt
diff p2-test-output2.txt p2-test-output02.txt
diff p2-test-output3.txt p2-test-output03.txt

rm p2-test-output1.txt p2-test-output2.txt p2-test-output3.txt *.out
