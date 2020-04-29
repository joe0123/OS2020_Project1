echo $1
sudo dmesg -C
sudo ./scheduler < "OS_PJ1_Test/"$1".txt" > "output/"$1"_stdout.txt"
sudo dmesg | grep Project1 > "output/"$1"_dmesg.txt"
./calc < "OS_PJ1_Test/"$1".txt" > "my_output/"$1"_theo.txt"
python3 diff.py $1
cat my_output/$1"_diff.txt"
