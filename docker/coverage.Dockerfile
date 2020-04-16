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

ENV REPORT /usr/src/projects/low-key-web-server/build_coverage
RUN mkdir -p $REPORT/coverage

VOLUME	$REPORT/coverage

# keep running so we can copy over the coverage reports
ENTRYPOINT ["/usr/bin/tail", "-f", "/dev/null"]
