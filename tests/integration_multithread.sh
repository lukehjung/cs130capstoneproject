#!/bin/bash
PATH_TO_BIN="../build/bin/webserver"
PATH_TO_CONFIG="/tmp/test_config"

echo "
port 8081;

location "/static_files" StaticHandler {
  root "./data/www";
}" > $PATH_TO_CONFIG

# fifo is named pipe
mkfifo fifo

# Run server
$PATH_TO_BIN $PATH_TO_CONFIG &
server_pid=$!
sleep .1

# Keep fifo open for multiple reads/writes
exec 3<> fifo

# Start up netcat connection to server
# Reads from fifo, writes to threads
nc localhost 8081 <fifo >> threads &
nc_pid=$!
sleep .1

# Send first half of a request to netcat
printf "GET /static_files/second.txt HTTP/1.1\r\nUser-Agent:" >&3
sleep .1

# Send full request and write response to threads
curl -s localhost:8081/static_files/first.txt >> threads

# Send second half of request to netcat
printf "nc/0.0.1\r\nHost: 127.0.0.1\r\nAccept: */*\r\n\r\n" >&3

# Close and remove fifo
exec 3>&-
rm fifo

first="$(grep -n "FIRST" threads)"
second="$(grep -n "SECOND" threads)"
echo $first
echo $second
rm threads

# Get line number of each response
first_linenum="$(echo $first | head -c 1)"
second_linenum="$(echo $second | head -c 1)"

kill $nc_pid
kill $server_pid

# Curl response should come before netcat response
if [ "$first_linenum" -gt "$second_linenum" ]; then
    echo "Responses out of order."
    exit 1
fi

echo "Responses in order."
exit 0
