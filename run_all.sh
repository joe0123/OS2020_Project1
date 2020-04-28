TM=TIME_MEASUREMENT
echo $TM
#sudo dmesg -C
#sudo ./scheduler < "OS_PJ1_Test/"$TM".txt" > "output/"$TM"_stdout.txt"
#sudo dmesg | grep Project1 > "output/"$TM"_dmesg.txt"
#./calc < "OS_PJ1_Test/"$TM".txt" > "my_output/"$TM"_theo.txt"
python3 diff.py $TM
cat my_output/$TM"_diff.txt"


for t in PSJF RR SJF FIFO; do
	for i in {1..5} ; do
		echo $t"_"$i
#		sudo dmesg -C
#		sudo ./scheduler < "OS_PJ1_Test/"$t"_"$i".txt" > "output/"$t"_"$i"_stdout.txt"
#		sudo dmesg | grep Project1 > "output/"$t"_"$i"_dmesg.txt"
#		./calc < "OS_PJ1_Test/"$t"_"$i".txt" > "my_output/"$t"_"$i"_theo.txt"
		python3 diff.py $t"_"$i
		cat my_output/$t"_"$i"_diff.txt"
	done
done
