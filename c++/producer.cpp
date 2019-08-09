#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>

#include <librdkafka/rdkafkacpp.h>

class ExampleDeliveryReportCb : public RdKafka::DeliveryReportCb {
 public:
  void dr_cb (RdKafka::Message &message) {
    std::string status_name;
    switch (message.status())
      {
      case RdKafka::Message::MSG_STATUS_NOT_PERSISTED:
        status_name = "NotPersisted";
        break;
      case RdKafka::Message::MSG_STATUS_POSSIBLY_PERSISTED:
        status_name = "PossiblyPersisted";
        break;
      case RdKafka::Message::MSG_STATUS_PERSISTED:
        status_name = "Persisted";
        break;
      default:
        status_name = "Unknown?";
        break;
      }
    std::cout << "Message delivery for (" << message.len() << " bytes): " <<
      status_name << ": " << message.errstr() << std::endl;
    if (message.key())
      std::cout << "Key: " << *(message.key()) << ";" << std::endl;
  }
};


class ExampleEventCb : public RdKafka::EventCb {
 public:
  void event_cb (RdKafka::Event &event) {
    switch (event.type())
    {
      case RdKafka::Event::EVENT_ERROR:
        if (event.fatal()) {
          std::cerr << "FATAL ";
        //   run = false;
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

      default:
        std::cerr << "EVENT " << event.type() <<
            " (" << RdKafka::err2str(event.err()) << "): " <<
            event.str() << std::endl;
        break;
    }
  }
};

int main (int argc, char **argv) {
    std::string brokers = "127.0.0.1:9093";
    std::string topic_str = "test";
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
    conf->set("sasl.username", "admin", errstr);
    conf->set("sasl.password", "adminpassword", errstr);

    std::cerr << errstr << std::endl;

    // if (conf->set("debug", debug, errstr) != RdKafka::Conf::CONF_OK) {
    //     std::cerr << errstr << std::endl;
    //     exit(1);
    // }

    ExampleEventCb ex_event_cb;
    conf->set("event_cb", &ex_event_cb, errstr);

    ExampleDeliveryReportCb ex_dr_cb;
    /* Set delivery report callback */
    conf->set("dr_cb", &ex_dr_cb, errstr);

    /*
    * Create producer using accumulated global configuration.
    */
    RdKafka::Producer *producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) {
        std::cerr << "Failed to create producer: " << errstr << std::endl;
        exit(1);
    }

    std::cout << "% Created producer " << producer->name() << std::endl;
      
    /*
     * Read messages from stdin and produce to broker.
     */
    for (std::string line; std::getline(std::cin, line);) {
      if (line.empty()) {
        producer->poll(0);
	break;
      }

      /*
       * Produce message
       */
      RdKafka::ErrorCode resp =
        producer->produce(topic_str, partition,
                          RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
                          /* Value */
                          const_cast<char *>(line.c_str()), line.size(),
                          /* Key */
                          const_cast<char *>(key.c_str()), key.size(),
                          /* Timestamp (A value of 0 will use the current wall-clock time.) */
                          0,
                          /* Per-message opaque value passed to
                           * delivery report */
                          NULL);
      if (resp != RdKafka::ERR_NO_ERROR) {
        std::cerr << "% Produce failed: " <<
          RdKafka::err2str(resp) << std::endl;
      } else {
        std::cerr << "% Produced message (" << line.size() << " bytes)" <<
          std::endl;
      }

      producer->poll(0);
    }

    while (producer->outq_len() > 0) {
      std::cerr << "Waiting for " << producer->outq_len() << std::endl;
      producer->poll(1000);
    }

    delete producer;
}