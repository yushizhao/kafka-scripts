# kafka-scripts

## folder structure

```
.
|-- apache-zookeeper-3.5.5-bin
|-- kafka_2.12-2.3.0
|-- kafka-scripts
```

## prepare

copy following config files to kafka_2.12-2.3.0/config

+ kafka_server_jaas.conf
+ server-3-sasl.properties
+ server-4-sasl.properties
+ server-5-sasl.properties

## usage

### start.sh

Start zookeeper and kafka brokers.

### stop.sh

Stop kafka brokers.

### stop-zk.sh

Stop zookeeper.

### clean.sh

Remove zookeeper and kafka's data (znodes and logs).

### acl-add.sh

Add kafka acl.

### acl-list.sh

Shoe kafka acl.
