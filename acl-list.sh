#!/bin/bash

cd ../kafka_2.12-2.3.0

bin/kafka-acls.sh --authorizer kafka.security.auth.SimpleAclAuthorizer --authorizer-properties zookeeper.connect=localhost:2181 --list