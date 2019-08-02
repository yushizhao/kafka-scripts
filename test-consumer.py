from confluent_kafka import Consumer, KafkaError, TopicPartition

def print_partition(p):
    print(f"assigned: {p.topic}.{p.partition} at {p.offset}")

def print_msg(msg):
    print(f"received: {msg.key()} : {msg.value()} of {msg.topic()}.{msg.partition()} at {msg.offset()}")

def my_on_assign(consumer, partitions):
    for p in partitions:
            # some starting offset, or use OFFSET_BEGINNING, et, al.
            # the default offset is STORED which means use committed offsets, and if
            # no committed offsets are available use auto.offset.reset config (default latest)
        print_partition(p)
        # p.offset = 10
    # call assign() to start fetching the given partitions.
    # consumer.assign(partitions)

if __name__ == "__main__":

    c = Consumer({
        'bootstrap.servers': '127.0.0.1:9094',
        'group.id': 'bob-group',
        'auto.offset.reset': 'earliest',
        'security.protocol': 'SASL_PLAINTEXT',
        'sasl.mechanisms': 'PLAIN',
        'sasl.username': 'bob',
        'sasl.password': 'bobpassword'
    })

    c.subscribe(['test'],on_assign=my_on_assign)

    while True:
        msg = c.poll(1.0)

        if msg is None:
            continue
        if msg.error():
            print("Consumer error: {}".format(msg.error()))
            continue

        print_msg(msg)
        
    c.close()