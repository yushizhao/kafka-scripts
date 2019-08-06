package main

import (
	"fmt"

	"github.com/confluentinc/confluent-kafka-go/kafka"
)

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

func ReadMessages(c *kafka.Consumer) {
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
}

func main() {

	c, err := kafka.NewConsumer(&kafka.ConfigMap{
		"bootstrap.servers": "127.0.0.1:9094",
		"group.id":          "bob-group",
		"auto.offset.reset": "earliest",
		"security.protocol": "SASL_PLAINTEXT",
		"sasl.mechanisms":   "PLAIN",
		"sasl.username":     "bob",
		"sasl.password":     "bobpassword",
	})

	if err != nil {
		panic(err)
	}

	c.SubscribeTopics([]string{"test"}, MyRebalanceCb)

	go ReadMessages(c)

	done := make(chan bool, 1)
	<-done
}
