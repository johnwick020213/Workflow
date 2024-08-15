#include<signal.h>
#include<workflow/WFFacilities.h>
#include<workflow/WFTaskFactory.h>
#include<iostream>

using std::cout;
using std::cerr;

static WFFacilities::WaitGroup WaitGroup(1);

void sighandler(int signum)
{
    cout<<"\ndone\n";
    WaitGroup.done();
}

void redisCallback(WFRedisTask *redisTask)
{
    // 检查报错
    int state = redisTask->get_state(); //获取状态
	int error = redisTask->get_error(); //获取errno
	switch (state){
	case WFT_STATE_SYS_ERROR: //操作系统层面的错误
		cerr << "system error: " << strerror(error) << "\n";
		break;
	case WFT_STATE_DNS_ERROR: //网络信息错误
		cerr << "DNS error: " << gai_strerror(error) << "\n";
		break;
	case WFT_STATE_SUCCESS:
		break;
	}

    protocol::RedisResponse *resp=redisTask->get_resp();
    protocol::RedisValue result;
    resp->get_result(result);
    if(result.is_error())
    {
        cerr<<"Redis.error\n";
        state=WFT_STATE_TASK_ERROR;
    }

    if(state!=WFT_STATE_SUCCESS)
    {
        cerr<<"Failed.Press Ctrl-C tp exit.\n";
        return;
    }

    if(result.is_string())
    {
        cout<<"result is"<<result.string_value()<<"\n";
    }
    else if(result.is_array())
    {
        cout<<"result is arry()\n";
        for(int i=0;i<result.arr_size();++i)
        {
            cout<<"i="<<i<<"value="<<result.arr_at(i).string_value()<<"\n";
        }
    }
}

int main()
{
    signal(SIGINT,sighandler);
    WFRedisTask *redisTask=WFTaskFactory::create_redis_task(
                        "redis://:020213@127.0.0.1:6379",
                        10,
                        redisCallback
                      );
    protocol::RedisRequest *req=redisTask->get_req();
    //req->set_request("SET",{"58key","value"});
    req->set_request("HGETALL",{"58table"});
    redisTask->start();
    WaitGroup.wait();
    cout<<"finished!\n";
}

