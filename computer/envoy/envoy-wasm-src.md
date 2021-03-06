## Envoy Wasm 源码

CT: 2020年 7月15日 星期三 15时54分29秒 CST

### main

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

#### hook(*sever)

```c++
//hook的原型
using PostServerHook = std::function<void(Server::Instance& server)>;
//MainCommon 的main方法声明个静态方法，使用ns调用。hook 默认为空，所以这里hook()不执行
static int main(int argc, char** argv, PostServerHook hook = nullptr);
//options.mode() == Server::Mode::Validate时，sever=nullptr，同样不执行
```

#### main_common->run() 

```c++
//base_.run()->	MainCommonBase::run()
依据启动时候传入的有三种启动模式
	Server::Mode{}->/Serve/Validate/InitOnly
	1. Server - 默认模式
	2. Validate - 验证，最大限度不开启上下游的网络连通,使用本地网络
	3. InitOnly - 初始化，完整加载并初始化配置，然后退出
```
###Server模式

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
### Valide 模式

```c++
    auto local_address = Network::Utility::getLocalAddress(options_.localAddressIpVersion());
    return Server::validateConfig(options_, local_address, component_factory_, thread_factory_,
                                  file_system_)
```
validateConfig
-> 
ValidationInstance
->
### InitOnly 模式

do nothing!
```c
  do {                                                                                             
  } while (false);
```

## 插件

### HTTP

####Encode

```c++
/**
 * Encodes an HTTP stream. This interface contains methods common to both the request and response
 * path.
 * TODO(mattklein123): Consider removing the StreamEncoder interface entirely and just duplicating
 * the methods in both the request/response path for simplicity.
 */
class StreamEncoder {
public:
  virtual ~StreamEncoder() = default;

  /**
   * Encode a data frame.
   * @param data supplies the data to encode. The data may be moved by the encoder.
   * @param end_stream supplies whether this is the last data frame.
   */
  virtual void encodeData(Buffer::Instance& data, bool end_stream) PURE;

  /**
   * @return Stream& the backing stream.
   */
  virtual Stream& getStream() PURE;

  /**
   * Encode metadata.
   * @param metadata_map_vector is the vector of metadata maps to encode.
   */
  virtual void encodeMetadata(const MetadataMapVector& metadata_map_vector) PURE;

  /**
   * Return the HTTP/1 stream encoder options if applicable. If the stream is not HTTP/1 returns
   * absl::nullopt.
   */
  virtual Http1StreamEncoderOptionsOptRef http1StreamEncoderOptions() PURE;
};
class RequestEncoder : public virtual StreamEncoder{};
class ResponseEncoder : public virtual StreamEncoder {};
class RequestEncoderWrapper : public RequestEncoder{};
```

#### Decode

```c++
/**
 * Decodes an HTTP stream. These are callbacks fired into a sink. This interface contains methods
 * common to both the request and response path.
 * TODO(mattklein123): Consider removing the StreamDecoder interface entirely and just duplicating
 * the methods in both the request/response path for simplicity.
 */
class StreamDecoder {
public:
  virtual ~StreamDecoder() = default;

  /**
   * Called with a decoded data frame.
   * @param data supplies the decoded data.
   * @param end_stream supplies whether this is the last data frame.
   */
  virtual void decodeData(Buffer::Instance& data, bool end_stream) PURE;

  /**
   * Called with decoded METADATA.
   * @param decoded METADATA.
   */
  virtual void decodeMetadata(MetadataMapPtr&& metadata_map) PURE;
};

class RequestDecoder : public virtual StreamDecoder {}
class ResponseDecoder : public virtual StreamDecoder {}
class ResponseDecoderWrapper : public ResponseDecoder {}
```

#### 汇总stream

```c++
 class ConnPoolImpl : public Http::HttpConnPoolImplBase { //包括StreamWrapper，ActiveClient
struct StreamWrapper : public RequestEncoderWrapper,
                         public ResponseDecoderWrapper,
                         public StreamCallbacks{};
 class ActiveClient : public Envoy::Http::ActiveClient {
   StreamWrapperPtr stream_wrapper_;
 };
 };
ConnPoolImpl::StreamWrapper::StreamWrapper(ResponseDecoder& response_decoder, ActiveClient& parent)
    : RequestEncoderWrapper(parent.codec_client_->newStream(*this)),
      ResponseDecoderWrapper(response_decoder), parent_(parent) {

  RequestEncoderWrapper::inner_.getStream().addCallbacks(*this);
}

//http1, newStreamEncoder 实例化 StreamWrapper
RequestEncoder& ConnPoolImpl::ActiveClient::newStreamEncoder(ResponseDecoder& response_decoder) {
  stream_wrapper_ = std::make_unique<StreamWrapper>(response_decoder, *this);
  return *stream_wrapper_;
}

//onPoolReady -> newStreamEncoder
void HttpConnPoolImplBase::onPoolReady(Envoy::ConnectionPool::ActiveClient& client,
                                       Envoy::ConnectionPool::AttachContext& context) {
  ActiveClient* http_client = static_cast<ActiveClient*>(&client);
  auto& http_context = typedContext<HttpAttachContext>(context);
  Http::ResponseDecoder& response_decoder = *http_context.decoder_;
  Http::ConnectionPool::Callbacks& callbacks = *http_context.callbacks_;
  Http::RequestEncoder& new_encoder = http_client->newStreamEncoder(response_decoder);
  callbacks.onPoolReady(new_encoder, client.real_host_description_,
                        http_client->codec_client_->streamInfo());
}

//attachRequestToClient -> onPoolReady
//newStream->attachRequestToClient | onUpstreamReady->attachRequestToClient
//ConnPoolImpl 实例化时候调用onUpstreamReady
//对接到下文的 ProdConnPoolImpl 实例化
```



