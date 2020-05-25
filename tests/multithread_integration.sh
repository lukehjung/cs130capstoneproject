#!/bin/bash

PATH_TO_BIN="../build/bin/webserver"
PATH_TO_CONFIG="/tmp/test_config"

echo "
port 8081;

location "/echo" EchoHandler {
}" > $PATH_TO_CONFIG

$PATH_TO_BIN $PATH_TO_CONFIG &
pid_server=$!
sleep .1

# Send the first request with interval of 1 second between each \r\n
printf "GET /echo HTTP/1.1\r\nHost: www.host1.com\r\nConnection: close\r\n\r\n" | nc -i 1 localhost 8081 &
pid_incomplete_request=$!
sleep .1

# Send another request, which should complete before previous one.
# If this request has response, it means the server handles concurrenr requests
request=$(printf "GET /echo HTTP/1.1\r\nHost: www.host2.com\r\nConnection: close\r\n\r\n" | nc localhost 8081)
echo "RESPONSE 2:"
echo $request

kill -9 $pid_server
rm -f $PATH_TO_CONFIG

if [[ $request != "" ]]; then
    echo "CONCURRENCY TEST SUCCESS"
    exit 0
else
    echo "CONCURRENCY TEST FAILED"
    exit 1
fi
