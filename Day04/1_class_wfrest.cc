#include<signal.h>
#include<iostream>
#include<workflow/WFFacilities.h>
#include<wfrest/HttpServer.h>


using std::cout;
using std::map;
using std::string;


static WFFacilities::WaitGroup waitGroup(1);


void sighandler(int signum)
{
    cout<<"\ndone\n";
    waitGroup.done();
}


int main()
{
    signal(SIGINT,sighandler);
    wfrest::HttpServer server;
    
    
    server.GET("/hello",[](const wfrest::HttpReq *req,wfrest::HttpResp *resp)
               {
               cout<<"hello\n";
               //resp->append_output_body("hello");//基类的方法
               resp->String("<html>Hello</html>");//派生类方法
               resp->headers["Content-Type"]="text/html";
               });
    
    
    server.GET("/world",[](const wfrest::HttpReq *req,wfrest::HttpResp *resp)
               {
                    cout<<"world\n";
               });

    
    server.GET("/login",[](const wfrest::HttpReq *req,wfrest::HttpResp *resp,SeriesWork *series)
               {
                    resp->File("postform.html");
               });


    server.POST("/login",[](const wfrest::HttpReq *req,wfrest::HttpResp *resp,SeriesWork *series)
                {
                    //解析urlencoded类型的报文体
                    map<string,string>mapForm=req->form_kv();
                    for(auto pair:mapForm)
                    {
                    cout<<"key="<<pair.first<<"value="<<pair.second<<"\n";
                    }

                    string password = mapForm["password"];
                    WFRedisTask *redisTask = WFTaskFactory::create_redis_task(
                            "redis://:020213@127.0.0.1:6379",10,
                            [resp,password](WFRedisTask *redisTask)
                            {
                                protocol::RedisResponse *respRedis = redisTask -> get_resp();
                                protocol::RedisValue result;
                                respRedis->get_result(result);
                                if(result.is_string()&&result.string_value()==password)
                                {
                                    resp->String("success");
                                }
                                else
                                {
                                    resp->String("fail");
                                }
                            }
                     );

                    redisTask->get_req()->set_request("HGET",{"USER",mapForm["username"]});
                    series->push_back(redisTask);
                });


    server.GET("/upload",[](const wfrest::HttpReq *req,wfrest::HttpResp *resp)
               {
                    resp->File("upload.html");
               });


    server.POST("/upload",[](const wfrest::HttpReq *req,wfrest::HttpResp *resp)
                {
                    map<string,std::pair<string,string>>&formData=req->form();

                    for(auto pair:formData)
                    {
                        cout<<"key="<<pair.first
                        <<"value1="<<pair.second.first
                        //<<"value2="<<pair.second.second
                        <<"\n";
                    }

                    int fd = open(formData["file"].first.c_str(),O_RDWR|O_CREAT,0666);
                    write(fd,formData["file"].second.c_str(),formData["file"].second.length());
                    close(fd);
                });


    if(server.track().start(12345) == 0){
        // start是非阻塞的
        server.list_routes();
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


