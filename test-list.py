from confluent_kafka import Producer, Consumer, TopicPartition
import jsonpickle

p = Producer({
        'bootstrap.servers': '127.0.0.1:9093,127.0.0.1:9094,127.0.0.1:9095',
        'security.protocol': 'SASL_PLAINTEXT',
        'sasl.mechanisms': 'PLAIN',
        'sasl.username': 'admin',
        'sasl.password': 'adminpassword'
    })

c = Consumer({
        'bootstrap.servers': '127.0.0.1:9094',
        'group.id': 'bob-group',
        'auto.offset.reset': 'earliest',
        'security.protocol': 'SASL_PLAINTEXT',
        'sasl.mechanisms': 'PLAIN',
        'sasl.username': 'bob',
        'sasl.password': 'bobpassword'
    })

if __name__ == "__main__":
    
    tptest0 = TopicPartition(topic="test",partition=0)
    tptest1 = TopicPartition(topic="test",partition=1)
    tptest2 = TopicPartition(topic="test",partition=2)

    # res = p.list_topics()
    # print(jsonpickle.encode(res))

    tps = c.committed([tptest0,tptest1,tptest2])
    for tp in tps:
        print(tp.partition)
        print(tp.offset)

    # c.assign([TopicPartition(topic="test",partition=2,offset=0)])
    asss = c.assignment()
    print(asss)
    for ass in asss:
        print(ass.topic)
        print(ass.partition)
        print(ass.offset)