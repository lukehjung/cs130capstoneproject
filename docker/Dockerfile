### Build/test container ###
# Define builder stage
FROM low-key-web-server:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:bionic as deploy

# TODO(!): Copy server output binary to "."
COPY --from=builder /usr/src/project/build/bin/webserver .
COPY --from=builder /usr/src/project/conf/deploy.conf .

COPY --from=builder /usr/src/project/conf/hello.txt /data/www/hello.txt
COPY --from=builder /usr/src/project/conf/something.html /data/www/something.html
COPY --from=builder /usr/src/project/conf/binaryfile /data/www/binaryfile
COPY --from=builder /usr/src/project/conf/test.zip /data/www/test.zip

COPY --from=builder /usr/src/project/conf/ucla.jpg /data/ucla.jpg
COPY --from=builder /usr/src/project/conf/test.png /data/test.png
COPY --from=builder /usr/src/project/conf/colors.png /data/colors.png

# TODO(!): Expose some port(s)
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
# TODO(!): Update with real server name
ENTRYPOINT ["./webserver"]

# Use CMD to specify arguments to ENTRYPOINT
# TODO(!): Update with real server args
CMD ["./deploy.conf"]
