#!/bin/bash
ip=$1
port=$2
numclients=$3
loop=$4
sleeptime=$5
#gcc gradingserver.c -o server
#./server 5001 &
sleep 2
gcc gradlingclient.c -o submit

# Create a file to store the Throughput and response time values
throughput_file="throughput_data.txt"
response_time_file="response_time_data.txt"
job_ids=()
echo "Clients Throughput " > "$throughput_file"
echo "Clients AverageResponseTime" > "$response_time_file"
> total_time.log
>avg_cpu.txt
>avg_thread.txt
#for loop  for varying number of clients
for ((j=1; j<=numclients; j=j+5)); do

    >"cpu_utilization$j.log"
    >"threads$j.log"

    vm_id=0;
    wt_id=0;

    #to create the vmstat logic
    vmstat 10 >> "cpu_utilization$j.log" &
    vm_id=($!)

    # #to store the data in the file 
    watch -n 1 "ps -eLf | grep \"\./server \" | grep -v \"grep\" | wc -l >> threads$j.log" > /dev/null &
    wt_id=($!)

    start=$(date +%s.%N)

    for ((i=1; i<=j; i++)); do
        f_name="otpt_$j&$i.txt"
        ./submit "$ip":"$port" demo.cpp "$loop" "$sleeptime" > "$f_name" &
        job_ids+=("$!")
    done

    for job_id in "${job_ids[@]}"; do
        wait "$job_id"
        echo "$job_id"
    done
    end=$(date +%s.%N)
    dif=$(echo "scale=6;$end - $start" | bc)
    kill "$vm_id"

    kill "$wt_id"
    job_ids=()
    echo "$j $dif" >> "total_time.log"
done

# Kill the server after all iterations
killall server

for ((j=1; j<=numclients; j=j+5)); do
    th=0
    resTime=0
    res=0
    totalN=0
    totaltime=$(grep -E "^$j " 'total_time.log' | cut -d ' ' -f 2)
    # total_th=0
    for ((i=1; i<=j; i++)); do
        n=$(grep "The No of Successfull response" "otpt_$j&$i.txt" | cut -d ':' -f 2)
        th=$(grep "Throughput is :" "otpt_$j&$i.txt" | cut -d ':' -f 2)
        res=$(grep "Average time is" "otpt_$j&$i.txt" | cut -d ':' -f 2)
        resTime=$(echo "$res + $resTime" | bc -l)
        # total_th=$(echo "scale=6; $th+$total_th" | bc -l)
        totalN=$((totalN + n)) 
        rm -f "otpt_$j&$i.txt"
        #echo "i : $i , j : $j" 
    done
    th=$(echo "scale=6; $totalN / $totaltime" | bc -l)
    #th=$total_th
    res=$(echo "scale=6; $resTime / $j" | bc -l)
    # Append data to throughput file
    echo "$j $th" >> "$throughput_file"
    # Append data to response time file
    echo "$j $res" >> "$response_time_file"


    avg_active_threads=$(cat threads$j.log | awk '{sum+=$1} END {print sum/NR}')
    echo "$j $avg_active_threads" >> avg_thread.txt

    avg_cpu_util=$(cat cpu_utilization$j.log | awk '{sum+=$15} END {print (((NR-2)*100)-sum)/(NR-2)}')
    echo "$j $avg_cpu_util" >> avg_cpu.txt
done


# Use Gnuplot to create the throughput graph
gnuplot <<EOF
set terminal png
set output "throughput_graph.png"
set title "Number of Clients vs. Throughput"
set xlabel "Number of Clients"
set ylabel "Throughput"
set grid
plot "$throughput_file" using 1:2 with linespoints title "Throughput"
EOF

# Use Gnuplot to create the response time graph
gnuplot <<EOF
set terminal png
set output "response_time_graph.png"
set title "Number of Clients vs. Average Response Time"
set xlabel "Number of Clients"
set ylabel "Average Response Time(in seconds)"
set grid
plot "$response_time_file" using 1:2 with linespoints title "Average Response Time"
EOF

gnuplot <<EOF
set terminal png
set output "avg_thread.png"
set xlabel "Number of Clients"
set ylabel "Average Number of Therads"
set grid
plot "avg_thread.txt" using 1:2 with linespoints title "Average Number of Thread"
EOF

gnuplot <<EOF
set terminal png
set output "avg_cpu_utilization.png"
set title "Number of Clients vs. Average CPU Utilization"
set xlabel "Number of Clients"
set ylabel "Average CPU utilization"
set grid
plot "avg_cpu.txt" using 1:2 with linespoints title "Average CPU Utilization Time"
EOF

echo "Graphs created"
