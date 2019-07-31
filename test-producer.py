from confluent_kafka import Producer

def delivery_report(err, msg):
    """ Called once for each message produced to indicate delivery result.
        Triggered by poll() or flush(). """
    if err is not None:
        print('Message delivery failed: {}'.format(err))
    else:
        print('Message delivered to {} [{}]'.format(msg.topic(), msg.partition()))

if __name__ == "__main__":

    p = Producer({
        'bootstrap.servers': '127.0.0.1:9093,127.0.0.1:9094,127.0.0.1:9095',
        'security.protocol': 'SASL_PLAINTEXT',
        'sasl.mechanisms': 'PLAIN',
        'sasl.username': 'admin',
        'sasl.password': 'admin'
        })

    for data in ("a","b","c"):
        # Trigger any available delivery report callbacks from previous produce() calls
        print("polling")
        p.poll(0)
        print("polled")
        # Asynchronously produce a message, the delivery report callback
        # will be triggered from poll() above, or flush() below, when the message has
        # been successfully delivered or failed permanently.
        p.produce('test', data.encode('utf-8'), key='235235', callback=delivery_report)

    # Wait for any outstanding messages to be delivered and delivery report
    # callbacks to be triggered.
    p.flush()