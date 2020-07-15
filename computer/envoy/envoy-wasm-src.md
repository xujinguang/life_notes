# Envoy Wasm 源码
CT: 2020年 7月15日 星期三 15时54分29秒 CST

##main

```c++
int main(int argc, char** argv) {
  	return Envoy::MainCommon::main(argc, argv); 
}
```



```c++
//->执行
int MainCommon::main(int argc, char** argv, PostServerHook hook) { //hook默认为 nullptr
  std::unique_ptr<Envoy::MainCommon> main_common;
  try {
    main_common = std::make_unique<Envoy::MainCommon>(argc, argv); //实例化MainCommon
    Envoy::Server::Instance* server = main_common->server(); //base_.server();
    if (server != nullptr && hook != nullptr) {
      hook(*server);
    }
  } catch (){}

  // Run the server listener loop outside try/catch blocks, so that unexpected exceptions
  // show up as a core-dumps for easier diagnostics.
  return main_common->run() ? EXIT_SUCCESS : EXIT_FAILURE; //base_.run()
}

//构造函数
MainCommon::MainCommon(int argc, const char* const* argv)
     //实例化OptionsImpl
    : options_(argc, argv, &MainCommon::hotRestartVersion, spdlog::level::info), 
		//实例化 MainCommonBase
      base_(options_, 
          real_time_system_, 
          default_listener_hooks_, 
      		prod_component_factory_,
      		std::make_unique<Runtime::RandomGeneratorImpl>(), 
      		platform_impl_.threadFactory(),
      		platform_impl_.fileSystem(), 
      		nullptr)
      {}

//其中base_ 是 MainCommonBase，因此转换成MainCommonBase 的server() 和 run(),构造函数如下
MainCommonBase::MainCommonBase(const OptionsImpl& options, Event::TimeSystem& time_system,
                               ListenerHooks& listener_hooks,
                               Server::ComponentFactory& component_factory,
                               std::unique_ptr<Runtime::RandomGenerator>&& random_generator,
                               Thread::ThreadFactory& thread_factory,
                               Filesystem::Instance& file_system,
                               std::unique_ptr<ProcessContext> process_context)
    : 
options_(options), 
component_factory_(component_factory), 
thread_factory_(thread_factory),
file_system_(file_system), symbol_table_(Stats::SymbolTableCreator::initAndMakeSymbolTable(
             options_.fakeSymbolTableEnabled())),
stats_allocator_(*symbol_table_) {
  //其他初始化
}
```

MainCommon这一层有点多余。代码中说未来会移除它。直接使用MainCommonBase

#### main_common->server()

```c++
//base_.server()->MainCommonBase::server()
// Will be null if options.mode() == Server::Mode::Validate
Server::Instance* server() { return server_.get(); }
//server是个InstanceImpl类型的 MainCommonBase 成员变量
std::unique_ptr<Server::InstanceImpl> server_;
//server_是在MainCommonBase实例化构造函数中初始化的
 server_ = std::make_unique<Server::InstanceImpl>(
   *init_manager_, 
   options_, 
   time_system, 
   local_address, 
   listener_hooks, 
   *restarter_,
   *stats_store_, 
   access_log_lock, 
   component_factory, 
   std::move(random_generator), 
   *tls_,
   thread_factory_, 
   file_system_, 
   std::move(process_context));
//返回 Instance 实例指针,因为unique不能作为参数传递，需要get
```

#####hook(*sever)

```c++
//hook的原型
using PostServerHook = std::function<void(Server::Instance& server)>;
//MainCommon 的main方法声明个静态方法，使用ns调用。hook 默认为空，所以这里hook()不执行
static int main(int argc, char** argv, PostServerHook hook = nullptr);
//options.mode() == Server::Mode::Validate时，sever=nullptr，同样不执行
```

####main_common->run() 

```c++
//base_.run()->	MainCommonBase::run()
依据启动时候传入的有三种启动模式
	Server::Mode{}->/Serve/Validate/InitOnly
	1. Server - 默认模式
	2. Validate - 验证，最大限度不开启上下游的网络连通,使用本地网络
	3. InitOnly - 初始化，完整加载并初始化配置，然后退出
```
## Server 模式

```c++
server_->run();       
//上面run()实际执行启动实例 run
void InstanceImpl::run(){
        const auto run_helper = RunHelper(*this, 
                                          options_, 
                                          *dispatcher_, 
                                          clusterManager(),
                                          access_log_manager_, 
                                          init_manager_, 
                                          overloadManager(), 
                                          [this] {
                                      notifyCallbacksForStage(Stage::PostInit);
                                      startWorkers();
                                    });

  // Run the main dispatch loop waiting to exit.
  ENVOY_LOG(info, "starting main dispatch loop");
  auto watchdog =
      guard_dog_->createWatchDog(api_->threadFactory().currentThreadId(), "main_thread");
  watchdog->startWatchdog(*dispatcher_);
  dispatcher_->post([this] { notifyCallbacksForStage(Stage::Startup); });
  dispatcher_->run(Event::Dispatcher::RunType::Block);
  ENVOY_LOG(info, "main dispatch loop exited");
  guard_dog_->stopWatching(watchdog);
  watchdog.reset();

  terminate();
}
```

#### startWorkers();

```c++
listener_manager_->startWorkers(*guard_dog_);
```

#### dispatcher_->run(Event::Dispatcher::RunType::Block);

```c++
//dispatcher_ 是InstanceImpl的成员变量
Event::DispatcherPtr dispatcher_;
using DispatcherPtr = std::unique_ptr<Dispatcher>;
//->
void DispatcherImpl::run(RunType type) {
  run_tid_ = api_.threadFactory().currentThreadId();
  runPostCallbacks();
  base_scheduler_.run(type); //LibeventScheduler
}
//->
void LibeventScheduler::run(Dispatcher::RunType mode) {
  int flag = 0;
  switch (mode) {
  case Dispatcher::RunType::NonBlock:
    flag = EVLOOP_NONBLOCK;
    break;
  case Dispatcher::RunType::Block:
    // The default flags have 'block' behavior. See
    // http://www.wangafu.net/~nickm/libevent-book/Ref3_eventloop.html
    break;
  case Dispatcher::RunType::RunUntilExit:
    flag = EVLOOP_NO_EXIT_ON_EMPTY;
    break;
  }
  event_base_loop(libevent_.get(), flag);
}
//->
/**
  Wait for events to become active, and run their callbacks.

  This is a more flexible version of event_base_dispatch().

  By default, this loop will run the event base until either there are no more
  pending or active events, or until something calls event_base_loopbreak() or
  event_base_loopexit().  You can override this behavior with the 'flags'
  argument.

  @param eb the event_base structure returned by event_base_new() or
     event_base_new_with_config()
  @param flags any combination of EVLOOP_ONCE | EVLOOP_NONBLOCK
  @return 0 if successful, -1 if an error occurred, or 1 if we exited because
     no events were pending or active.
  @see event_base_loopexit(), event_base_dispatch(), EVLOOP_ONCE,
     EVLOOP_NONBLOCK
  */
EVENT2_EXPORT_SYMBOL
int event_base_loop(struct event_base *, int)
```
## Valide 模式
```c++
    auto local_address = Network::Utility::getLocalAddress(options_.localAddressIpVersion());
    return Server::validateConfig(options_, local_address, component_factory_, thread_factory_,
                                  file_system_)
```
validateConfig
-> 
ValidationInstance
->
## InitOnly 模式
do nothing!
```c
  do {                                                                                             \
  } while (false);
```
