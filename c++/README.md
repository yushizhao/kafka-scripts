# Confluent’s c++ client for Apache Kafka

## 介绍

Confluent 在提供 kafka client libraries 时, 是这样考虑的, 

写好一个 c 的 client, 也就是:

https://github.com/edenhill/librdkafka

其他语言的 client 都是 c 语言 client 的包装, 包括 c++.

## 安装

推荐从源码编译安装.其他安装方式可能会在使用中出现问题,需要卸载后重新从源码编译安装.

```
git clone https://github.com/edenhill/librdkafka.git
cd librdkafka
./configure --prefix=/usr
make
sudo make install
```

之后可以尝试编译本文件夹下的示例代码

```
g++ -o producer producer.cpp -lrdkafka++ -std=c++11
g++ -o consumer consumer.cpp -lrdkafka++ -std=c++11
```

## 文档

https://github.com/edenhill/librdkafka/blob/master/src/rdkafka.h
https://github.com/edenhill/librdkafka/blob/master/src-cpp/rdkafkacpp.h
https://github.com/edenhill/librdkafka/blob/master/CONFIGURATION.md