```c++

class Instance {};
class Instance : public Envoy::ConnectionPool::Instance, public Event::DeferredDeletable{};
class ConnPoolImplBase : protected Logger::Loggable<Logger::Id::pool> {};
class HttpConnPoolImplBase : public Envoy::ConnectionPool::ConnPoolImplBase,
                             public Http::ConnectionPool::Instance {};//注意命名空间
class ConnPoolImpl : public Http::HttpConnPoolImplBase {}; //这里有三个，分别是Envoy(http2)，http1,Envoy(tcp)空间下的 ConnPoolImpl
class ProdConnPoolImpl : public ConnPoolImpl {};
//HTTP1 allocateConnPool 实例化 ProdConnPoolImpl
ConnectionPool::InstancePtr
allocateConnPool(Event::Dispatcher& dispatcher, Upstream::HostConstSharedPtr host,
                 Upstream::ResourcePriority priority,
                 const Network::ConnectionSocket::OptionsSharedPtr& options,
                 const Network::TransportSocketOptionsSharedPtr& transport_socket_options) {
  return std::make_unique<Http::Http1::ProdConnPoolImpl>(dispatcher, host, priority, options,
                                                         transport_socket_options);
}

//成员函数 allocateConnPool() -> NS::allocateConnPool()
class ProdClusterManagerFactory : public ClusterManagerFactory {}
Http::ConnectionPool::InstancePtr ProdClusterManagerFactory::allocateConnPool() {
  //根据protocol协议分发，http3暂无
  Http::Http2::allocateConnPool();
  Http::Http1::allocateConnPool();
}

// connPool() -> allocateConnPool()
Http::ConnectionPool::Instance*
ClusterManagerImpl::ThreadLocalClusterManagerImpl::ClusterEntry::connPool() {
    ConnPoolsContainer::ConnPools::PoolOptRef pool =
      container.pools_->getPool(priority, hash_key, [&]() {
        return parent_.parent_.factory_.allocateConnPool(
            parent_.thread_local_dispatcher_, host, priority, upstream_protocol,
            !upstream_options->empty() ? upstream_options : nullptr,
            have_transport_socket_options ? context->upstreamTransportSocketOptions() : nullptr);
      });
}
//httpConnPoolForCluster() -> connPool()
Http::ConnectionPool::Instance*
ClusterManagerImpl::httpConnPoolForCluster(const std::string& cluster, ResourcePriority priority,
                                           absl::optional<Http::Protocol> protocol,
                                           LoadBalancerContext* context) {
  ThreadLocalClusterManagerImpl& cluster_manager = tls_->getTyped<ThreadLocalClusterManagerImpl>();

  auto entry = cluster_manager.thread_local_clusters_.find(cluster);
  if (entry == cluster_manager.thread_local_clusters_.end()) {
    return nullptr;
  }

  // Select a host and create a connection pool for it if it does not already exist.
  return entry->second->connPool(priority, protocol, context);
}

// HttpConnPool 构造函数 -> httpConnPoolForCluster()
class HttpConnPool : public Router::GenericConnPool, public Envoy::Http::ConnectionPool::Callbacks {
  HttpConnPool() {
      conn_pool_ = cm.httpConnPoolForCluster(route_entry.clusterName(), route_entry.priority(),
                                           downstream_protocol, ctx);
  }
}

//HttpGenericConnPoolFactory::createGenericConnPool() -> 实例化 HttpConnPool
Router::GenericConnPoolPtr HttpGenericConnPoolFactory::createGenericConnPool(
    Upstream::ClusterManager& cm, bool is_connect, const Router::RouteEntry& route_entry,
    absl::optional<Envoy::Http::Protocol> downstream_protocol,
    Upstream::LoadBalancerContext* ctx) const {
  auto ret = std::make_unique<HttpConnPool>(cm, is_connect, route_entry, downstream_protocol, ctx);
  return (ret->valid() ? std::move(ret) : nullptr);
}
REGISTER_FACTORY(HttpGenericConnPoolFactory, Router::GenericConnPoolFactory);

//Filter::createConnPool() -> HttpGenericConnPoolFactory::createGenericConnPool()
std::unique_ptr<GenericConnPool> Filter::createConnPool() {
  factory = &Envoy::Config::Utility::getAndCheckFactory<GenericConnPoolFactory>(
        cluster_->upstreamConfig().value()); //GenericConnPoolFactory 上面注册
  return factory->createGenericConnPool(config_.cm_, should_tcp_proxy, *route_entry_,
                                        callbacks_->streamInfo().protocol(), this);
}
//Filter::decodeHeaders() -> Filter::createConnPool()
Http::FilterHeadersStatus Filter::decodeHeaders(Http::RequestHeaderMap& headers, bool end_stream) {
  //创建
  std::unique_ptr<GenericConnPool> generic_conn_pool = createConnPool();\
  //连接处理上行数据
    UpstreamRequestPtr upstream_request =
      std::make_unique<UpstreamRequest>(*this, std::move(generic_conn_pool));
  upstream_request->moveIntoList(std::move(upstream_request), upstream_requests_);
  upstream_requests_.front()->encodeHeaders(end_stream);
}
//重试的时候也要使用 void Filter::doRetry(){}

//HTTP filter
class StreamFilterBase {};
class StreamDecoderFilter : public StreamFilterBase {};
class StreamFilter : public virtual StreamDecoderFilter, public virtual StreamEncoderFilter {};
class Filter : public Http::StreamFilter, public AccessLog::Instance {}

```



