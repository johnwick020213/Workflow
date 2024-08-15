#include<signal.h>
#include<iostream>
#include<string>
#include<sys/types.h>
#include<sys/stat.h>
#include<workflow/WFFacilities.h>
#include<workflow/WFHttpServer.h>
#include<workflow/WFTaskFactory.h>
#include<unistd.h>
#include<fcntl.h>
using std::cout;
using std::string;
using std::cerr;
struct SeriesContext{
    string name;
    string password;
    protocol::HttpResponse *resp;
};

static WFFacilities::WaitGroup waitGroup(1);

void sighandler(int signum)
{
    cout<<"\ndone\n";
    waitGroup.done();
}

void redisCallback(WFRedisTask *redisTask)
{
    protocol::RedisResponse *respRedis = redisTask->get_resp();
    protocol::RedisValue result;
    respRedis->get_result(result);

    SeriesContext *context = static_cast<SeriesContext*>(series_of(redisTask)->get_context());
   
    if(result.is_string()&&result.string_value()==context->password)
    {
        context->resp->append_output_body("<html>Success</html>");
    }
    else
    {
        context->resp->append_output_body("<html>Fail</html>");
    }
}

void process(WFHttpTask * serverTask)
{
    //req是客户端发来的请求
    protocol::HttpRequest *req = serverTask->get_req();
    //resp是将要回复给客户端的响应
    protocol::HttpResponse *resp = serverTask->get_resp();
    
    string method=req->get_method();
    if(method=="GET")
    {
        int fd=open("postform.html",O_RDWR);
        char *buf=new char[615]{0};
        read=

    }
  
}

int main()
{
    signal(SIGINT,sighandler);
    WFHttpServer server(process);
    if(server.start(12345) == 0){
        // start是非阻塞的
        waitGroup.wait();
        cout << "finished!\n";
        server.stop();
        return 0;
    }
    else{
        perror("server start fail!");
        return -1;
    }
}
