from confluent_kafka import Producer
import jsonpickle

p = Producer({
    'bootstrap.servers': '127.0.0.1:9093,127.0.0.1:9094,127.0.0.1:9095',
    'security.protocol': 'SASL_PLAINTEXT',
    'sasl.mechanisms': 'PLAIN',
    'sasl.username': 'alice',
    'sasl.password': 'alicepassword'
    })

if __name__ == "__main__":
    
    res = p.list_topics("test")
    print(jsonpickle.encode(res))