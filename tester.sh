#!/bin/bash

time_start=$(date +%s%N)
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

for dir in $(ls ./open)
do
	cp ./open/${dir}/dimage.bin ./simulator/dimage.bin
	cp ./open/${dir}/iimage.bin ./simulator/iimage.bin
	cp ./open/${dir}/dimage.bin ./golden/dimage.bin
        cp ./open/${dir}/iimage.bin ./golden/iimage.bin
	cd ./golden
	./CMP > tmp
	if [ "$(cat tmp)" != "" ]; then
		cd ..;	
		echo -e "   Testcase: ${dir}\t- ${YELLOW}[Illegal]${NC}... ($(cat golden/tmp))"
		continue;
	fi
	cd ../simulator
	./CMP
	cd ..
	diff ./golden/snapshot.rpt ./simulator/snapshot.rpt > diff_snapshot.tmp
	diff ./golden/report.rpt ./simulator/report.rpt > report.tmp
	diff ./golden/trace.rpt ./simulator/trace.rpt > trace.tmp
	if [ "$(cat diff_snapshot.tmp)" = "" -a "$(cat trace.tmp)" = "" -a "$(cat report.tmp)" = "" ]; then
		echo -e "   Testcase: ${dir}\t- ${GREEN}[Accecpted]${NC}..."
	else
		echo -e "   Testcase: ${dir}\t- ${RED}[Wrong Answer]${NC}..."
		echo "${dir}" > who
		break
	fi
done

time_end=$(date +%s%N)
echo "   Total runtime: $(((time_end-time_start)/1000000))ms"