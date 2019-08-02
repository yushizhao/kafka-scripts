# Confluent’s Python client for Apache Kafka

## 介绍

Confluent 在提供 kafka client libraries 时，是这样考虑的，

写好一个 c 的 client，也就是：

https://github.com/edenhill/librdkafka

其他语言的 client 都是 c 语言 client 的包装， 包括 python：

https://github.com/confluentinc/confluent-kafka-python

## 安装

首先添加confluent 的 apt 或 yum 源，参考:

https://docs.confluent.io/current/installation/installing_cp/index.html#installation-apt

接着安装 c 语言 client librdkafka

```
sudo apt-get install librdkafka-dev python-dev
sudo yum install librdkafka-devel python-devel
```

最后安装 python3 库

```
pip3 install --no-binary :all: confluent-kafka
```

这要比直接

```
pip3 install confluent-kafka
```

包含更多加密认证相关的功能

## 文档

https://docs.confluent.io/current/clients/confluent-kafka-python/index.html

python 库自带的文档比较简陋，更多内容包含在 c 语言库的文档之中：

https://github.com/edenhill/librdkafka/blob/master/src/rdkafka.h
https://github.com/edenhill/librdkafka/blob/master/src-cpp/rdkafkacpp.h

另外关于 consumer 和 producer 的配置有如下文档：

https://github.com/edenhill/librdkafka/blob/master/CONFIGURATION.md

## 使用

### Producer

https://docs.confluent.io/current/clients/confluent-kafka-python/index.html#producer

#### 推荐配置

```python
from confluent_kafka import Producer
p = Producer({
    	'bootstrap.servers': '127.0.0.1:9093,127.0.0.1:9094,127.0.0.1:9095',
    	'security.protocol': 'SASL_PLAINTEXT',
    	'sasl.mechanisms': 'PLAIN',
    	'sasl.username': 'alice',
    	'sasl.password': 'alicepassword'
    })
```

#### 主要方法

本节不能代替原文档，只是作为补充，使用方法前请确保查阅原文档。

+ **produce**

  + 示例

    ```python
    def delivery_report(err, msg):
    """ Called once for each message produced to indicate delivery result.
        Triggered by poll() or flush(). """
    if err is not None:
        print('Message delivery failed: {}'.format(err))
    else:
        print('Message delivered to {} [{}]'.format(msg.topic(), msg.partition()))

    p.produce(topic='test', value=data.encode('utf-8'), key='235235', callback=delivery_report)
    ```
  + 关于 key 

    决定这条消息被送去哪个 partition. 这也就是说, 如果一列消息要定序, 那么就意味着这列消息要在同一个 partition, 就可以用带同一个 key 来达到效果.

  + 关于 callback

    收到回报也不会自动执行,只有在执行 poll 或 flush 时才会执行
    
  + 关于 遇错重发

    message.send.max.retries 默认值为2，在此之外我们将进行手动重试发送, 保障发送成功. 当然, 这在特殊的情况下会导致消息重复以及消息乱序. 推荐设定自增主键以便 consumer 发现重复消息和跳过的消息. 

  + 关于 创建 topic

    produce 向一个不存在的 topic 时会尝试创建该 topic. 没有相应权限的话会失败.
  
+ **poll**

  + 示例

      ```python
      p.poll(timeout=0)
      ```
  
  + 关于 timeout

    秒单位阻塞进程, 会把之前以及阻塞时收到但还没有 callback 过的回报进行 callback. 输0时会立刻callback并返回. 推荐主进程中周期性的 poll(0) 或者用一个专门的进程.
  
+ **flush**

  + 示例

      ```python
      p.flush(timeout=60)
      ```
  
  + 与 poll 的关系

    flush 会连续调用 poll, 直到阻塞进程的总时间达到 timeout 值, 或者所有发出的消息都已经回报并 callback. 推荐在关闭某个 producer 前使用 flush.

### Consumer

https://docs.confluent.io/current/clients/confluent-kafka-python/index.html#consumer

