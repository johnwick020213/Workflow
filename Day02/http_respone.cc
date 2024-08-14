#include<workflow/WFFacilities.h>
#include<workflow/WFTaskFactory.h>
#include<signal.h>
#include<iostream>
#include <workflow/HttpUtil.h>
using namespace std;
static WFFacilities::WaitGroup waitGroup(1);

void signalhandler(int signum)
{
    cout<<"\ndone\n";
    waitGroup.done();
}

void httpcallback(WFHttpTask *httptask)
{
     // 检查报错
    int state = httptask->get_state(); //获取状态
	int error = httptask->get_error(); //获取errno
	switch (state){
	case WFT_STATE_SYS_ERROR:
		cerr << "system error: " << strerror(error) << "\n";
		break;
	case WFT_STATE_DNS_ERROR:
		cerr << "DNS error: " << gai_strerror(error) << "\n";
		break;
	case WFT_STATE_SSL_ERROR:
		cerr <<"SSL error: " << error << "\n";
		break;
	case WFT_STATE_TASK_ERROR:
		cerr <<"Task error: " << error << "\n"; 
		break;
	case WFT_STATE_SUCCESS:
		break;
	}
	if (state != WFT_STATE_SUCCESS){
		cerr << "Failed. Press Ctrl-C to exit.\n";
		return;
	}

    protocol::HttpRequest *req=httptask->get_req();
    cout<<"http request method = "<<req->get_method()<<"\n";
    cout<<"http request uri = "<<req->get_request_uri()<<"\n";
    cout<<"http request version = "<<req->get_http_version()<<"\n";

    protocol::HttpHeaderCursor reqCursor(req); 
    string key,value;
    while(reqCursor.next(key,value))
    {
        cout<<"key = "<<key<<"value="<<value<<"\n";
    }

    cout<<"------------------------------------------\n";
    protocol::HttpResponse *resp =httptask->get_resp();
    cout<<"http response version = "<<resp->get_http_version()<<"\n";
    cout<<"http response status code = "<<resp->get_status_code()<<"\n";
    cout<<"http response reason phrase = "<<resp->get_reason_phrase()<<"\n";
    protocol::HttpHeaderCursor respCursor(resp);
    while(respCursor.next(key,value))
    {
        cout<<"key = "<<key<<"value = "<<value<<"\n";
    }

    const void *body;
    size_t size;
    resp->get_parsed_body(&body,&size);
    cout<<static_cast<const char *>(body)<<"\n";
}

int main()
{
    signal(SIGINT,signalhandler);
    WFHttpTask *httptask=WFTaskFactory::create_http_task(
                        "http://www.baidu.com",
                        10,10,
                        httpcallback
                        );
    protocol::HttpRequest *req=httptask->get_req();
    req->add_header_pair("key1","value1");
    httptask->start();

    waitGroup.wait();
    cout<<"finish\n";
                                                        
}

