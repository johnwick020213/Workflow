#include <signal.h>
#include <iostream>
#include <workflow/WFFacilities.h>
#include <workflow/WFTaskFactory.h>

using std::cout;
using std::cerr;
using std::string;
// 静态全局变量，确保 waitGroup 在整个程序运行期间保持有效
static WFFacilities::WaitGroup waitGroup(1);

void sighandler(int signum) {
    cout << "done!\n";  // 尽量避免使用 endl
    waitGroup.done();
}

void redisCallback(WFRedisTask *redisTask, string currentKey, string finalValue) {
    // 获取任务状态和错误信息
    int state = redisTask->get_state();
    int error = redisTask->get_error();

    switch (state) {
    case WFT_STATE_SYS_ERROR:
        cerr << "System error: " << strerror(error) << "\n";
        break;
    case WFT_STATE_DNS_ERROR:
        cerr << "DNS error: " << gai_strerror(error) << "\n";
        break;
    case WFT_STATE_SUCCESS:
        break;
    // 获取 Redis 指令的执行结果
    protocol::RedisResponse *resp = redisTask->get_resp();
    protocol::RedisValue result;
    resp->get_result(result);

    if (result.is_error()) {
        cerr << "Redis error\n";
        state = WFT_STATE_TASK_ERROR;
    }

    if (state != WFT_STATE_SUCCESS) {
        cerr << "Failed. Press Ctrl-C to exit.\n";
        return;
    }

    // 获取键对应的值
    string nextKey = result.string_value();
    cout << "Key: " << currentKey << " --> Value: " << nextKey << "\n";

    // 如果找到了最终的值
    if (nextKey == finalValue) {
        cout << "Found the final value: " << finalValue << "\n";
        waitGroup.done();
        return;
    }

    // 创建下一个 Redis GET 任务
    WFRedisTask *nextTask = WFTaskFactory::create_redis_task(
        "redis://:020213@127.0.0.1:6379", 
        10,
        std::bind(redisCallback, std::placeholders::_1, nextKey, finalValue)
    );
    protocol::RedisRequest *nextReq = nextTask->get_req();
    nextReq->set_request("GET", {nextKey});
    nextTask->start();
}
}

int main()
{
    signal(SIGINT, sighandler);

    string initialKey = "x1";
    string finalValue = "100";

    // 创建初始的 Redis GET 任务
    WFRedisTask *redisTask = WFTaskFactory::create_redis_task(
        "redis://:020213@127.0.0.1:6379", 
        10, 
        std::bind(redisCallback, std::placeholders::_1, initialKey, finalValue)
    );
    protocol::RedisRequest *req = redisTask->get_req();
    req->set_request("GET", {initialKey});
    redisTask->start();

    waitGroup.wait();
    cout << "finished!\n";
}

