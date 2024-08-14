#include<workflow/WFFacilities.h>
#include<workflow/WFTaskFactory.h>
#include<signal.h>
#include<iostream>

using namespace std;
static WFFacilities::WaitGroup waitGroup(1);

void signalhandler(int signum)
{
    cout<<"\ndone\n";
    waitGroup.done();
}


int main()
{
    signal(SIGINT,signalhandler);
    WFHttpTask *httptask=WFTaskFactory::create_http_task(
                        "http://www.baidu.com",
                        10,10,
                        nullptr
                        );
    protocol::HttpRequest *req=httptask->get_req();
    req->add_header_pair("Agent","Workflow");
    httptask->start();

    waitGroup.wait();
    cout<<"finish\n";
                                                        
}

