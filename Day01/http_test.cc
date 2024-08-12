#include <workflow/WFFacilities.h>  // 用于创建等待组
#include <workflow/WFTaskFactory.h>  // 包含创建任务相关的函数
#include <workflow/HttpUtil.h>       // 包含处理HTTP的工具
#include <iostream>
#include <string>
#include <signal.h>                  // 用于处理信号，如 SIGINT

using std::cout;
using std::cerr;
using std::string;

// 创建一个等待组对象，初始计数为1
static WFFacilities::WaitGroup waitGroup(1);

// 信号处理函数，当收到指定信号（如SIGINT）时调用
void sigHandler(int signum) {
    cout << "done!\n";
    waitGroup.done();  // 计数减一，表示任务完成
}

// HTTP 请求的回调函数，当请求完成后调用
void httpCallback(WFHttpTask *httpTask) {
    // 检查请求的状态和错误
    int state = httpTask->get_state(); // 获取任务的状态
    int error = httpTask->get_error(); // 获取任务的错误码

    // 根据不同的状态处理错误
    switch (state) {
    case WFT_STATE_SYS_ERROR:  // 系统错误
        cerr << "system error: " << strerror(error) << "\n";
        break;
    case WFT_STATE_DNS_ERROR:  // DNS错误
        cerr << "DNS error: " << gai_strerror(error) << "\n";
        break;
    case WFT_STATE_SSL_ERROR:  // SSL错误
        cerr << "SSL error: " << error << "\n";
        break;
    case WFT_STATE_TASK_ERROR:  // 任务错误
        cerr << "Task error: " << error << "\n";
        break;
    case WFT_STATE_SUCCESS:  // 请求成功
        break;
    }

    // 如果请求未成功，打印错误信息并返回
    if (state != WFT_STATE_SUCCESS) {
        cerr << "Failed. Press Ctrl-C to exit.\n";
        return;
    }

    // 获取请求的信息并打印
    protocol::HttpRequest * req = httpTask->get_req();
    cout << "http request method = " << req->get_method() << "\n";           // 打印请求方法（如GET、POST）
    cout << "http request uri = " << req->get_request_uri() << "\n";         // 打印请求的URI
    cout << "http request version = " << req->get_http_version() << "\n";    // 打印HTTP版本

    // 遍历并打印请求的头部字段
    protocol::HttpHeaderCursor reqCursor(req);
    string key, value;
    while (reqCursor.next(key, value)) {
        cout << "key = " << key << " value = " << value << "\n";
    }

    // 分割线用于区分请求和响应信息
    cout << "--------------------------------------------------\n";

    // 获取响应的信息并打印
    protocol::HttpResponse *resp = httpTask->get_resp();
    cout << "http response version = " << resp->get_http_version() << "\n";  // 打印响应的HTTP版本
    cout << "http response status code = " << resp->get_status_code() << "\n"; // 打印状态码
    cout << "http response reason phrase = " << resp->get_reason_phrase() << "\n"; // 打印原因短语

    // 遍历并打印响应的头部字段
    protocol::HttpHeaderCursor respCursor(resp);
    while (respCursor.next(key, value)) {
        cout << "key = " << key << " value = " << value << "\n";
    }

    // 获取并打印响应体
    const void *body; // 指向响应体的指针
    size_t size;      // 响应体的大小
    resp->get_parsed_body(&body, &size);
    cout << static_cast<const char *>(body) << "\n"; // 打印响应体
}

int main() {
    // 注册SIGINT信号处理函数
    signal(SIGINT, sigHandler);

    // 创建HTTP任务，请求百度主页
    WFHttpTask * httpTask = WFTaskFactory::create_http_task(
        "http://www.baidu.com",  // 请求的URL
        10,                      // 最大重定向次数
        10,                      // 重试次数
        httpCallback);           // 请求完成后的回调函数

    // 设置请求的头部字段
    protocol::HttpRequest * req = httpTask->get_req();
    req->add_header_pair("key1", "value1"); // 添加一个自定义头部字段

    // 启动任务
    httpTask->start();

    // 等待任务完成
    waitGroup.wait();

    cout << "finished!\n";  // 程序结束提示
    return 0;
}

