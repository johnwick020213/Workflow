#include <workflow/WFFacilities.h> // 包含 WaitGroup 类的定义，用于实现多线程的同步等待
#include <workflow/WFHttpServer.h> // 包含 WFHttpServer 类的定义，用于创建和管理 HTTP 服务器
#include <iostream>  // 包含标准输入输出流的定义
#include <string>    // 包含字符串类的定义
#include <signal.h>  // 包含信号处理相关的函数和定义
#include <unistd.h>  // 包含 POSIX 操作系统 API 的定义，例如 sleep 函数

using std::cout;
using std::string;

// 用于在不同任务之间共享数据的上下文结构体
struct SeriesContext {
    string name;                      // 用户名
    string password;                  // 密码
    protocol::HttpResponse *resp;     // HTTP 响应指针，用于向客户端发送响应
};

// 全局 WaitGroup 对象，用于主线程等待某些任务完成
static WFFacilities::WaitGroup waitGroup(1);

// 信号处理函数，当接收到指定信号时调用
void sigHandler(int signum) {
    cout << "done!\n";
    waitGroup.done(); // 让 WaitGroup 计数器减1，表示任务完成
}

// Redis 回调函数，当 Redis 任务完成时调用
void redisCallback(WFRedisTask *redisTask) {
    // 获取 Redis 执行结果
    protocol::RedisValue result;
    protocol::RedisResponse *resp = redisTask->get_resp();
    resp->get_result(result);

    // 访问上下文数据
    SeriesContext *context = static_cast<SeriesContext *>(series_of(redisTask)->get_context());
    if (result.is_string() && result.string_value() == context->password) {
        // 如果 Redis 中存储的密码与用户输入的密码匹配
        cout << context->name << " success\n";
        context->resp->append_output_body("<html>success</html>"); // 向客户端返回成功的 HTML 页面
    } else {
        // 如果密码不匹配
        cout << context->name << " fail\n";
        context->resp->append_output_body("<html>fail</html>"); // 向客户端返回失败的 HTML 页面
    }
}

// 处理 HTTP 请求的函数
void process(WFHttpTask *serverTask) {
    // 获取 HTTP 请求
    protocol::HttpRequest *req = serverTask->get_req();
    string uri = req->get_request_uri(); // 获取请求的 URI
    cout << uri << "\n";

    // 解析用户名和密码
    // 例如：/login?username=niqiu&password=123
    string nameKV = uri.substr(0, uri.find("&"));
    string passwordKV = uri.substr(uri.find("&") + 1);
    string name = nameKV.substr(nameKV.find("=") + 1);
    string password = passwordKV.substr(passwordKV.find("=") + 1);
    cout << "name = " << name << ", password = " << password << "\n";

    // 分配内存，用于在序列中的任务之间共享数据
    SeriesContext *context = new SeriesContext;
    context->name = name;
    context->password = password;
    context->resp = serverTask->get_resp(); // 保存 HTTP 响应对象的指针
    series_of(serverTask)->set_context(context); // 将上下文设置为序列的上下文

    // 创建一个 Redis 任务，连接到本地 Redis 服务器
    WFRedisTask *redisTask = WFTaskFactory::create_redis_task("redis://127.0.0.1:6379", 10, redisCallback);

    // 设置 Redis 请求，命令为 HGET，获取哈希表 57user 中 key 为 name 的值
    redisTask->get_req()->set_request("HGET", {"57user", name});

    // 将 Redis 任务添加到当前任务序列中
    series_of(serverTask)->push_back(redisTask);
}

// 主函数，程序入口
int main() {
    // 注册信号处理函数，当接收到 SIGINT (Ctrl+C) 信号时调用 sigHandler
    signal(SIGINT, sigHandler);

    // 创建一个 HTTP 服务器，使用 process 函数处理请求
    WFHttpServer server(process);

    // 启动服务器，监听端口 12345
    if (server.start(12345) == 0) {
        // 如果服务器成功启动，主线程进入等待状态，直到收到信号
        waitGroup.wait();
        server.stop(); // 停止服务器
    } else {
        // 如果服务器启动失败，输出错误信息
        perror("server start failed!");
        return -1;
    }

    cout << "finished!\n";
    return 0;
}

