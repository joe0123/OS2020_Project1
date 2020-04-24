cat OS_PJ1_Test/$1.txt
sudo dmesg -C
sudo ./main < OS_PJ1_Test/$1.txt > output/$1_stdout.txt 
sudo dmesg | grep Project1 > output/$1_dmesg.txt 
