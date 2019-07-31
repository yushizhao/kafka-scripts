from confluent_kafka import Consumer, KafkaError


if __name__ == "__main__":

    c = Consumer({
        'bootstrap.servers': '127.0.0.1:9094',
        'group.id': 'mygroup',
        'auto.offset.reset': 'earliest',
        'security.protocol': 'SASL_PLAINTEXT',
        'sasl.mechanisms': 'PLAIN',
        'sasl.username': 'admin',
        'sasl.password': 'adminpassword'
    })

    c.subscribe(['test'])

    while True:
        msg = c.poll(1.0)

        if msg is None:
            continue
        if msg.error():
            print("Consumer error: {}".format(msg.error()))
            continue

        print('Received message: {}'.format(msg.value().decode('utf-8')))

    c.close()