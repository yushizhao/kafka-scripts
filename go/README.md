# confluent-kafka-go

## 介绍

Confluent 在提供 kafka client libraries 时, 是这样考虑的, 

写好一个 c 的 client, 也就是:

https://github.com/edenhill/librdkafka

其他语言的 client 都是 c 语言 client 的包装, 包括 go:

https://github.com/confluentinc/confluent-kafka-go

## 安装

首先添加 confluent 的 apt 或 yum 源, 参考:

https://docs.confluent.io/current/installation/installing_cp/index.html#installation-apt

接着安装 c 语言 client librdkafka

```
sudo apt-get install librdkafka-dev python-dev
sudo yum install librdkafka-devel python-devel
```

最后克隆 repo
git clone https://github.com/confluentinc/confluent-kafka-go.git

## 文档

https://docs.confluent.io/current/clients/confluent-kafka-go/index.html

结合 c 语言客户端文档

https://github.com/edenhill/librdkafka/blob/master/src/rdkafka.h
https://github.com/edenhill/librdkafka/blob/master/src-cpp/rdkafkacpp.h
https://github.com/edenhill/librdkafka/blob/master/CONFIGURATION.md

## 使用

### Producer

#### 推荐配置

```golang
p, err := kafka.NewProducer(&kafka.ConfigMap{
    "bootstrap.servers": "127.0.0.1:9093,127.0.0.1:9094,127.0.0.1:9095",
    "security.protocol": "SASL_PLAINTEXT",
    "sasl.mechanisms":   "PLAIN",
    "sasl.username":     "alice",
    "sasl.password":     "alicepassword",
})
```

#### 主要方法

本节不能代替原文档, 只是作为补充, 使用方法前请确保查阅原文档.

+ **Produce**

  + 示例

    ```golang
    // Produce messages to topic (asynchronously)
	topic := "test"
	for _, word := range []string{"Welcome", "to", "the", "Confluent", "Kafka", "Golang", "client"} {
		p.Produce(&kafka.Message{
			TopicPartition: kafka.TopicPartition{Topic: &topic, Partition: kafka.PartitionAny},
			Value:          []byte(word),
			Key:            []byte("asd"),
		}, nil)
    }
    
    // Delivery report handler for produced messages
	go func() {
		for e := range p.Events() {
			switch ev := e.(type) {
			case *kafka.Message:
				if ev.TopicPartition.Error != nil {
					fmt.Printf("Delivery failed: %v\n", ev.TopicPartition)
				} else {
					fmt.Printf("Delivered message to %v\n", ev.TopicPartition)
				}
			default:
				fmt.Println(ev.String())
			}
		}
	}()
    ```

  + 关于 创建 topic

    produce 向一个不存在的 topic 时会尝试创建该 topic. 没有相应权限的话会失败.

  + 关于 key

    决定这条消息被送去哪个 partition. 这也就是说, 如果一列消息要定序, 那么就意味着这列消息要在同一个 partition, 就可以用带同一个 key 来达到效果.

  + 关于 遇错重发

    message.send.max.retries 默认值为2, 在此之外我们将进行手动重试发送, 保障发送成功. 当然, 这在特殊的情况下会导致消息重复以及消息乱序. 推荐设定自增主键以便 consumer 发现重复消息和跳过的消息. 

  + 关于 回报

    回报将通过 channel 被 application 使用. 可以在 produce 时自设一个 channel, 否则该 Producer 的回报将一并送入它自带的 channel events 中.

+ **flush**

  + 示例

    ```golang
    // Wait for message deliveries before shutting down
	p.Flush(15 * 1000)
    ```

  + 关于 timeoutMs

    微秒单位阻塞进程, 如果所有的回报都通过了 channel, 就会立即返回.

### Consumer

#### 推荐配置

```golang
c, err := kafka.NewConsumer(&kafka.ConfigMap{
    "bootstrap.servers": "127.0.0.1:9094",
    "group.id":          "bob-group",
    "auto.offset.reset": "earliest",
    "security.protocol": "SASL_PLAINTEXT",
    "sasl.mechanisms":   "PLAIN",
    "sasl.username":     "bob",
    "sasl.password":     "bobpassword",
})
```

#### 主要方法

本节不能代替原文档, 只是作为补充, 使用方法前请确保查阅原文档.

+ SubscribeTopics

  + 示例

    ```golang
    func MyRebalanceCb(c *kafka.Consumer, event kafka.Event) error {
        fmt.Println(event.String())
        part, ok := event.(kafka.AssignedPartitions)
        if !ok {
            return fmt.Errorf("expecting kafka.AssignedPartitions, got %T", event)
        }

        var toBeAssigned []kafka.TopicPartition
        for _, tp := range part.Partitions {
            tp.Offset = 0
            toBeAssigned = append(toBeAssigned, tp)
        }
        return c.Assign(toBeAssigned)
    }

    c.SubscribeTopics([]string{"test"}, MyRebalanceCb)
    ```

  + 关于 RebalanceCb

    broker 接到 consumer 的订阅后返回分配给该 consumer 的 partition. 这个返回将触发 consumer 的 `RebalanceCb` 回调. 另外, 对一个 consumer group, broker 有时会进行 rebalance, 比如在这个 consumer group 增加或减少订阅者的时候, 这时也会触发 `RebalanceCb` 回调.
    
  + 关于自设 offset

    如果想要从指定的一个 offset 开始 consume, 那么就要在 `RebalanceCb` 中用 Assign() 设置. 详见示例.

+ ReadMessage
  
  + 示例

    ```golang
	for {
		// people wrap Poll() for some reasons
		msg, err := c.ReadMessage(-1)
		if err == nil {
			fmt.Printf("Message on %s: %s\n", msg.TopicPartition, string(msg.Value))
		} else {
			// The client will automatically try to recover from all errors.
			fmt.Printf("Consumer error: %v (%v)\n", err, msg)
		}
	}
    ```

  + 关于 Poll()

    ReadMessage() 内会调用 Poll() 并处理返回值. Poll() 到一些次要的消息时 ReadMessage() 是不会返回的, 详见函数注释.

  + 关于 timeout

    纳米单位阻塞进程. 收到一条消息后返回. 填 -1 会一直阻塞直到收到一条消息.

  + 关于 offset
  
    默认配置下, poll 每收到一个消息将自动更新 offset. 即, at most once. 如果需要 at least once, 那么请关闭 `enable.auto.commit`, 并在消息处理完成后手动调用 commit().
  
  + 关于 offset storage

    offset 将以主题 __consumer_offsets 存进 Kafka broker 中并和 group.id 关联. 一个 consumer 进行订阅时, 将自动得到它所在的 group.id 在相应主题的分配给自己的 partition 上的 offset.