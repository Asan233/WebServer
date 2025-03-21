#include "webserver.h"

WebServer::WebServer()
{
    //http_conn 对象
    users = new http_conn[MAX_FD];

    // root文件路径
    char server_path[200];
    getcwd(server_path, 200);

    char root[6] = "/root";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);

    // 定时器初始化
    users_timer = new client_data[MAX_FD];
}

WebServer::~WebServer()
{
    close(m_epollfd);
    close(m_listenfd);
    close(m_pipefd[0]);
    close(m_pipefd[1]);
    free(m_root);
    delete[] users;
    delete[] users_timer;
    delete m_pool;
}

void WebServer::init(int port, std::string user, std::string passWord, std::string databaseName,
                     int log_write, int opt_linger, int trigmode, int sql_num,
                     int thread_num, int close_log, int actor_model)
{
    m_port = port;
    m_user = user;
    m_passWord = passWord;
    m_databaseName = databaseName;
    m_sql_num = sql_num;
    m_thread_num = thread_num;
    m_log_write = log_write;
    m_OPT_LINGER = opt_linger;
    m_TRIGMode = trigmode;
    m_close_log = close_log;
    m_actormodel = actor_model;    
}

void WebServer::trig_mode()
{
    //LT + LT
    if (0 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigMode = 0;
    }
    //LT + ET
    else if (1 == m_TRIGMode)
    {
        m_LISTENTrigmode = 0;
        m_CONNTrigMode = 1;
    }
    //ET + LT
    else if (2 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigMode = 0;
    }
    //ET + ET
    else if (3 == m_TRIGMode)
    {
        m_LISTENTrigmode = 1;
        m_CONNTrigMode = 1;
    }
}

// 服务器日志初始化
void WebServer::log_write()
{
    if(0 == m_close_log) {
        if(1 == m_log_write)
            Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 800);
        else
            Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 0);
    }
}

// sql连接池初始化
void WebServer::sql_pool()
{
    m_sqlconnectionPool = sqlconnection_pool::GetInstance();
    m_sqlconnectionPool->init("localhost", m_user, m_passWord, m_databaseName, 3306, m_sql_num, m_close_log);

    // 初始化数据库读取表
    users->initmysql_result(m_sqlconnectionPool);
}

// 线程池初始化
void WebServer::thread_pool()
{
    m_pool = new threadpool<http_conn>(m_actormodel, m_sqlconnectionPool, m_thread_num);
}


