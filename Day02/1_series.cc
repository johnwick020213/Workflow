#include<signal.h>
#include<workflow/WFFacilities.h>
#include<workflow/WFTaskFactory.h>
#include<iostream>
#include<unistd.h>

using std::cout;
using std::cerr;

static WFFacilities::WaitGroup WaitGroup(1);

void sighandler(int signum)
{
    cout<<"\ndone\n";
    WaitGroup.done();
}

void redisCallback1(WFRedisTask *redisTask)
{
    cout<<"callback1111111\n";
    sleep(5);
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


void redisCallback2(WFRedisTask *redisTask)
{
    cout<<"callback2222\n";
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
    
    
    WFRedisTask *redisTask1 = WFTaskFactory::create_redis_task(
                        "redis://:020213@127.0.0.1:6379",
                        10,
                        redisCallback1
                      );
    protocol::RedisRequest *req1 = redisTask1 -> get_req();
    //req->set_request("SET",{"58key","value"});
    req1->set_request("SET",{"58key","newvalue"});

    WFRedisTask *redisTask2 = WFTaskFactory::create_redis_task(
                        "redis://:020213@127.0.0.1:6379",
                        10,
                        redisCallback2
                      );
    protocol::RedisRequest *req2 = redisTask2 -> get_req();
    //req->set_request("SET",{"58key","value"});
    req2 -> set_request("GET",{"58key"});

    
    SeriesWork *series = Workflow::create_series_work(redisTask1,nullptr);
    series->push_back(redisTask2);
    series->start();

    WaitGroup.wait();
    cout<<"finished!\n";
}

