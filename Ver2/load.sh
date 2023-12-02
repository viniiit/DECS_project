#!/bin/bash
ip=$1
port=$2
numclients=$3
a=$3
loop=$4
sleeptime=$5
times=$6
#gcc simple-server.c -o server
#./server 5003 &
sleep 2
gcc gradingclient.c -o submit
# Create a file to store the Throughput and response time values
throughput_file="throughput_data.txt"
response_time_file="response_time_data.txt"
echo "Clients Throughput AverageResponseTime" > "$throughput_file"
echo "Clients AverageResponseTime" > "$response_time_file"

>total_time.log
>avg_cpu.txt
>avg_thread.txt
>error_rate.txt
>timeout_rate.txt
>goodput_rate.txt
>request_sent_rate.txt

# Create an array to store background job IDs
declare -a job_ids

#for loop  for varying number of clients
for ((j=1; j<=numclients; j++)); do

    >"cpu_utilization$j.log"
    >"threads$j.log"

    # Create an array to store background job IDs
    unset job_ids
    vm_id=0;
    wt_id=0;

    #to create the vmstat logic
    vmstat 10 >> "cpu_utilization$j.log" &
    vm_id=($!)

    # #to store the data in the file 
    watch -n 1 "ps -eLf | grep \"\./server \" | grep -v \"grep\" | wc -l >> threads$j.log" > /dev/null &
    wt_id=($!)

    #to calculate start time
    start=$(date +%s.%N)

    for ((i=1; i<=j; j=j+5)); do
        ./submit "$ip":"$port" demo.cpp "$loop" "$sleeptime" "$times" > "otpt_${j}${i}.txt" &
        job_ids+=($!)
        sleep 1
    done

    for job_id in "${job_ids[@]}"; do
        wait "$job_id"
    done

    end=$(date +%s.%N)
    dif=$(echo "scale=6;$end - $start" | bc)
    kill "$vm_id"

    kill "$wt_id"

    job_ids=()
    echo "$j $dif" >> "total_time.log"


done

# Kill the server after all iterations
killall "server"

for ((j=1; j<=numclients; j=j+5)); do
    th=0
    resTime=0
    res=0
    totalN=0
    error=0
    total_error=0
    error_rate=0
    timeout=0
    total_timeout=0
    goodput=0
    request_sent_rate=0
    avg_res=0
    request=0
    totaltime=$(grep -E "^$j " 'total_time.log' | cut -d ' ' -f 2)
    for ((i=1; i<=j; i++)); do
        
        n=$(grep "The No of Successfull response" "otpt_$j$i.txt" | cut -d ':' -f 2)
        totalN=$((totalN + n)) 
        
        # res=$(grep "Total Response time is" "otpt_$j$i.txt" | cut -d ':' -f 2)
        # resTime=$(echo "$res + $resTime" | bc -l)
        
        avg_res=$(grep "Average time is" "otpt_$j$i.txt" | cut -d ':' -f 2)
        resTime=$(echo "$avg_res + $resTime" | bc -l)

        error=$(grep "ERROR connecting" "otpt_$j$i.txt" | wc -l)  
        total_error=$((total_error + error))
        
        timeout=$(grep "Timeout" "otpt_$j$i.txt"| wc -l)
        total_timeout=$(echo "$total_timeout + $timeout" | bc -l)

        rm "otpt_$j$i.txt"
    done

    request=$((totalN + total_error + total_timeout))
    th=$(echo "scale=6; $request / $totaltime" | bc -l)

    res=$(echo "scale=6; $resTime/$j" | bc -l)

    # avg_res=$(echo "scale=6; $total_avg_res/$totalN / 1000000" | bc -l)
    # res=$(echo "scale=6; $resTime / 1000000" | bc -l)

    goodput=$(echo "scale=6; $totalN / $totaltime" | bc -l)

    error_rate=$(echo "scale=6; $total_error/$totaltime" | bc -l)

    timeout_rate=$(echo "scale=6; $total_timeout / $totaltime" | bc -l)

    request_sent_rate=$(echo "scale=6; $th+$timeout_rate+$error_rate" | bc -l)

    #append data to the request_sent_rate file
    echo "$j $request_sent_rate" >> request_sent_rate.txt

    #append data to the goodput file
    echo "$j $goodput" >> goodput_rate.txt
    
    #append data to the timeoutrate file
    echo "$j $timeout_rate" >> timeout_rate.txt
    
    #append data to error rate file
    echo "$j $error_rate" >> error_rate.txt
    
    # Append data to throughput file
    echo "$j $th" >> "$throughput_file"
    
    # Append data to response time file
    echo "$j $res" >> "$response_time_file"

    avg_active_threads=$(cat threads$j.log | awk '{sum+=$1} END {print sum/NR}')
    echo "$j $avg_active_threads" >> avg_thread.txt

    avg_cpu_util=$(cat cpu_utilization$j.log | awk '{sum+=$15} END {print (((NR-2)*100)-sum)/(NR-2)}')
    echo "$j $avg_cpu_util" >> avg_cpu.txt

done

# Use Gnuplot to create the Successfull_req graph
gnuplot <<EOF
set terminal png
set output "Succesfull_req_graph.png"
set title "Number of Clients vs. Succesful_request_Rate"
set xlabel "Number of Clients"
set ylabel "Timeout-rate(per second)"
set grid
plot "goodput_rate.txt" using 1:2 with linespoints title "Successfull_request_Rate"
EOF

echo "goodput graph comleted"

# Use Gnuplot to create the throughput graph
gnuplot <<EOF
set terminal png
set output "request_sent_graph.png"
set title "Number of Clients vs. Request-Sent-Rate"
set xlabel "Number of Clients"
set ylabel "Request-sent-rate(per second)"
set grid
plot "request_sent_rate.txt" using 1:2 with linespoints title "Request-Sent-Rate"
EOF

echo "requestsent graph comleted"

# Use Gnuplot to create the timeout_rate graph
gnuplot <<EOF
set terminal png
set output "timeout_graph.png"
set title "Number of Clients vs. Timeout-Rate"
set xlabel "Number of Clients"
set ylabel "Timeout-rate(persecond)"
set grid
plot "timeout_rate.txt" using 1:2 with linespoints title "Timeout"
EOF

echo "timeout graph comleted"

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

echo "through graph comleted"

# Use Gnuplot to create the Error_rate graph
gnuplot <<EOF
set terminal png
set output "error_rate_graph.png"
set title "Number of Clients vs. Error Rate"
set xlabel "Number of Clients"
set ylabel "Error Rate(per second)"
set grid
plot "error_rate.txt" using 1:2 with linespoints title "Error rate"
EOF

# Use Gnuplot to create the response time graph
gnuplot <<EOF
set terminal png
set output "response_time_graph.png"
set title "Number of Clients vs. Average Response Time"
set xlabel "Number of Clients"
set ylabel "Average Response Time '(in seconds)'"
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

echo "Graphs created: throughput_graph.png and response_time_graph.png"