```c++
//它作为 MainCommonBase 成员变量，实例化时初始化，参见main
std::unique_ptr<Server::InstanceImpl> server_;

//实例化时->initialize()
InstanceImpl::InstanceImpl() {
	initialize(options, std::move(local_address), component_factory, hooks);
}

//server/server.cc 实例化 ProdClusterManagerFactory
void InstanceImpl::initialize(const Options& options,
                              Network::Address::InstanceConstSharedPtr local_address,
                              ComponentFactory& component_factory, ListenerHooks& hooks) {
  cluster_manager_factory_ = std::make_unique<Upstream::ProdClusterManagerFactory>(
      *admin_, Runtime::LoaderSingleton::get(), stats_store_, thread_local_, *random_generator_,
      dns_resolver_, *ssl_context_manager_, *dispatcher_, *local_info_, *secret_manager_,
      messageValidationContext(), *api_, http_context_, grpc_context_, access_log_manager_,
      *singleton_manager_);
}
```



## WASM

```c++
class Context : public proxy_wasm::ContextBase,
                public Logger::Loggable<Logger::Id::wasm>,
                public AccessLog::Instance,
                public Http::StreamFilter,
                public Network::ConnectionCallbacks,
                public Network::Filter,
                public google::api::expr::runtime::BaseActivation,
                public std::enable_shared_from_this<Context> {}
//创建 Context
createFilter()  {
  std::make_shared<Context>(wasm, root_context_id_, plugin_);
}

//工厂调用
Http::FilterFactoryCb WasmFilterConfig::createFilterFactoryFromProtoTyped(
    const envoy::extensions::filters::http::wasm::v3::Wasm& proto_config, const std::string&,
    Server::Configuration::FactoryContext& context) {
  auto filter_config = std::make_shared<FilterConfig>(proto_config, context);
  return [filter_config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    auto filter = filter_config->createFilter();
    if (!filter) { // Fail open
      return;
    }
    callbacks.addStreamFilter(filter);
    callbacks.addAccessLogHandler(filter);
  };
}

//add
    void addStreamFilter(StreamFilterSharedPtr filter) override {
      addStreamDecoderFilterWorker(filter, true);
      addStreamEncoderFilterWorker(filter, true);
    }
//会存储在一个链表中，然后再遍历链表执行插件
插件会继承这个Context，然后执行方法重载。
```



EnvoyFilter协议

### EnvoyFilter

```yaml
apiVersion: networking.istio.io/v1alpha3
kind: EnvoyFilter
metadata:
  name: demo-filter
  namespace: istio-system
spec:
  configPatches:
  - applyTo: HTTP_FILTER
    match:
      context: GATEWAY
      listener:
        filterChain:
          filter:
            name: envoy.http_connection_manager
            subFilter:
              name: envoy.router
    patch:
      operation: INSERT_BEFOREs
      value:
        typed_config:
          '@type': type.googleapis.com/udpa.type.v1.TypedStruct
          type_url: type.googleapis.com/envoy.extensions.filters.http.wasm.v3.Wasm
          value: # WasmService
            config: # PluginConfig
              name: demofilter
              root_id: demofilter
              vm_config: # VmConfig
                code: # AsyncDataSource
                  local: # DataSource
                    filename: /data/wasm-filter/_wasm_demo.wasm
                runtime: envoy.wasm.runtime.v8
                vm_id: demofilter
        name: envoy.filters.http.wasm
  workloadSelector:
    labels:
      app: istio-ingressgateway
```

### CRD

crd的完整定义：

envoyfilters.networking.istio.io

### proto

