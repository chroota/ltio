## ltio (LigthingIO)

![master](https://github.com/echoface/ltio/actions/workflows/build.yml/badge.svg?branch=master)

LigthingIO is a lightweight network IO framework with some infrastructure code for better coding experience;
all of those code based on my work experience in those years, inspired by project Chromium/libevent/Qt/NodeJs

## code implemnet

Here is a brief summary of this project:

### base
- base/util code
- message/task loop
- repeat timer
- lazyinstance
- coroutine scheduler(a limited G/M/P schedule model with work stealing)

### network io
- reactor model
- resp/line/raw/http[s]1.x protocol
- openssl tls socket implement
- raw/http[s]/line general server
- maglevHash/consistentHash/roundrobin router
- raw/http[s]/line client with full async/waitable coro
- async redis protocol[only client side support]
- websocket bi-stream support
- http2 server side(h2,h2c) with server push

### component
- geo utils
- bloom filter
- countmin-sketch
- source loader (TP)
- lru/mru component
- boolean indexer(computing advertising)
- inverted indexer(computing advertising)
- add full async mysql client support; [move to ltapp]

TODO:
- RPC implement(may another repo)
- Http Message body reader/writer interface refactor
  - header refactor
  - current only full-filled body supported
- Adaptive io buffer for long running io connection

Issues and PRs are welcome 🎉🎉🎉


## Build And Deploy
```bash
sudo apt-get update -yqq
sudo apt-get install -yqq \
       software-properties-common unzip \
       cmake build-essential libgoogle-perftools-dev \
       git-core libgoogle-perftools-dev libssl-dev zlib1g-dev

git clone https://github.com/echoface/ltio.git
cd ltio
git submodule update --init --recursive
mkdir build; cd build;
cmake -DWITH_OPENSSL=[ON|OFF]       \
      -DLTIO_BUILD_UNITTESTS=OFF .. \

./bin/simple_ltserver
```

## LazyInstance:

```c++
  //class Foo, only when use to create Foo Object
  static base::LazyInstance<Foo> gFoo = LAZY_INSTANCE_INIT;
```

## repeated timer:

see ut `unittests/base/repeat_timer_unittest.cc` for more detail
```
base::RepeatingTimer timer(&loop);
timer.Start(interval_ms, [&]() {
   // do something;
});
```

## TaskLoop[MessageLoop]

like mostly loop implement, all PostTask/PostDelayTask/PostTaskWithReply implemented, it's inspired by chromium messageloop code;

```c++
/*
  follow code will give you infomathion about the exception task where come from
  W1127 08:36:09.786927  9476 closure_task.h:46] Task Error Exception, From:run_loop_test.cc:24
*/

void LocationTaskTest() {
  base::MessageLoop loop;
  loop.Start();
  loop.PostTask(FROM_HERE, [](){
    throw "bad task!";
  });
  loop.WaitLoopEnd();
}

// assume a loop started
base::MessageLoop loop;

/* schedule task*/
loop.PostTask(NewClosure([&]() {
        LOG(INFO) << "delay task";
      }));
loop.PostTask(NewClosure(std::bind(...)));

loop.PostTask(FROM_HERE, [&]() {
        LOG(INFO) << "delay task";
      });
loop.PostTask(FROM_HERE, StubClass::MemberFunc, stub, ...args);

/* delay task*/
loop.PostDelayTask(NewClosure([&]() {
                    LOG(INFO) << "delay task";
                   }),500);

/* with callback notify*/
loop.PostTaskWithReply(FROM_HERE,
                       []() {/*task 1*/},
                       []() {/*task callback*/});

base::MessageLoop callback_loop;
loop.PostTaskWithReply(FROM_HERE,
                       []() {/*task 1*/},
                       []() {/*task callback*/},
                       &callback_loop,
                      );
/*callback notify task equal implement*/
loop.PostTask(FROM_HERE, [&]() {
  /* run task*/
  if (callback_loop) {
      callback_loop.PostTask(FROM_HERE, [](){
        /*run callback task*/
      })
    return;
  }
  /*run callback task*/
});

...
```

## Coroutine:

current only fcontext(extract from boost library(but no boost lib needed)) supported,
in history commit, another two coroutine backend suppored, (libcoro|libaco),
in some reason switch to fcontext impl now;

NOTE: NO SYSTEM Hook
reason: many struggle behind this choose; not enough/perfect fundamentals implement can
make it stable work on a real world complex project with widely varying dependency 3rdparty lib

ltcoro top on base::MessageLoop, you should remember this!, another point need considered, a started corotuine
will be guarded always runing on it's binded MessageLoop(physical thread), all coro dependency things
need run on a corotine context, eg: `CO_RESUMER, CO_YIELD, CO_SLEEP, CO_SYNC`

some brief usage code looks like below:
```c++
// just imaging `CO_GO/co_go` equal golang's go keywords
void coro_c_function();
void coro_fun(std::string tag);
{
    CO_GO coro_c_function; //schedule c function use coroutine

    CO_GO std::bind(&coro_fun, "tag_from_go_synatax"); // c++11 bind

    CO_GO [&]() {  // lambda support
        LOG(INFO) << " run lambda in coroutine" ;
    };

    CO_GO &loop << []() { //run corotine with specify loop runing on
      LOG(INFO) << "go coroutine in loop ok!!!";
    };

    // 使用net.client定时去获取网络资源
    bool stop = false;
    CO_GO [&]() {

      CO_SYNC []() {
      };
      // equal CO_GO std::bind([](){}, CO_RESUMER());

      co_sleep(1000); // every 1s

      //self control resume, must need call resumer manually,
      //otherwise will cause corotuine leak, this useful with other async library
      auto resumer = CO_RESUMER();
      loop.PostTask(FROM_HERE, [&]() {
        resumer();
      })
    };
}

// broadcast make sure wg in a coro context
{
  CO_GO [&]() { //main
    auto wg = base::WaitGroug::New();

    wg.Add(1);
    loop.PostTask(FROM_HERE,
                  [wg]() {
                    //things
                    wg->Done();
                  });

    for (int i = 0; i < 10; i++) {
      wg.Add(1);

      CO_GO [wg]() {
        base::ScopedGuard guard([wg]() {wg->Done();});

        LOG(INFO) << "normal task start..." << l->LoopName();

        CO_SLEEP(100);
        // mock network stuff
        // request = BuildHttpRequest();
        // auto response = client.Get(request, {})
        // handle_response();
        LOG(INFO) << "normal task end..." << l->LoopName();
      };
    }

    LOG_IF(INFO, WaitGroup::kTimeout == wg->Wait(10000))
      << " timeout, not all task finish in time";
  }
```

基于coroutine的IO. 这里还有区别于callback 和 coroutine 协作的异步IO
```c++
TEST_CASE("coro.ioevent", "[ioevent for coro]") {
  //FLAGS_v = 26;
  // 因为coro是构建在底层的消息循环之上的task runner
  // 实际中可以隐藏掉loop，通常为构建物理线程个loops
  base::MessageLoop loop("main");
  loop.Start();

  int timeout = 20;
  bool running = true;
  int fd = eventfd(0, EFD_NONBLOCK);
  CHECK(fd > 0);

  co_go &loop << [&]() {
    base::FdEvent fdev(fd, base::LtEv::READ);
    co::IOEvent ioev(&fdev);
    do {
      ignore_result(ioev.Wait(timeout)); //等待IOEvent
      uint64_t val = 0;
      if (eventfd_read(fd, &val) == 0) {
        std::cout << ioev.ResultStr() << ", read val:" << val << std::endl;
        continue;
      }
      // 对于Server: 这里则是accept -> co_go handle_connection(fd);
      // 对于Client: 这里则是connect-> co_go handle_client_connection(fd);
      // 对于Unary connection: read -> protocolDecode -> MessageHandler(req, res) -> SendResponse
      // 对于BiStreamConnection: ....
      std::cout << ioev.ResultStr() << ", read err:" << base::StrError() << std::endl;
      if (errno != EAGAIN) {
        break;
      }
    } while(running);
    loop.QuitLoop();
  };

  int cnt = 100;
  while(cnt--) {
    uint64_t v = base::RandInt(timeout / 2, timeout + (timeout / 2));
    usleep(1000 * v); // ms
    ignore_result(eventfd_write(fd, v));
  };
  running = false;
  loop.WaitLoopEnd();
  close(fd);
}
```

这里没有Hook系统调用的原因主要有两个:
1. Hook系统调用的集成性并没有他们所宣传的那么好(可能我理解不够).
2. 个人仍然坚持需要知道自己在干什么,有什么风险, 开发者有选择的使用Coroutone

基于上面这个原因, 所以在ltio中, Coroutine是基于MessageLoop的TopLevel的工具. 其底层模拟实现了Golang类似的G,M,P 角色调度.并支持Worksteal, 其中有跨线程调度在C++资源管理方面带来的问题, 更重要的一点是希望通过约束其行为, 让使用着非常明确其运行的环境和作用. 从个人角度上讲, 仍旧希望他是一基于MessageLoop的Task调度为主的实现方式, 但是可以让用户根据需要使用Coroutine作为工具辅助, 使得完成一些事情来让逻辑编写更加舒适.所以有两个需要开发者了解的机制(了解就足够了)
- 1. Coroutine Task 开始运行后,以后只会在指定的物理线程切换状态 Yield-Run-End, 所以WorkSteal的语义被约束在一个全新的(schedule之后未开始运行的)可以被stealing调度到其他woker上运行, 而不是任何状态都可以被stealing调度, 任务Yield后恢复到Run状态后,仍旧在先前绑定的物理线程上; 在现实项目中的C++工程, 开发者无法忽视线程和相关线程绑定的存在. 这样的实现可以使得一些基于thread local的数据仍可以被安全的使用.😊
- 2. 调度方式两种, 作出合理的选择, 有时候这很有用, eg: 访问/修改的数据是被某一个绑定的loop管理维护的
  - `CO_GO task;` 允许这个task被workstealing的方式调度
  - `CO_GO &specified_loop << task;` 指定物理线程运行调度任务
  作为一个在一线业务开发多年的菜鸟本鸟, 合理的设计业务比什么都重要; 合理的选择和业务设计, 会让很多所谓的锁和资源共享变得多余; 在听到golang的口号:"不要通过共享内存来通信，而应该通过通信来共享内存"之前,本人基于chromium content api做开发和在计算广告设计的这几年的经验有很大的感触. 基于转移控制权的逻辑来设计数据会让很多冲突的解决变得简单.

## NET IO:
---

see `examples/net_io/simple_ltserver.cc examples/net_io/lt_http_client.cc` for more detail

### Benchmark

just run a server with: `./bin/simple_ltserver`
a more benchable server: `./bin/http_benchmark_server`

a tfb benchark report will found at tfb project on next bench round
see: [tfb](https://www.techempower.com/benchmarks/)

### TLS support
- compile with `-DWITH_OPENSSL=ON`
- run simple server with selfsigned cert&key
> `./bin/simple_ltserver [--v=26] --ssl=true --cert=./cert.pem --key=./key.pem`
- use `openssl s_client` or `curl`
```
curl: with insercure(ingnore certifaction verify)
curl -v -k "https://localhost:5006/ping"

curl: with certifaction verify
curl -v --cacert ./cert.pem "https://localhost:5006/ping"

lt_http_client: without certifaction verify, insercure
./bin/lt_http_client [--v=26] --insercure=true --remote="https://localhost:5006"

lt_http_client: with certifaction verify
./bin/lt_http_client [--v=26] --insercure=true --remote="https://localhost:5006"
./bin/lt_http_client [--v=26] --cert=./cert.pem --remote="https://localhost:5006/ping"
```


NOTE
---

**email** me if any question and problem;

Learning and integrate what i leaned/think into ltio

From HuanGong 2018-01-08


# Copyright and License

Copyright (C) 2018, by HuanGong [<gonghuan.dev@gmail.com>](mailto:gonghuan.dev@gmail.com).

Under the Apache License, Version 2.0.

See the [LICENSE](LICENSE) file for details.
