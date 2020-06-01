#!/bin/bash

# Build base image from base.Dockerfile
echo "** Build base image of Docker ** ";
docker build -f docker/base.Dockerfile -t low-key-web-server:base .;

# Build server image
echo "** Build server image of low-key-web-server image **";
docker build -f docker/Dockerfile -t webserver .;

# Save docker image
echo "** Saving the current docker image **";
docker save -o webserver_image webserver;

# Start using Google Cloud and enable some common APIs
echo "*** Log in ***";
gcloud init;

echo "** Enable common APIs ***";
gcloud services enable \
    cloudbuild.googleapis.com \
    compute.googleapis.com \
    containerregistry.googleapis.com \
    dns.googleapis.com \
    sourcerepo.googleapis.com;

# Start a build on Cloud Build using docker/cloudbuild.yaml
echo "** Start a build on Cloud Build **";
gcloud builds submit --config docker/cloudbuild.yaml .;

# Kill all docker container processes that are running
echo "** Kill all docker containers that are running **";
docker kill $(docker ps -a -q);

# Load new docker image
echo "** Loading new docker image ***";
docker load -i webserver_image;

# Run the docker image
echo "*** Run the docker image ***";
docker run --rm -t -p 8081:8081 webserver;