```protobuf
syntax = "proto3";

package envoy.extensions.wasm.v3;

import "envoy/config/core/v3/base.proto";

import "google/protobuf/any.proto";

import "udpa/annotations/status.proto";
import "udpa/annotations/versioning.proto";
import "validate/validate.proto";

option java_package = "io.envoyproxy.envoy.extensions.wasm.v3";
option java_outer_classname = "WasmProto";
option java_multiple_files = true;
option (udpa.annotations.file_status).package_version_status = ACTIVE;

// [#protodoc-title: Wasm service]

// Configuration for a Wasm VM.
// [#next-free-field: 6]
message VmConfig {
  // An ID which will be used along with a hash of the wasm code (or the name of the registered Null
  // VM plugin) to determine which VM will be used for the plugin. All plugins which use the same
  // *vm_id* and code will use the same VM. May be left blank. Sharing a VM between plugins can
  // reduce memory utilization and make sharing of data easier which may have security implications.
  // See ref: "TODO: add ref" for details.
  string vm_id = 1;

  // The Wasm runtime type (either "v8" or "null" for code compiled into Envoy).
  string runtime = 2 [(validate.rules).string = {min_bytes: 1}];

  // The Wasm code that Envoy will execute.
  config.core.v3.AsyncDataSource code = 3;

  // The Wasm configuration used in initialization of a new VM (proxy_on_start).
  google.protobuf.Any configuration = 4;

  // Allow the wasm file to include pre-compiled code on VMs which support it.
  // Warning: this should only be enable for trusted sources as the precompiled code is not
  // verified.
  bool allow_precompiled = 5;
}

// Base Configuration for Wasm Plugins e.g. filters and services.
// [#next-free-field: 6]
message PluginConfig {
  // A unique name for a filters/services in a VM for use in identifying the filter/service if
  // multiple filters/services are handled by the same *vm_id* and *group_name* and for
  // logging/debugging.
  string name = 1;

  // A unique ID for a set of filters/services in a VM which will share a RootContext and Contexts
  // if applicable (e.g. an Wasm HttpFilter and an Wasm AccessLog). If left blank, all
  // filters/services with a blank root_id with the same *vm_id* will share Context(s).
  string root_id = 2;

  // Configuration for finding or starting VM.
  oneof vm {
    VmConfig vm_config = 3;
    // TODO: add referential VM configurations.
  }

  // Filter/service configuration used to configure or reconfigure a plugin
  // (proxy_on_configuration).
  google.protobuf.Any configuration = 4;

  // If there is a fatal error on the VM (e.g. exception, abort(), on_start or on_configure return false),
  // then all plugins associated with the VM will either fail closed (by default), e.g. by returning an HTTP 503 error,
  // or fail open (if 'fail_open' is set to true) by bypassing the filter. Note: when on_start or on_configure return false
  // during xDS updates the xDS configuration will be rejected and when on_start or on_configuration return false on initial
  // startup the proxy will not start.
  bool fail_open = 5;
}

// WasmService is configured as a built-in *envoy.wasm_service* :ref:`ServiceConfig
// <envoy_api_msg_extensions.wasm.v3.WasmService>`. This opaque configuration will be used to
// create a Wasm Service.
message WasmService {
  // General plugin configuration.
  PluginConfig config = 1;

  // If true, create a single VM rather than creating one VM per worker. Such a singleton can
  // not be used with filters.
  bool singleton = 2;
}

```



```protobuf
// Async data source which support async data fetch.
message AsyncDataSource {
  option (udpa.annotations.versioning).previous_message_type = "envoy.api.v2.core.AsyncDataSource";

  oneof specifier {
    option (validate.required) = true;

    // Local async data source.
    DataSource local = 1;

    // Remote async data source.
    RemoteDataSource remote = 2;
  }
}

// Data source consisting of either a file or an inline value.
message DataSource {
  option (udpa.annotations.versioning).previous_message_type = "envoy.api.v2.core.DataSource";

  oneof specifier {
    option (validate.required) = true;

    // Local filesystem data source.
    string filename = 1 [(validate.rules).string = {min_bytes: 1}];

    // Bytes inlined in the configuration.
    bytes inline_bytes = 2 [(validate.rules).bytes = {min_len: 1}];

    // String inlined in the configuration.
    string inline_string = 3 [(validate.rules).string = {min_bytes: 1}];
  }
}

// The message specifies how to fetch data from remote and how to verify it.
message RemoteDataSource {
  option (udpa.annotations.versioning).previous_message_type = "envoy.api.v2.core.RemoteDataSource";

  // The HTTP URI to fetch the remote data.
  HttpUri http_uri = 1 [(validate.rules).message = {required: true}];

  // SHA256 string for verifying data.
  string sha256 = 2 [(validate.rules).string = {min_bytes: 1}];

  // Retry policy for fetching remote data.
  RetryPolicy retry_policy = 3;
}
```



```protobuf
// Message type for extension configuration.
// [#next-major-version: revisit all existing typed_config that doesn't use this wrapper.].
message TypedExtensionConfig {
  option (udpa.annotations.versioning).previous_message_type =
      "envoy.config.core.v3.TypedExtensionConfig";

  // The name of an extension. This is not used to select the extension, instead
  // it serves the role of an opaque identifier.
  string name = 1 [(validate.rules).string = {min_len: 1}];

  // The typed config for the extension. The type URL will be used to identify
  // the extension. In the case that the type URL is *udpa.type.v1.TypedStruct*,
  // the inner type URL of *TypedStruct* will be utilized. See the
  // :ref:`extension configuration overview
  // <config_overview_extension_configuration>` for further details.
  google.protobuf.Any typed_config = 2 [(validate.rules).any = {required: true}];
}
```



初始化链路

```c++
   HttpConnectionManagerConfig::HttpConnectionManagerConfig() {
       const auto& filters = config.http_filters(); //config:envoy::extensions::filters::network::http_connection_manager::v3::HttpConnectionManager
  for (int32_t i = 0; i < filters.size(); i++) {
    processFilter(filters[i], i, "http", filter_factories_, "http", i == filters.size() - 1);
  }
   }
void HttpConnectionManagerConfig::processFilter(...) {
     Http::FilterFactoryCb callback =
      factory.createFilterFactoryFromProto(*message, stats_prefix_, context_);
     
      filter_factories.push_back(callback);//std::list<Http::FilterFactoryCb>&
   }