#### 推荐配置

```python
from confluent_kafka import Consumer   
c = Consumer({
    'bootstrap.servers': '127.0.0.1:9094',
    'group.id': 'bob-group',
    'auto.offset.reset': 'earliest',
    'security.protocol': 'SASL_PLAINTEXT',
    'sasl.mechanisms': 'PLAIN',
    'sasl.username': 'bob',
    'sasl.password': 'bobpassword'
})
```

#### 主要方法

本节不能代替原文档，只是作为补充，使用方法前请确保查阅原文档。

+ **subscribe**

  + 示例

    ```python
    from confluent_kafka import Consumer, KafkaError, TopicPartition
    
    def print_partition(p):
        print(f"assigned: {p.topic}.{p.partition} at {p.offset}")
    
    def my_on_assign(consumer, partitions):
        for p in partitions:
                # some starting offset, or use OFFSET_BEGINNING, et, al.
                # the default offset is STORED which means use committed offsets, and if
                # no committed offsets are available use auto.offset.reset config (default latest)
            print_partition(p)
            # p.offset = 10
        # call assign() to start fetching the given partitions.
        # consumer.assign(partitions)
        
    c.subscribe(['test'],on_assign=my_on_assign)
    ```

  + 关于初始 offset

    这里 `on_assign` 中给出的 offset 是 -1001 (invalid offset). 这是 c 客户端固有的问题，详见：https://github.com/confluentinc/confluent-kafka-python/issues/406. 事实上，broker 已经把 group.id 对应的存在主题 __consumer_offsets 下的 offsets 分配给 consumer 了，只是 c 客户端的 consumer 在拉取一次数据之前都不知道这个 offset.
    
  + 关于自设 offset
  
    如果想要从指定的一个 offset 开始 consume，那么就要在 `on_assign` 中设置. 方法见 `my_on_assign` 中注释掉的最后三行。
  
+ **poll**

  + 示例

    ```python
    while True:
        msg = c.poll(timeout=0)
    
        if msg is None:
            continue
        if msg.error():
            print("Consumer error: {}".format(msg.error()))
            continue
    
        print('Received message: {}'.format(msg.value().decode('utf-8')))
    ```

  
  + 关于 timeout
  
    秒单位阻塞进程. 与 Producer 中的 poll 不同，一旦收到一条消息，就会立刻返回.
  
  + 关于返回值
  
    收到一条消息后返回一个 `confluent_kafka.Message`：
  
    https://docs.confluent.io/current/clients/confluent-kafka-python/index.html#message
    没有收到消息，因为 timeout 返回时返回值为 `NoneType`.
  
  + 关于 consume(num_messages=1,timeout=-1)
  
    Consumer 有另一个较旧的方法叫 consume，当 num_messages 设为1时，其行为和 poll 一致，换句话说 consume 是在收到一定数量的消息后返回. 但实际上不论 application 如何使用 poll 和 consume，在底层，python 客户端总是批量的从 Kafka broker 拉取消息，只是 application 碰不碰这些消息而已. 由于不必要的复杂性 consume 可能被弃用，参见：
  
    https://github.com/confluentinc/confluent-kafka-python/issues/580
  
  + 关于 offset
  
    默认配置下，poll 每收到一个消息将自动更新 offset. 即，at most once. 如果需要 at least once, 那么请关闭 `enable.auto.commit`，并在消息处理完成后手动调用 commit().
  
  + 关于 offset storage
     offset 将以主题 __consumer_offsets 存进 Kafka broker 中并和 group.id 关联. 一个 consumer 进行订阅时，将自动得到它所在的 group.id 在相应主题的分配给自己的 partition 上的 offset.
     
  + 关于 rebalance
    
     在一个 consumer group 中，即使知道没有消息也要定期进行 poll，不然会被认为是处理能力差的 consumer 导致 broker 进行 rebalance.
  
+ **close**

  + 示例

    ```python
    c.close()
    ```

    