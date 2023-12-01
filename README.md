# TFTP_client
A simple TFTP_client
## 每个函数的作用
- print_time() 在 log 中打印时间
- startUpsocket() 
    1. 初始化 wincock
    2. 创建套接字 socket
    3. 初始化套接字 
    4. 设置套接字的延迟时间为1s
- sockaddr_in get_addr() 根据IP 和 端口 port设置服务器地址
- write_request_pac() 制作==写请求包==
- Make_data_pac() 制作数据包
- read_request_pac() 制作==读请求包==
- MAKE_ACK_PAC() 制作ACK数据包
- WRITE_REQUEST() 一次完整的写入请求
- READ_REQUEST() 实现一次完整的读取请求
- ShowMenu() 展示菜单栏
## 工作流程
1. 
2. 