#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"
class http_conn
{

public:
    /* 文件名称大小 */
    static const int FILENAME_LEN = 200;

    /* 读缓冲区大小 */
    static const int READ_BUFFER_SIZE = 2048;

    /* 写缓冲区大小 */
    static const int WRITE_BUFFER_SIZE = 1024;

    /* HTT请求方法 */
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    /* 主状态机状态 */
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    /* 从状态机状态*/
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

    /* 服务器处理HTTP请求的可能结构*/
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };


public:
    http_conn() {}
    ~http_conn() {}

public:
    /* 初始化新接受的连接，函数内部会调用私有化的方法init*/
    void init(int sockfd, const sockaddr_in &addr);

    /* 关闭HTTP连接*/    
    void close_conn(bool real_close = true);

	/* 处理客户请求*/
    void process();

	/* 读取浏览器发送过来的数据*/
    bool read_once();

	/* 响应报文写入函数*/
    bool write();
    sockaddr_in *get_address()
    {
        return &m_address;
    }

    /* 同步线程初始化数据库读取表*/
    void initmysql_result(connection_pool *connPool);

private:
    //
    void init();


    HTTP_CODE process_read();
    bool process_write(HTTP_CODE ret);
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();
    char *get_line() { return m_read_buf + m_start_line; };
    LINE_STATUS parse_line();
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epollfd;
    static int m_user_count;
    MYSQL *mysql;

private:
    /* 该HTTP连接的socket和对方的地址*/
    int m_sockfd;
    sockaddr_in m_address;
    
    /* 读缓冲区*/
    char m_read_buf[READ_BUFFER_SIZE];

    /* 以下三个是用于从状态机读取每行数据时的标记
    m_read_idx：表示当前已读如客户端数据的下一字节
    m_cheked_idx：表示当前正在分析的读缓冲区的位置
    m_start_line：表示行在缓冲区的起始位置
    */
    int m_read_idx;
    int m_checked_idx;
    int m_start_line;
    
    /* 写缓冲区*/
    char m_write_buf[WRITE_BUFFER_SIZE];
    /* 写缓冲区待发子结束*/
    int m_write_idx;

    CHECK_STATE m_check_state;
    METHOD m_method;
   
    /* 以下6个为解析请求报文中对应的6个变量*/
    
    /* 客户请求的文件名的完整路径，其内容为doc_root + m_url,doc_root是网站根目录*/
    char m_real_file[FILENAME_LEN];
    /* 客户请求的目标文件的文件名*/
    char *m_url;
    /* HTTP协议版本*/
    char *m_version;
    /* 主机名*/
    char *m_host;
    /* 请求的消息体的长度*/
    int m_content_length;
    /* HTTP请求是否要保持长连接*/
    bool m_linger;

    /* 客户请求的目标文件被mmap到内存中的起始位置*/
    char *m_file_address;
    /* 目标文件的状态*/
    struct stat m_file_stat;
    
    /*采用writev来执行写操作，所以定义如下两个成员，m_iv_count表示被写内存块的数量*/
    struct iovec m_iv[2];
    int m_iv_count;
    
    int cgi;        //是否启用的POST
    char *m_string; //存储请求头数据
    int bytes_to_send;
    int bytes_have_send;
};

#endif
