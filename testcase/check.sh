./i_generator
./d_generator
cp dimage.bin ../golden/dimage.bin
cp iimage.bin ../golden/iimage.bin
cp dimage.bin ../simulator/dimage.bin
cp iimage.bin ../simulator/iimage.bin

cd ../golden
./CMP
cd ../simulator
./CMP
cd ../testcase

echo -------------diff-------------
diff -c ../golden/snapshot.rpt ../simulator/snapshot.rpt
diff -c ../golden/trace.rpt ../simulator/trace.rpt
diff -c ../golden/report.rpt ../simulator/report.rpt
