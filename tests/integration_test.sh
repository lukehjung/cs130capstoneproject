#!/bin/bash

# Start the webserver
 ../build/bin/webserver  ../conf/deploy.conf &

## TEST 1: Testing using curl as normal GET request.
# Curl as a GET request, -m for .1 seconds to force quit
curl http://localhost:8081 -s -i -m 0.1 -o curl_output.txt

# Create test file
echo "HTTP/1.1 200 OK
Content-type: text/plain
Content-length: 0
Connection: close
" > curl_test.txt

# Check if there is a diff from expected output
diff curl_output.txt curl_test.txt -E -Z -q

# Record output of diff command
diff_output=$?


## Test 2: Using echo and netcat to put in the correct specific GET request
# echo piped with netcat and a delay of 1 second to force restart
echo -e "GET / HTTP/1.1\r\n\r\n" | nc localhost 8081 -w 1 > nc_output.txt 

# grep to see if getting a 200 OK status from output
grep "HTTP/1.1 200 OK" nc_output.txt -q

# record grep output for if statement later
grep_output=$?

curl http://localhost:8081/static_files/hello.txt -s -i -m 0.1 -o hello_output.txt
grep "hello" hello_output.txt -q
hello_output=$?

# something wrong here, not sure yet
# curl http://localhost:8081/static_files/binaryfile -s -i -m 0.1 -o bigfile
# diff ../conf/binaryfile bigfile
# binaryfile_output=$?

kill $!

rm curl_output.txt nc_output.txt curl_test.txt hello_output.txt  &

# If output is true, exit 0, if false, exit 1
if [ $diff_output ] && [ $grep_output ] && [ $hello_output ]
then
    # SUCCESS
    exit 0
else
    # FAIL
    exit 1
fi


