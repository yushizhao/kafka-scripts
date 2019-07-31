from confluent_kafka import Producer
import jsonpickle

p = Producer({'bootstrap.servers': '127.0.0.1:9094'})

if __name__ == "__main__":
    
    res = p.list_topics("test")
    print(jsonpickle.encode(res))