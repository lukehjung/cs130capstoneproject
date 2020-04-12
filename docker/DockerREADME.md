# TO RUN
## Step 1: Build Base Ubuntu Image
1. In order to run docker, we need to run the 'base.Dockerfile' base image so we can build off the Ubuntu image on Docker.
2. To do this, run 'docker build -f docker/base.Dockerfile -t low-key-web-server:base .'
3. This builds the image from base.Dockerfile and tags it 'low-key-web-server:base'
4. Check if this works by seeing 'low-key-web-server:base' when you use 'docker images'

## Step 2: Build Server Image 
1. We then build our server image off of the low-key-web-server image by running
    docker build -f docker/Dockerfile -t webserver .
2. This builds our image, which we can check again by using 'docker images'
3. This also copies our executable to our new docker image and copies our conf/deploy.conf file into a docker container

## Step 3: Run Container
1. Run 'docker run --rm -p 8081:8081 --name my_run webserver:latest'
2. This runs the container in the background.
3. To kill this container, run 'docker container stop my_run' in another terminal

## Step 4: Testing
1. Go to another terminal and use 'nc localhost 8081'
2. Type any GET request or value and double enter.  You should get outputs like
GET google,com


HTTP/1.1 400 Bad Request
Content-Type: text/plain

GET google,com