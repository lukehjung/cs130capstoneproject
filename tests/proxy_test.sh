#!/bin/bash

binary_dir=$1
tests_dir=$2

frontend_conf=/usr/src/projects/low-key-web-server/conf/frontend.conf
backend_conf=/usr/src/projects/low-key-web-server/conf/backend.conf
front_port=8080
#start servers
$binary_dir/bin/webserver $backend_conf &
backend_server_pid=$!
sleep 0.05

$binary_dir/bin/webserver $frontend_conf &
frontend_server_pid=$!
sleep 0.05

exit_status=0

for file in `ls ./proxy_tests`
do
    if [[ $file != proxy_* ]]; then
        continue
    fi
  
      echo "$file:"
      timeout 0.5 nc localhost $front_port < ./proxy_tests/$file > full_response
      tail -n +6 < full_response > response_body
#      echo RESPONSE BODY:
#      cat response_body 
#      echo ACTUAL FILE: 
#      cat $tests_dir/data/www/hello.txt 
      sleep 0.1
      grep -q 200 < full_response || (echo FAIL && exit_status=1 && continue)
        
    if [[ $file == proxy_GET_txt ]]; then
        (cmp --silent response_body $tests_dir/data/www/hello.txt && echo PASS ) || (echo FAIL && exit_status=1)
    elif [[ $file == proxy_GET_zip ]]; then
        (cmp --silent response_body $tests_dir/data/www/test.zip && echo PASS ) || (echo FAIL && exit_status=1)
    elif [[ $file == proxy_GET_png ]]; then
        (cmp --silent response_body $tests_dir/data/www/colors.png && echo PASS ) || (echo FAIL && exit_status=1)
    elif [[ $file == proxy_GET_jpg ]]; then
        (cmp --silent response_body $tests_dir/data/www/ucla.jpg && echo PASS ) || (echo FAIL && exit_status=1)
    elif [[ $file == proxy_GET_binaryfile ]]; then
        (cmp --silent response_body $tests_dir/data/www/binaryfile && echo PASS ) || (echo FAIL && exit_status=1)
    else
        echo TEST_REQUEST_NOT_FOUND && exit_status=1
    fi
done

rm -f full_response
rm -f response_body
kill -9 $backend_server_pid
kill -9 $frontend_server_pid

exit $exit_status