void WebServer::eventListen()
{
    // 网络编程基础
    m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(m_listenfd >= 0);

    // 优雅关闭连接
    if(0 == m_OPT_LINGER)
    {
        struct linger tmp = {0, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }else if(1 == m_OPT_LINGER)
    {
        struct linger tmp = {1, 1};
        setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    // 设置 SO_REUSEADDR 确保能立即重用地址
    int flag = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(m_listenfd, (sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(m_listenfd, 5);
    assert(ret >= 0);

    // 工具类初始化
    utils.init(TIMESLOT);

    // epoll创建内核事件表
    epoll_event events[MAX_EVENT_NUMBER];
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);

    utils.addfd(m_epollfd, m_listenfd, false, m_LISTENTrigmode);
    http_conn::m_epollfd = m_epollfd;

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret != -1);
    utils.setnonblocking(m_pipefd[1]);
    utils.addfd(m_epollfd, m_pipefd[0], false, 0);

    utils.addsig(SIGPIPE, SIG_IGN);
    utils.addsig(SIGALRM, utils.sig_handler, false);
    utils.addsig(SIGTERM, utils.sig_handler, false);

    // 启动定时信号
    alarm(TIMESLOT);

    // 工具类变量初始化
    Utils::u_epollfd = m_epollfd;
    Utils::u_pipefd = m_pipefd;
}

void WebServer::timer(int connfd, struct sockaddr_in client_address)
{
    // 初始化一个新连接
    users[connfd].init(connfd, client_address, m_root, m_CONNTrigMode, m_close_log, m_user,
                       m_passWord, m_databaseName);
    
    //初始化client_data数据
    // 创建定时器，设置回调函数与超时时间，绑定http_conn 与 定时器
    users_timer[connfd].address = client_address;
    users_timer[connfd].sockfd = connfd;

    tw_timer *timer = new tw_timer(0 , 3);
    timer->data_user = users_timer + connfd;
    timer->cb_func = cb_func;
    //LOG_INFO("address : %d", cb_func);

    users_timer[connfd].timer = timer;
    utils.m_time_wheel.add_timer(timer);
}

// 假如发生数据传输，则将定时器延迟3个单位
// 并对新定时器在链表上位置进行调整
void WebServer::adjust_timer(tw_timer* timer)
{
    utils.m_time_wheel.adjust_timer(timer);
    LOG_INFO("Client(%s) Adjust Timer", inet_ntoa(timer->data_user->address.sin_addr));
}

void WebServer::deal_timer(tw_timer* timer, int sockfd)
{
    timer->cb_func(&users_timer[sockfd]);
    if(timer)
    {
        utils.m_time_wheel.del_timer(timer);
    }
    LOG_INFO("close fd %d", users_timer[sockfd].sockfd);
}


// 处理用户数据
// 当用户发起连接时，处理新到来的用户连接
bool WebServer::dealclientdata()
{
    struct sockaddr_in client_address;
    socklen_t client_addrlen = sizeof(client_address);
    if(0 == m_LISTENTrigmode)
    {   // LT触发模式
        int connfd = accept(m_listenfd, (struct sockaddr *)&client_address, &client_addrlen);
        if(connfd < 0)
        {
            LOG_ERROR("%s error is: %d", "accept error", errno);
            return false;
        }

        if(http_conn::m_user_count >= MAX_FD)
        {
            utils.show_error(connfd, "Internal server busy");
            LOG_ERROR("%s", "Internal server busy");
            return false;
        }
        timer(connfd, client_address);
    } else {
        while(1)
        {
            int connfd = accept(m_listenfd, (struct sockaddr*)&client_address, &client_addrlen);
            if(connfd < 0)
            {
                LOG_ERROR("%s: errno is:%d", "accept error", errno);
                break;
            }

            if(http_conn::m_user_count >= MAX_FD)
            {
                utils.show_error(connfd, "Internal server busy");
                LOG_ERROR("%s", "Internal server busy");
                break;
            }
            timer(connfd, client_address);
        }
        return false;
    }
    LOG_INFO("Accept Client(%s)", inet_ntoa(client_address.sin_addr));
    return true;
}

bool WebServer::dealwithsignal(bool &timeout, bool &stop_server)
{
    int ret = 0;
    int sig;
    char signals[1024];
    ret = recv(m_pipefd[0], signals, sizeof(signals), 0);
    if(0 >= ret)
    {
        return false;
    }else {
        for(int i = 0; i < ret; ++i)
        {
            switch (signals[i])
            {
                case SIGALRM:
                    timeout = true;
                    break;
                
                case SIGTERM:
                    stop_server = true;
                    break;
            default:
                break;
            }
        }
    }
    return true;
}

void WebServer::dealwithread(int sockfd)
{
    tw_timer *timer = users_timer[sockfd].timer;

    //reactor
    if(1 == m_actormodel)
    {
        if(timer)
        {
            adjust_timer(timer);
        }
        // 监测到读事件, 放入请求队列中
        m_pool->append(users + sockfd, 0);

        while(true)
        {
            if(1 == users[sockfd].improv)
            {
                if(1 == users[sockfd].timer_flag)
                {
                    deal_timer(timer, sockfd);
                    users[sockfd].timer_flag = 0;
                }
                users[sockfd].improv = 0;
                break;
            }
        }
    }
    else {
        // proactor
        if(users[sockfd].read_once())
        {
            LOG_INFO("Proactor deal with the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));

            // 若监测到读事件，则放入请求队列
            m_pool->append_p(users + sockfd);

            if(timer)
            {
                adjust_timer(timer);
            }
        }else 
        {
            deal_timer(timer, sockfd);
        }
    }
}

void WebServer::dealwithwrite(int sockfd)
{
    tw_timer *timer = users_timer[sockfd].timer;

    //reactor
    if(1 == m_actormodel)
    {
        if(timer)
        {
            adjust_timer(timer);
        }

        // 将写任务放入请求队列中
        m_pool->append(users + sockfd, 1);

        while(true)
        {
            if(1 == users[sockfd].improv)
            {
                if(1 == users[sockfd].timer_flag)
                {
                    deal_timer(timer, sockfd);
                    users[sockfd].timer_flag = 0;
                }
                users[sockfd].improv = 0;
                break;
            }
        }
    }else {
        // proactor
        if(users[sockfd].write())
        {
            LOG_INFO("send data to the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));

            if(timer)
            {
                adjust_timer(timer);
            }
        }else {
            deal_timer(timer, sockfd);
        }
    }
}

void WebServer::eventLoop()
{
    bool timeout = false;
    bool stop_server = false;

    while (!stop_server)
    {
        int number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR)
        {
            LOG_ERROR("%s", "epoll failure");
            break;
        }

        for (int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;

            //处理新到的客户连接
            if (sockfd == m_listenfd)
            {
                bool flag = dealclientdata();
                if (false == flag)
                    continue;
            }
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                //服务器端关闭连接，移除对应的定时器
                tw_timer *timer = users_timer[sockfd].timer;
                deal_timer(timer, sockfd);
            }
            //处理信号
            else if ((sockfd == m_pipefd[0]) && (events[i].events & EPOLLIN))
            {
                bool flag = dealwithsignal(timeout, stop_server);
                if (false == flag)
                    LOG_ERROR("%s", "dealclientdata failure");
            }
            //处理客户连接上接收到的数据
            else if (events[i].events & EPOLLIN)
            {
                dealwithread(sockfd);
            }
            else if (events[i].events & EPOLLOUT)
            {
                dealwithwrite(sockfd);
            }
        }                                                             
        if (timeout)
        {
            utils.timer_handler();
            //LOG_INFO("%s", "timer tick");
            timeout = false;
        }
    }
}


