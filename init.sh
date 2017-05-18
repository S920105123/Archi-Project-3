chmod 777 golden/single_cycle
cd simulator
sh cr.sh
cd ../testcase
g++ -std=c++11 d_generator.cpp -o d_generator
g++ -std=c++11 i_generator.cpp -o i_generator
cd ..
