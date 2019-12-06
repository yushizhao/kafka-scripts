#!/bin/bash

cd ../kafka_2.12-2.3.0

nohup bin/zookeeper-server-start.sh config/zookeeper.properties > nohup.zookeeper.out 2>&1 &

nohup bin/kafka-server-start.sh config/server-single.properties > nohup.kafka.out 2>&1 &