Network::FilterFactoryC createFilterFactoryFromProto(const Protobuf::Message& proto_config,
                               Server::Configuration::FactoryContext& context) override {
    return createFilterFactoryFromProtoTyped(MessageUtil::downcastAndValidate<const ConfigProto&>(
                                                 proto_config, context.messageValidationVisitor()),
                                             context);
  }

Http::FilterFactoryCb WasmFilterConfig::createFilterFactoryFromProtoTyped(
    const envoy::extensions::filters::http::wasm::v3::Wasm& proto_config, const std::string&,
    Server::Configuration::FactoryContext& context) {
  auto filter_config = std::make_shared<FilterConfig>(proto_config, context);
  return [filter_config](Http::FilterChainFactoryCallbacks& callbacks) -> void {
    auto filter = filter_config->createFilter();
    if (!filter) { // Fail open
      return;
    }
    callbacks.addStreamFilter(filter);
    callbacks.addAccessLogHandler(filter);
  };
}

/**
 * Static registration for the Wasm filter. @see RegisterFactory.
 */
REGISTER_FACTORY(WasmFilterConfig, Server::Configuration::NamedHttpFilterConfigFactory);


FilterConfig::FilterConfig(const envoy::extensions::filters::http::wasm::v3::Wasm& config,
                           Server::Configuration::FactoryContext& context{}
void createWasm(){}
                          
```



```c++
void HttpConnectionManagerConfig::processFilter(
    const envoy::extensions::filters::network::http_connection_manager::v3::HttpFilter&
        proto_config,
    int i, absl::string_view prefix, std::list<Http::FilterFactoryCb>& filter_factories,
    const char* filter_chain_type, bool last_filter_in_current_config) {
  ENVOY_LOG(debug, "    {} filter #{}", prefix, i);
  ENVOY_LOG(debug, "      name: {}", proto_config.name());
  ENVOY_LOG(debug, "    config: {}",
            MessageUtil::getJsonStringFromMessage(
                proto_config.has_typed_config()
                    ? static_cast<const Protobuf::Message&>(proto_config.typed_config())
                    : static_cast<const Protobuf::Message&>(
                          proto_config.hidden_envoy_deprecated_config()),
                true));

  // Now see if there is a factory that will accept the config.
  auto& factory =
      Config::Utility::getAndCheckFactory<Server::Configuration::NamedHttpFilterConfigFactory>(
          proto_config);
  ProtobufTypes::MessagePtr message = Config::Utility::translateToFactoryConfig(
      proto_config, context_.messageValidationVisitor(), factory);
  Http::FilterFactoryCb callback =
      factory.createFilterFactoryFromProto(*message, stats_prefix_, context_);
  bool is_terminal = factory.isTerminalFilter();
  Config::Utility::validateTerminalFilters(proto_config.name(), factory.name(), filter_chain_type,
                                           is_terminal, last_filter_in_current_config);
  filter_factories.push_back(callback);
}

//getAndCheckFactory 
  template <class Factory, class ProtoMessage>
  static Factory& getAndCheckFactory(const ProtoMessage& message) {
    const ProtobufWkt::Any& typed_config = message.typed_config();
    static const std::string& typed_struct_type =
        udpa::type::v1::TypedStruct::default_instance().GetDescriptor()->full_name();

    if (!typed_config.type_url().empty()) {
      // Unpack methods will only use the fully qualified type name after the last '/'.
      // https://github.com/protocolbuffers/protobuf/blob/3.6.x/src/google/protobuf/any.proto#L87
      auto type = std::string(TypeUtil::typeUrlToDescriptorFullName(typed_config.type_url()));
      if (type == typed_struct_type) {
        udpa::type::v1::TypedStruct typed_struct;
        MessageUtil::unpackTo(typed_config, typed_struct);
        // Not handling nested structs or typed structs in typed structs
        type = std::string(TypeUtil::typeUrlToDescriptorFullName(typed_struct.type_url()));
      }
      Factory* factory = Registry::FactoryRegistry<Factory>::getFactoryByType(type);
      if (factory != nullptr) {
        return *factory;
      }
    }

    return Utility::getAndCheckFactoryByName<Factory>(message.name());
  }


class WasmFilterConfig
    : public Common::FactoryBase<envoy::extensions::filters::network::wasm::v3::Wasm> {
      //工厂初始化为 envoy::extensions::filters::network::wasm::v3::Wasm
public:
      //构造函数传入  WebAssembly filter名称存入 FactoryBase 的name_成员变量
  WasmFilterConfig() : FactoryBase(NetworkFilterNames::get().Wasm) {}

private:
  Network::FilterFactoryCb createFilterFactoryFromProtoTyped(
      const envoy::extensions::filters::network::wasm::v3::Wasm& proto_config,
      Server::Configuration::FactoryContext& context) override;
};

//WasmFilterConfig() : FactoryBase(NetworkFilterNames::get().Wasm) {}
class NetworkFilterNameValues {
public:
  // WebAssembly filter
  const std::string Wasm = "envoy.filters.network.wasm";
};

using NetworkFilterNames = ConstSingleton<NetworkFilterNameValues>;

//const Protobuf::Message类型，使用downcastAndValidate工具强转成envoy::extensions::filters::network::wasm::v3::Wasm类型,最终调用上文的wasm的createFilterFactoryFromProtoTyped
  Network::FilterFactoryCb
  createFilterFactoryFromProto(const Protobuf::Message& proto_config,
                               Server::Configuration::FactoryContext& context) override {
    return createFilterFactoryFromProtoTyped(MessageUtil::downcastAndValidate<const ConfigProto&>(
                                                 proto_config, context.messageValidationVisitor()),
                                             context);
  }

```

## gRPC

### Debug Log

```shell
2020-07-23T02:59:39.441617Z     critical        envoy backtrace Caught Segmentation fault, suspect faulting address 0x374
2020-07-23T02:59:39.441703Z     critical        envoy backtrace Backtrace (use tools/stack_decode.py to get line numbers):
2020-07-23T02:59:39.441752Z     critical        envoy backtrace Envoy version: 3aaff15a85a5c0cc5c23635d4b858eee001df836/1.15.0-dev/Clean/RELEASE/BoringSSL
2020-07-23T02:59:39.442138Z     critical        envoy backtrace #0: __restore_rt [0x7f53d388b8a0]
2020-07-23T02:59:39.534222Z     critical        envoy backtrace #1: Envoy::Router::Filter::decodeData() [0x55e5fde26516]
2020-07-23T02:59:39.542064Z     critical        envoy backtrace #2: Envoy::Http::AsyncStreamImpl::sendData() [0x55e5fde1d5f1]
2020-07-23T02:59:39.550356Z     critical        envoy backtrace #3: Envoy::Grpc::AsyncStreamImpl::closeStream() [0x55e5fde17374]
2020-07-23T02:59:39.559161Z     critical        envoy backtrace #4: Envoy::Extensions::Common::Wasm::Context::grpcClose() [0x55e5fce11057]
2020-07-23T02:59:39.568031Z     critical        envoy backtrace #5: proxy_wasm::exports::grpc_close() [0x55e5fcf512c7]
2020-07-23T02:59:39.620008Z     critical        envoy backtrace #6: proxy_wasm::(anonymous namespace)::V8::registerHostFunctionImpl<>()::{lambda()#1}::__invoke() [0x55e5fcf78571]
2020-07-23T02:59:39.628901Z     critical        envoy backtrace #7: wasm::FuncData::v8_callback() [0x55e5fcf87a5f]
2020-07-23T02:59:39.629324Z     critical        envoy backtrace #8: [0x17a638901ddd]
2020-07-23T02:59:39.629824Z     critical        envoy backtrace #9: [0x17a6384ea32f]
2020-07-23T02:59:39.630269Z     critical        envoy backtrace #10: [0x17a6384eeae6]
2020-07-23T02:59:39.630731Z     critical        envoy backtrace #11: [0x17a6384d5026]
2020-07-23T02:59:39.631187Z     critical        envoy backtrace #12: [0x1baf00083851]
2020-07-23T02:59:39.639116Z     critical        envoy backtrace #13: v8::internal::Execution::CallWasm() [0x55e5fd0652c3]
2020-07-23T02:59:39.648181Z     critical        envoy backtrace #14: wasm::Func::call() [0x55e5fcf86db5]
2020-07-23T02:59:39.657178Z     critical        envoy backtrace #15: std::__1::__function::__func<>::operator()() [0x55e5fcf746c5]
2020-07-23T02:59:39.665901Z     critical        envoy backtrace #16: proxy_wasm::ContextBase::onGrpcClose() [0x55e5fcf4b23a]
2020-07-23T02:59:39.718678Z     critical        envoy backtrace #17: Envoy::Extensions::Common::Wasm::Context::onGrpcCloseWrapper() [0x55e5fce10c39]
2020-07-23T02:59:39.726213Z     critical        envoy backtrace #18: Envoy::Extensions::Common::Wasm::Context::GrpcStreamClientHandler::onRemoteClose() [0x55e5fce1633d]
2020-07-23T02:59:39.734126Z     critical        envoy backtrace #19: Envoy::Grpc::AsyncStreamImpl::onTrailers() [0x55e5fde1704b]
2020-07-23T02:59:39.741744Z     critical        envoy backtrace #20: Envoy::Grpc::AsyncStreamImpl::onHeaders() [0x55e5fde16ceb]
2020-07-23T02:59:39.750337Z     critical        envoy backtrace #21: Envoy::Http::AsyncStreamImpl::encodeHeaders() [0x55e5fde1c768]
2020-07-23T02:59:39.758046Z     critical        envoy backtrace #22: Envoy::Http::Utility::sendLocalReply() [0x55e5fdf4ac2a]
2020-07-23T02:59:39.765573Z     critical        envoy backtrace #23: Envoy::Http::AsyncStreamImpl::sendLocalReply() [0x55e5fde1e566]
2020-07-23T02:59:39.818718Z     critical        envoy backtrace #24: Envoy::Router::Filter::onUpstreamAbort() [0x55e5fde28801]
2020-07-23T02:59:39.826282Z     critical        envoy backtrace #25: Envoy::Router::Filter::onUpstreamReset() [0x55e5fde28e3d]
2020-07-23T02:59:39.835171Z     critical        envoy backtrace #26: Envoy::Router::UpstreamRequest::onResetStream() [0x55e5fde32c6e]
2020-07-23T02:59:39.843943Z     critical        envoy backtrace #27: Envoy::Http::Http1::ClientConnectionImpl::onResetStream() [0x55e5fdeb0e3f]
2020-07-23T02:59:39.852665Z     critical        envoy backtrace #28: Envoy::Http::CodecClient::onEvent() [0x55e5fde0c707]
2020-07-23T02:59:39.861580Z     critical        envoy backtrace #29: Envoy::Network::ConnectionImplBase::raiseConnectionEvent() [0x55e5fdc2d2ed]
2020-07-23T02:59:39.868347Z     critical        envoy backtrace #30: Envoy::Network::ConnectionImpl::closeSocket() [0x55e5fdc278f2]
2020-07-23T02:59:39.918618Z     critical        envoy backtrace #31: Envoy::Network::ConnectionImpl::close() [0x55e5fdc27586]
2020-07-23T02:59:39.925827Z     critical        envoy backtrace #32: Envoy::Http::CodecClient::onData() [0x55e5fde0cc8d]
2020-07-23T02:59:39.933464Z     critical        envoy backtrace #33: Envoy::Http::CodecClient::CodecReadFilter::onData() [0x55e5fde0d8dd]
2020-07-23T02:59:39.942119Z     critical        envoy backtrace #34: Envoy::Network::FilterManagerImpl::onContinueReading() [0x55e5fdc2da63]
2020-07-23T02:59:39.949501Z     critical        envoy backtrace #35: Envoy::Network::ConnectionImpl::onReadReady() [0x55e5fdc29bee]
2020-07-23T02:59:39.957126Z     critical        envoy backtrace #36: Envoy::Network::ConnectionImpl::onFileEvent() [0x55e5fdc28ced]
2020-07-23T02:59:39.966116Z     critical        envoy backtrace #37: Envoy::Event::FileEventImpl::assignEvents()::$_0::__invoke() [0x55e5fdc23180]
2020-07-23T02:59:40.022911Z     critical        envoy backtrace #38: event_process_active_single_queue [0x55e5fe081c4b]
2020-07-23T02:59:40.031746Z     critical        envoy backtrace #39: event_base_loop [0x55e5fe0804ce]
2020-07-23T02:59:40.040703Z     critical        envoy backtrace #40: Envoy::Server::InstanceImpl::run() [0x55e5fdba96ba]
2020-07-23T02:59:40.049321Z     critical        envoy backtrace #41: Envoy::MainCommonBase::run() [0x55e5fc62c918]
2020-07-23T02:59:40.056968Z     critical        envoy backtrace #42: Envoy::MainCommon::main() [0x55e5fc62d0f5]
2020-07-23T02:59:40.065817Z     critical        envoy backtrace #43: main [0x55e5fc62b31c]
2020-07-23T02:59:40.066113Z     critical        envoy backtrace #44: __libc_start_main [0x7f53d34a9b97]
```



```shell
2020-07-23T08:34:50.068865Z     debug   envoy wasm      wasm log demofilter: [./extensions/mixless/demo/plugin.h:145]::onConfigure() grpcStreamHandler, result: Ok
2020-07-23T08:34:50.069849Z     trace   envoy grpc      handleOpCompletion op=0 ok=true inflight=1
2020-07-23T08:34:51.843911Z     debug   envoy conn_handler      [C2] new connection
2020-07-23T08:34:51.843957Z     trace   envoy connection        [C2] socket event: 2
2020-07-23T08:34:51.843961Z     trace   envoy connection        [C2] write ready
2020-07-23T08:34:51.843964Z     trace   envoy connection        [C2] socket event: 3
2020-07-23T08:34:51.843966Z     trace   envoy connection        [C2] write ready
2020-07-23T08:34:51.843969Z     trace   envoy connection        [C2] read ready. dispatch_buffered_data=false
2020-07-23T08:34:51.843978Z     trace   envoy connection        [C2] read returns: 129
2020-07-23T08:34:51.843987Z     trace   envoy connection        [C2] read error: Resource temporarily unavailable
2020-07-23T08:34:51.844033Z     trace   envoy http      [C2] parsing 129 bytes
2020-07-23T08:34:51.844040Z     trace   envoy http      [C2] message begin
2020-07-23T08:34:51.844046Z     debug   envoy http      [C2] new stream
2020-07-23T08:34:51.844069Z     trace   envoy http      [C2] completed header: key=Host value=172.16.0.161:15021
2020-07-23T08:34:51.844089Z     trace   envoy http      [C2] completed header: key=User-Agent value=kube-probe/1.16+
2020-07-23T08:34:51.844097Z     trace   envoy http      [C2] completed header: key=Accept-Encoding value=gzip
2020-07-23T08:34:51.844115Z     trace   envoy http      [C2] onHeadersCompleteBase
2020-07-23T08:34:51.844118Z     trace   envoy http      [C2] completed header: key=Connection value=close
2020-07-23T08:34:51.844127Z     trace   envoy http      [C2] Server: onHeadersComplete size=4
2020-07-23T08:34:51.844141Z     trace   envoy http      [C2] message complete
2020-07-23T08:34:51.844146Z     trace   envoy connection        [C2] readDisable: disable=true disable_count=0 state=0 buffer_length=129
```



```c++
source/common/grpc/google_async_client_impl.h
class GoogleAsyncRequestImpl : public AsyncRequest,
                               public GoogleAsyncStreamImpl,
                               RawAsyncStreamCallbacks {
public:
  GoogleAsyncRequestImpl(GoogleAsyncClientImpl& parent, absl::string_view service_full_name,
                         absl::string_view method_name, Buffer::InstancePtr request,
                         RawAsyncRequestCallbacks& callbacks, Tracing::Span& parent_span,
                         const Http::AsyncClient::RequestOptions& options);

  void initialize(bool buffer_body_for_retry) override;

  // Grpc::AsyncRequest
  void cancel() override;

private:
  // Grpc::RawAsyncStreamCallbacks
  void onCreateInitialMetadata(Http::RequestHeaderMap& metadata) override;
  void onReceiveInitialMetadata(Http::ResponseHeaderMapPtr&&) override;
  bool onReceiveMessageRaw(Buffer::InstancePtr&& response) override;
  void onReceiveTrailingMetadata(Http::ResponseTrailerMapPtr&&) override;
  void onRemoteClose(Grpc::Status::GrpcStatus status, const std::string& message) override;

  Buffer::InstancePtr request_;
  RawAsyncRequestCallbacks& callbacks_;
  Tracing::SpanPtr current_span_;
  Buffer::InstancePtr response_;
};
```





## Envoy

```shell
# docker run -it --rm envoyproxy/envoy:v1.14-latest envoy --help

USAGE: 

   envoy  [--disable-extensions <string>] [--use-fake-symbol-table <bool>]
          [--cpuset-threads] [--enable-mutex-tracing]
          [--disable-hot-restart] [--max-obj-name-len <uint64_t>]
          [--max-stats <uint64_t>] [--mode <string>]
          [--parent-shutdown-time-s <uint32_t>] [--drain-time-s <uint32_t>]
          [--file-flush-interval-msec <uint32_t>] [--service-zone <string>]
          [--service-node <string>] [--service-cluster <string>]
          [--hot-restart-version] [--restart-epoch <uint32_t>] [--log-path
          <string>] [--log-format-escaped] [--log-format <string>]
          [--component-log-level <string>] [-l <string>]
          [--local-address-ip-version <string>] [--admin-address-path
          <string>] [--reject-unknown-dynamic-fields]
          [--allow-unknown-static-fields] [--allow-unknown-fields]
          [--config-yaml <string>] [-c <string>] [--concurrency <uint32_t>]
          [--base-id <uint32_t>] [--] [--version] [-h]


Where: 

   --disable-extensions <string>
     Comma-separated list of extensions to disable

   --use-fake-symbol-table <bool>
     Use fake symbol table implementation

   --cpuset-threads
     Get the default # of worker threads from cpuset size

   --enable-mutex-tracing
     Enable mutex contention tracing functionality

   --disable-hot-restart
     Disable hot restart functionality

   --max-obj-name-len <uint64_t>
     Deprecated and unused; please do not specify.

   --max-stats <uint64_t>
     Deprecated and unused; please do not specify.

   --mode <string>
     One of 'serve' (default; validate configs and then serve traffic
     normally) or 'validate' (validate configs and exit).

   --parent-shutdown-time-s <uint32_t>
     Hot restart parent shutdown time in seconds

   --drain-time-s <uint32_t>
     Hot restart and LDS removal drain time in seconds

   --file-flush-interval-msec <uint32_t>
     Interval for log flushing in msec

   --service-zone <string>
     Zone name #Enovy部署的可用区

   --service-node <string>
     Node name #envoy的本地节点名称

   --service-cluster <string>
     Cluster name #envoy本地服务集群

   --hot-restart-version
     hot restart compatibility version

   --restart-epoch <uint32_t>
     hot restart epoch #热重启周期

   --log-path <string>
     Path to logfile

   --log-format-escaped
     Escape c-style escape sequences in the application logs

   --log-format <string>
     Log message format in spdlog syntax (see
     https://github.com/gabime/spdlog/wiki/3.-Custom-formatting)

     Default is "[%Y-%m-%d %T.%e][%t][%l][%n] %v"

   --component-log-level <string>
     Comma separated list of component log levels. For example
     upstream:debug,config:trace

   -l <string>,  --log-level <string>
     Log levels: [trace][debug][info][warning
     |warn][error][critical][off]

     Default is [info]

   --local-address-ip-version <string>
     The local IP address version (v4 or v6).

   --admin-address-path <string>
     Admin address path

   --reject-unknown-dynamic-fields
     reject unknown fields in dynamic configuration

   --allow-unknown-static-fields
     allow unknown fields in static configuration

   --allow-unknown-fields
     allow unknown fields in static configuration (DEPRECATED)

   --config-yaml <string>
     Inline YAML configuration, merges with the contents of --config-path

   -c <string>,  --config-path <string>
     Path to configuration file

   --concurrency <uint32_t>
     # of worker threads to run

   --base-id <uint32_t>
     base ID so that multiple envoys can run on the same host if needed

   --,  --ignore_rest
     Ignores the rest of the labeled arguments following this flag.

   --version
     Displays version information and exits.

   -h,  --help
     Displays usage information and exits.


   envoy
```

如果直接启动会报没有配置文件的错误

```she
error initializing configuration '': At least one of --config-path or --config-yaml or Options::configProto() should be non-empty
```



## 参考资料

[envoy-1.15](https://www.bookstack.cn/read/envoy-1.15/07535d2404dfe4ec.md)

