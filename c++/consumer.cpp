#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>

#include <librdkafka/rdkafkacpp.h>

static bool run = true;

class ExampleEventCb : public RdKafka::EventCb {
 public:
  void event_cb (RdKafka::Event &event) {

    switch (event.type())
    {
      case RdKafka::Event::EVENT_ERROR:
        if (event.fatal()) {
          std::cerr << "FATAL ";
          run = false;
        }
        std::cerr << "ERROR (" << RdKafka::err2str(event.err()) << "): " <<
            event.str() << std::endl;
        break;

      case RdKafka::Event::EVENT_STATS:
        std::cerr << "\"STATS\": " << event.str() << std::endl;
        break;

      case RdKafka::Event::EVENT_LOG:
        fprintf(stderr, "LOG-%i-%s: %s\n",
                event.severity(), event.fac().c_str(), event.str().c_str());
        break;

      case RdKafka::Event::EVENT_THROTTLE:
	std::cerr << "THROTTLED: " << event.throttle_time() << "ms by " <<
	  event.broker_name() << " id " << (int)event.broker_id() << std::endl;
	break;

      default:
        std::cerr << "EVENT " << event.type() <<
            " (" << RdKafka::err2str(event.err()) << "): " <<
            event.str() << std::endl;
        break;
    }
  }
};

class ExampleRebalanceCb : public RdKafka::RebalanceCb {
private:
  static void part_list_print (const std::vector<RdKafka::TopicPartition*>&partitions){
    for (unsigned int i = 0 ; i < partitions.size() ; i++)
      std::cerr << "RebalanceCb:" << partitions[i]->topic() <<
	"[" << partitions[i]->partition() << "], ";
    std::cerr << "\n";
  }

public:
void rebalance_cb (RdKafka::KafkaConsumer *consumer,
        	      RdKafka::ErrorCode err,
                  std::vector<RdKafka::TopicPartition*> &partitions) {
        
        part_list_print(partitions);
        
        if (err == RdKafka::ERR__ASSIGN_PARTITIONS) {
            // application may load offets from arbitrary external
            // storage here and update \p partitions

            for (unsigned int i = 0 ; i < partitions.size() ; i++) {
                partitions[i]->set_offset(RdKafka::Topic::OFFSET_STORED);
            }

            consumer->assign(partitions);

            std::cerr << "RebalanceCb: ASSIGN_PARTITIONS"  << std::endl;

        } else if (err == RdKafka::ERR__REVOKE_PARTITIONS) {
            // Application may commit offsets manually here
            // if auto.commit.enable=false

            std::cerr << "RebalanceCb: REVOKE_PARTITIONS"  << std::endl;

            consumer->unassign();

        } else {
            std::cerr << "RebalanceCb error:" <<
                        RdKafka::err2str(err) << std::endl;
            consumer->unassign();
        }
    }
};

void msg_consume(RdKafka::Message* message, void* opaque) {
  switch (message->err()) {
    case RdKafka::ERR__TIMED_OUT:
      break;

    case RdKafka::ERR_NO_ERROR:
      /* Real message */
      std::cerr << "Read msg at offset " << message->offset() << std::endl;
      RdKafka::MessageTimestamp ts;
      ts = message->timestamp();
      if (ts.type != RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE) {
	    std::string tsname = "?";
	    if (ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_CREATE_TIME)
	      tsname = "create time";
        else if (ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_LOG_APPEND_TIME)
          tsname = "log append time";
        std::cout << "Timestamp: " << tsname << " " << ts.timestamp << std::endl;
      }
      if (message->key()) {
        std::cout << "Key: " << *message->key() << std::endl;
      }
      printf("%.*s\n",
            static_cast<int>(message->len()),
            static_cast<const char *>(message->payload()));
      break;

    case RdKafka::ERR__UNKNOWN_TOPIC:
    case RdKafka::ERR__UNKNOWN_PARTITION:
      std::cerr << "Consume failed: " << message->errstr() << std::endl;
      run = false;
      break;

    default:
      /* Errors */
      std::cerr << "Consume failed: " << message->errstr() << std::endl;
      run = false;
  }
}

int main (int argc, char **argv) {
    std::string brokers = "127.0.0.1:9093";
    std::vector<std::string> topics = {"test"};
    std::string key = "fgr";
    int32_t partition = RdKafka::Topic::PARTITION_UA;
    int64_t start_offset = RdKafka::Topic::OFFSET_BEGINNING;

    std::string errstr;

    /*
    * Create configuration objects
    */
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

    conf->set("bootstrap.servers", brokers, errstr);
    conf->set("security.protocol", "SASL_PLAINTEXT", errstr);
    conf->set("sasl.mechanisms", "PLAIN", errstr);
    conf->set("sasl.username", "bob", errstr);
    conf->set("sasl.password", "bobpassword", errstr);
    conf->set("group.id", "bob-group", errstr);

    std::cerr << errstr << std::endl;

    ExampleEventCb ex_event_cb;
    conf->set("event_cb", &ex_event_cb, errstr);
    
    ExampleRebalanceCb ex_rebalance_cb;
    conf->set("rebalance_cb", &ex_rebalance_cb, errstr);

   /*
   * Create consumer using accumulated global configuration.
   */
  RdKafka::KafkaConsumer *consumer = RdKafka::KafkaConsumer::create(conf, errstr);
  if (!consumer) {
    std::cerr << "Failed to create consumer: " << errstr << std::endl;
    exit(1);
  }

  delete conf;

  std::cout << "% Created consumer " << consumer->name() << std::endl;


  /*
   * Subscribe to topics
   */
  RdKafka::ErrorCode err = consumer->subscribe(topics);
  if (err) {
    std::cerr << "Failed to subscribe to " << topics.size() << " topics: "
              << RdKafka::err2str(err) << std::endl;
    exit(1);
  }

  /*
   * Consume messages
   */
  while (run) {
    RdKafka::Message *msg = consumer->consume(1000);
    msg_consume(msg, NULL);
    delete msg;
  }

  /*
   * Stop consumer
   */
  consumer->close();
  delete consumer;

  /*
   * Wait for RdKafka to decommission.
   * This is not strictly needed (with check outq_len() above), but
   * allows RdKafka to clean up all its resources before the application
   * exits so that memory profilers such as valgrind wont complain about
   * memory leaks.
   */
  RdKafka::wait_destroyed(5000);

  return 0;
}