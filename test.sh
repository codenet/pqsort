rm time.txt
make; make clean all

echo "---------------- 1 thread ---------------"
./run 1 
sleep 2

echo "---------------- 2 threads ---------------"
./run 2
sleep 2

echo "---------------- 4 threads ---------------"
./run 4
sleep 2

echo "---------------- 8 threads ---------------"
./run 8
sleep 2

echo "---------------- 7 threads ---------------"
./run 7
sleep 2

echo "---------------- 100 threads ---------------"
./run 100

python speedups.py
