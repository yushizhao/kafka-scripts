# Kafka 介绍 

## 为什么用 Kafka

  + 高可用集群
    + 对客户端透明
  + 持久化
    + 可重放消息序列
  + 多账户权限管理
    + 细到某个账户对某个 topic 的读写权限
  + 订阅组机制
    + 实现并发

## 工作原理示意图

## 使用须知

### 作为 admin

  + broker 配置
  + topic 的建立
  + 账户 acl

### 作为 producer

  + producer 配置

  + C++ 客户端 produce 流程
    + 应用: 继承并构造 callback 类
    + 应用: 实例化配置类
      + kafka 配置
      + 注册 callback 类实例
    + 应用: 实例化 producer  
    + 应用: 实例化 message
    + 应用: 调用 produce, 无阻塞
    + 客户端: 把 message 加入发送序列 / 客户端拒绝非法 message
    + 客户端: 同步返回 message 是被客户端拒了还是进了发送序列
    + 客户端: 由其他线程向 broker 顺序发送发送序列中的 message 包括自动重试
    + broker: 将 message 分配到某个 partition
    + broker: 将 message 下发到集群的 某个 partition leader进行写入
    + broker: 回报写入成功或失败 / 等到 partition follower 也写入后再回报
    + 客户端: 从 broker 收到写入成功或失败的回报 / 自动重试 / 重试次数用完回报一个失败
    + 客户端: 管理内存中的回报队列
    + 应用: 调用 poll, 可设置阻塞
    + 客户端: 回报队列逐一调用 callback / 可选择阻塞若回报队列空
    + 应用: 收到 callback 结果

  + message 结构
    + header: 键值对, 可省略. 用来比如指定 value 的解析方式.
    + key: 字节串, 可省略. broker 根据 key 和 partitioner 函数来决定一个 message 去哪个 partition. 换句话说 key 相同的 message 一定在同一个 partition.
    + value: 字节串. 正文.
    + timestamp: 整数. producer 指定. 似乎只是记录, 在 broker 中并没有逻辑, 待考.
    + opaque: 字节串, 可省略. 在 broker 中不储存不做任何处理. 仅用来在客户端把 produce 的 message 与 触发 callback 的 message 回报 关联起来.

  + callback 类
    + DeliveryReportCb
      + 成功 produce 时回调
      + 入参是一个 message
      + 不指定则使用默认, 默认的什么都不做
    + EventCb
      + 所有失败信息将回调这个
      + 入参是一个 event
      + 除失败信息之外的 event 怎么用有待进一步研究
      + 不指定则使用默认, 默认的会以默认格式把错误信息打印出来

### 作为 consumer

  + consumer 配置

  + C++ 客户端 subscribe 流程
    + 应用: 继承并构造 callback 类
    + 应用: 实例化配置类
      + kafka 配置
      + 注册 callback 类实例
    + 应用: 实例化 consumer  
    + 应用: 调用 subscribe, 无阻塞
    + 客户端: 向 broker 订阅
    + broker: 根据 consumer 的订阅组分配 partition
    + broker: 根据 consumer 配置 和 broker 内部记录确定 offset
    + broker: 将订阅成功或失败返回给客户端
    + 客户端: 收到 broker 返回的回报 / 自动重试 / 重试次数用完回报一个失败
    + 客户端: 直接调用 RebalanceCb 实例
    + 应用: 收到 RebalanceCb 结果

  + C++ 客户端 consume 流程
    + broker: 发送 message 和 rebalance 等消息给客户端
    + 客户端: 自动接收 broker 发来的消息
    + 客户端: 管理内存中的回报队列
    + 应用: 调用 consume, 可选择阻塞
    + 客户端: 回报队列逐一调用 callback 直到回报队列中出现一个没有设置 callback 的消息, 比如一个 message, 返回它.
    + 客户端: 自动向 kafka commit offset
    + 客户端: 由其他线程向 broker 要 message
    + broker: 根据配置将适量的新 message 发给客户端
    + 客户端: 管理内存中的回报队列
    
  + C++ 客户端 commit 流程
    + 客户端: 配置中关闭自动 commit offset
    + 应用: 处理完消息后调用 commitSync / commitAsync
    + 客户端: 向 broker commit offset
    + broker: 将订阅成功或失败返回给客户端
    + 客户端: 收到 broker 返回的回报 / 自动重试 / 重试次数用完回报一个失败
    + 客户端: 直接调用 OffsetCommitCb 实例
    + 应用: 收到 OffsetCommitCb 结果 

  + C++ 客户端 unsubscribe 流程
    + 略

  + message 结构
    + header: 键值对, 可省略. 用来比如指定 value 的解析方式.
    + key: 字节串, 可省略. broker 根据 key 和 partitioner 函数来决定一个 message 去哪个 partition. 换句话说 key 相同的 message 一定在同一个 partition.
    + value: 字节串. 正文.
    + timestamp: 整数. producer 指定. 似乎只是记录, 在 broker 中并没有逻辑, 待考.
    + opaque: 字节串, 可省略. 在 broker 中不储存不做任何处理. 仅用来在客户端把 produce 的 message 与 触发 callback 的 message 回报 关联起来.

  + partition 结构
    + topic: 字符串, 主题名.
    + partition id: 整数, partition 编号.
    + offset: 整数, 下次读取从这个 partition 的第几条消息开始. 同个 partiotion 内的消息读取是定序的.

  + callback 类
    + EventCb
      + 所有失败信息将回调这个
      + 入参是一个 event
      + 除失败信息之外的 event 怎么用有待进一步研究
      + 不指定则使用默认, 默认的会以默认格式把错误信息打印出来
    + RebalanceCb
      + subscribe / unsubscribe 将调用这个
      + broker 主动发起 rebalance 也会调用这个
      + 入参是一列 partition
      + 不指定则使用默认, 默认的会自动处理订阅位置和状态
    + OffsetCommitCb
      + 自动和非自动的 commit 都将调用这个
      + 不指定则使用默认, 默认的什么都不做

### 示例代码

    