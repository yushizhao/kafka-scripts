#!/bin/bash

cd ../kafka_2.12-2.3.0

nohup bin/zookeeper-server-start.sh config/zookeeper.properties > nohup.zookeeper.out 2>&1 &

nohup bin/sasl-kafka-server-start.sh config/server-3-sasl.properties > nohup.kafka-3.out 2>&1 &
nohup bin/sasl-kafka-server-start.sh config/server-4-sasl.properties > nohup.kafka-4.out 2>&1 &
nohup bin/sasl-kafka-server-start.sh config/server-5-sasl.properties > nohup.kafka-5.out 2>&1 &