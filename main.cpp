#include<winsock2.h>
#include<string.h>
#include<stdio.h>
#include<iostream>
#include<ctime>

using namespace std;

#define DATA_PAC_MAX 515
#define Error_code -2
#define MAX_BUFFER_LEN 1024

void print_time(FILE *fp)
{
    time_t t;
    time(&t);
    char stime[100];
    strcpy(stime,ctime(&t));
    *(strchr(stime,'\n')) = '\0';
    fprintf(fp,"[ %s ]",stime);

    return ;
}

SOCKET startUpsocket()
{
    /*初始化 wincock*/
    WSAData wasdata;
    int nRc = WSAStartup(0x0202,&wasdata);

    if(nRc)
    {
        // printf("Winsock 初始化错误\n");
        return Error_code;
    }
    if(wasdata.wVersion != 0x0202)
    {
        // printf("版本支持不够\n");
        return Error_code;
    }



    /* 创建socket*/
    SOCKET server_socket_fd = socket(AF_INET,SOCK_DGRAM,0);
    if(server_socket_fd == -1)
    {
        perror("Create Socket Failed !\n");
        return Error_code;
    }
    int sendTimeout = 1000; // 1秒
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&sendTimeout, sizeof(sendTimeout)) == SOCKET_ERROR) {
        printf("Failed to set sendto timeout.\n");
        closesocket(server_socket_fd);
        WSACleanup();
        return Error_code;
    }
    int recvTimeout = 1000; // 1秒
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&recvTimeout, sizeof(recvTimeout)) == SOCKET_ERROR) {
        printf("Failed to set recvfrom timeout.\n");
        closesocket(server_socket_fd);
        WSACleanup();
        return Error_code;
    }
    return server_socket_fd;
}

struct sockaddr_in get_addr(const char *ip,int port)
{
    /* 服务器地址 */
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.S_un.S_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    return server_addr;
}

// char *write_request_pac1(char * filename,int mode,int &datalen)
// {
//     datalen = 4 + strlen(filename) + mode;
//     char *buffer = new char[datalen];           //2个字节表示操作码 + 文件名字长度 + 模式名字长度 + 两个结束符'\0'
//     // memset(buffer,0,datalen);
//     buffer[0] = 0x00;
//     buffer[1] = 0x02;                                               //02表示为写入请求包
//     strcpy(buffer+2,filename);
//     if(mode == 8)
//         strcpy(buffer+3+strlen(filename),"netascii\0");
//     if(mode == 5)
//         strcpy(buffer+3+strlen(filename),"octet\0");

//     return buffer;
// }
char *write_request_pac(const char *filename, int mode, int &datalen)
{
    int len = strlen(filename);
    datalen = len + mode + 2 + 2; // 文件名长度 + null 终止符 + 操作码 + 模式 + null 终止符
    char *buffer = new char[datalen];

    buffer[0] = 0x00;
    buffer[1] = 0x02; // 02 表示写入请求包
    memcpy(buffer + 2, filename, len);
    buffer[2 + len] = '\0'; // 添加文件名的 null 终止符

    if (mode == 8)
    {
        memcpy(buffer + 3 + len, "netascii\0", 9);
    }
    else if (mode == 5)
    {
        memcpy(buffer + 3 + len, "octet\0", 6);
    }

    return buffer;
}

int Make_data_pac(FILE *f,char *pac,int &datalen,int block)                                                                               //生成数据包
{
    char buffer[DATA_PAC_MAX];
    datalen = fread(buffer,1,512,f);
    if(datalen == -1)
    {
        printf("File read error!\n");
        return -1;                                                                                                              // 返回错误标志以指示文件读取失败
    }
    char high = (block>>8) & (0x00ff);
    char low = block & (0x00ff);

    pac[0] = 0x00;
    pac[1] = 0x03;
    pac[2] = high;
    pac[3] = low;

    memcpy(pac + 4,buffer,datalen);
    pac[4+datalen] = '\0';

    return datalen;

}
char *read_request_pac(const char *filename, int mode, int &datalen)
{
    int len = strlen(filename);
    datalen = len + mode + 2 + 2; // 文件名长度 + null 终止符 + 操作码 + 模式 + null 终止符
    char *buffer = new char[datalen];

    buffer[0] = 0x00;
    buffer[1] = 0x01; // 02 表示写入请求包
    memcpy(buffer + 2, filename, len);
    buffer[2 + len] = '\0'; // 添加文件名的 null 终止符

    if (mode == 8)
    {
        memcpy(buffer + 3 + len, "netascii\0", 9);
    }
    else if (mode == 5)
    {
        memcpy(buffer + 3 + len, "octet\0", 6);
    }

    return buffer;
}
int MAKE_ACK_PAC(char * buffer,int block,int &datalen)
{

    buffer[0] = 0x00;
    buffer[1] = 0x04; //04 表示ack包

    buffer[2] = (block>>8) & (0x00ff);
    buffer[3] = block & (0x00ff);
    datalen = 4;
    return 0;
}


void WRITE_REQUEST(FILE * fp)
{
    char filename[1000];
    int mode;

    char recv_buffer[MAX_BUFFER_LEN];
    SOCKET sock = startUpsocket();            //初始化和生成一个套接字
    struct sockaddr_in server_addr = get_addr("127.0.0.1",69);                                      //这是第一次请求包的地址信息,之后更新为服务端的地址信息

    printf("Please enter the complete file name:");
    scanf("%s",filename);
    while(1)
    {
        printf("Please select the file format:\n1. String file 2. Binary file\n");
        scanf("%d",&mode);
        if(mode == 1)
        {
            mode = 8;
            break;
        }
        else if(mode == 2)
        {
            mode = 5;
            break;
        }
        else
        {
            printf("Please select the file format:\n1. String file 2. Binary file\n");
            // printf("请输入正确的序号！\n");
            // printf("请选择文件格式：\n1.字符串文件  2.二进制文件");
        }
    }
    char buf[DATA_PAC_MAX],datapck[DATA_PAC_MAX];                                                                   //datapck为数据报，buf中存储缓存
    int block = 0;                                                                                                  //记录包模块
    int datalen; //记录数据包大小
    int send_error_times = 1;//发送错误的次数
    char* first_pac = write_request_pac(filename,mode,datalen);
    memcpy(buf,first_pac,datalen);
    int ret_send = sendto(sock,first_pac,datalen,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //需要套接字和服务器地址信息，如果正确发送，返回发送的字节数
    // 记录开始时间
    clock_t start = clock();




    while(ret_send != datalen && send_error_times < 10)
    {
        printf("Send Error %d times!\n",send_error_times);
        send_error_times++;
        ret_send = sendto(sock,first_pac,datalen,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //重传请求包
    }

    FILE *f = NULL;
    if(mode == 8)
        f = fopen(filename,"r");                                                                                  //f为打开该文件指针
    else if(mode == 5)
        f = fopen(filename,"rb");                                                                                  //以二进制方式打开文件
    if(f == NULL) printf("Open file filed!\n");

    print_time(fp);
    fprintf(fp,"WRQ for file %s\n",filename);

    datalen = 512;
    int dataUploaded = 0;
    int loop_times = 0;
    while(1)                                                                                                       //发送数据包过程
    {
        int recv_pack_len = sizeof(server_addr);
        int ret_rec = recvfrom(sock,recv_buffer,MAX_BUFFER_LEN,0,(sockaddr*)&server_addr,&recv_pack_len);

        if(ret_rec > 0)                                                                                             //正确收到应答
        {
            if(recv_buffer[1]==5)                                                                                               //返回错误代码
            {
                int Errorcode = recv_buffer[2]<<8 | recv_buffer[3];
                printf("%d Error!\n",Errorcode);
                print_time(fp);
                fprintf(fp,"%d Error!\n",Errorcode);
                break;
            }

            if(recv_buffer[1] == 4)                                                                                             //返回正确ack代码
            {
                int recv_block = recv_buffer[2]<<8 | recv_buffer[3];
                if(recv_block == block)                                                                                         //返回正确的block number，则继续打包下一个包/或者关闭连接
                {
                    if(datalen < 512)                                                                                           //已经发送完最后一个包，关闭连接
                    {
                        clock_t end = clock();

                        // 计算执行时间（以秒为单位）
                        double elapsedTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
                        double uploadRate = static_cast<double>(dataUploaded) / elapsedTime; // 字节/秒


                        // 输出上传速率
                        std::cout << "Upload Rate: " << uploadRate << " bytes/second" << std::endl;
                        printf("File %s upload is complete\n",filename);
                        print_time(fp);
                        fprintf(fp,"File %s upload is complete\n",filename);
                        break;
                    }
                    else                                                                                                        //还有数据需要传输，继续打包下一个包
                    {

                        block++;
                        Make_data_pac(f,datapck,datalen,block);
                        memcpy(buf,datapck,datalen + 4);
                        dataUploaded += datalen;

                        int send_error_times = 1;
                        
                        int ret_send = sendto(sock,datapck,datalen+4,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //需要套接字和服务器地址信息，如果正确发送，返回发送的字节数

                        while(ret_send != datalen + 4 && send_error_times <= 10)
                        {
                            printf("Send %d package ,Error %d times!\n",block,send_error_times);
                            print_time(fp);
                            fprintf(fp,"Send %d package ,Error %d times!\n",block,send_error_times);
                            send_error_times++;
                            int ret_send = sendto(sock,buf,datalen+4,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //重传请求包
                        }
                        if(send_error_times > 10)
                        {
                            printf("Send %d package faild\n",block);
                            print_time(fp);
                            fprintf(fp,"Send %d package faild\n",block);
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
                else                                                                                                            //否则重传原来的包
                {
                    int send_error_times = 1;
                    
                    int ret_send = sendto(sock,buf,datalen + 4,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //需要套接字和服务器地址信息，如果正确发送，返回发送的字节数
                        printf("reSend %d package!\n",block,send_error_times);
                        print_time(fp);
                        fprintf(fp,"reSend %d package!\n",block,send_error_times);
                    while(ret_send != datalen + 4 && send_error_times <= 10)
                    {
                        printf("Send %d package ,Error %d times!\n",block,send_error_times);
                        print_time(fp);
                        fprintf(fp,"Send %d package ,Error %d times!\n",block,send_error_times);
                        send_error_times++;
                        int ret_send = sendto(sock,buf,datalen,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //重传请求包
                    }
                    if(send_error_times > 10)
                    {
                        printf("Send %d package faild\n",block);
                        print_time(fp);
                        fprintf(fp,"Send %d package faild\n",block);
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
        }
        else if(ret_rec == 0)                                                                                        //链接已经关闭
        {
            printf("Connect has closed!\n");
            print_time(fp);
            fprintf(fp,"Connect has closed!\n");
            
            break;
        }
        else                                                                                                        //超时或者产生丢包
        {
            loop_times++;
            if(loop_times > 20)
            {
                cout<<ret_rec<<endl;
                printf("The server is not responding\n");
                print_time(fp);
                fprintf(fp,"The server is not responding\n");
                break;

            }
        // printf("in loop!block:%d,datalen:%d\n",block,datalen);
            int send_error_times = 1;
            
            int ret_send = sendto(sock,buf,datalen + 4,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //需要套接字和服务器地址信息，如果正确发送，返回发送的字节数

            while(ret_send != datalen + 4 && send_error_times <= 10)
            {
                printf("Send %d package ,Error %d times!\n",block,send_error_times);
                print_time(fp);
                fprintf(fp,"Send %d package ,Error %d times!\n",block,send_error_times);
                send_error_times++;
                ret_send = sendto(sock,buf,datalen+4,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //重传请求包
            }
            if(send_error_times > 10)
            {
                printf("Send %d package faild\n",block);
                print_time(fp);
                fprintf(fp,"Send %d package faild\n",block);
                break;
            }
            else
            {
                continue;
            }
        }
    }

    fclose(f);
    closesocket(sock);
    WSACleanup();
}

void READ_REQUEST(FILE * fp)
{
    char filename[1000];
    int mode;

    char recv_buffer[MAX_BUFFER_LEN];
    SOCKET sock = startUpsocket();            //初始化和生成一个套接字
    struct sockaddr_in server_addr = get_addr("127.0.0.1",69);                                      //这是第一次请求包的地址信息,之后更新为服务端的地址信息

    printf("Please enter the complete file name:");
    scanf("%s",filename);
    while(1)
    {
        printf("Please select the file format:\n1. String file 2. Binary file\n");
        scanf("%d",&mode);
        if(mode == 1)
        {
            mode = 8;
            break;
        }
        else if(mode == 2)
        {
            mode = 5;
            break;
        }
        else
        {
            printf("Please select the file format:\n1. String file 2. Binary file\n");
            // printf("请输入正确的序号！\n");
            // printf("请选择文件格式：\n1.字符串文件  2.二进制文件");
        }
    }
    char buf[MAX_BUFFER_LEN],ack[4];                                                                   //ack为回复包，buf中存储缓存
    int block = 1;                                                                                                  //记录包模块
    int datalen; //记录数据包大小
    int send_error_times = 1;//发送错误的次数
    char* first_pac = read_request_pac(filename,mode,datalen);
    memcpy(buf,first_pac,datalen);
    int ret_send = sendto(sock,first_pac,datalen,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //需要套接字和服务器地址信息，如果正确发送，返回发送的字节数
    // 记录开始时间
    clock_t start = clock();




    while(ret_send != datalen && send_error_times < 10)
    {
        printf("Send RRQ Error %d times!\n",send_error_times);
        print_time(fp);
        fprintf(fp,"Send RRQ Error %d times!\n",send_error_times);

        send_error_times++;
        int ret_send = sendto(sock,first_pac,datalen,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //重传请求包
    }

    FILE *f = NULL;
    if(mode == 8)
        f = fopen(filename,"w");                                                                                  //f为打开该文件指针
    else if(mode == 5)
        f = fopen(filename,"wb");                                                                                  //以二进制方式打开文件
    if(f == NULL) printf("Open file filed!\n");

    datalen = 512;
    int dataUploaded = 0;

    print_time(fp);
    fprintf(fp,"RRQ for file %s\n",filename);
    while(1)                                                                                                       //发送数据包过程
    {
        int recv_pack_len = sizeof(server_addr);
        int ret_rec = recvfrom(sock,recv_buffer,MAX_BUFFER_LEN,0,(sockaddr*)&server_addr,&recv_pack_len);

        if(ret_rec > 0)                                                                                             //正确收到应答
        {
            if(recv_buffer[1]==5)                                                                                               //返回错误代码
            {
                int Errorcode = recv_buffer[2]<<8 | recv_buffer[3];
                printf("%d Error!\n",Errorcode);
                print_time(fp);
                fprintf(fp,"%d Error!\n",Errorcode);
                break;
            }

            if(recv_buffer[1] == 3)                                                                                             //返回正确的数据包
            {
                int recv_block = recv_buffer[2]<<8 | recv_buffer[3];
                if(recv_block == block)                                                                                         //返回正确的block number，则继续打包下一个包/或者关闭连接
                {
                        //把数据写入文件中
                    fwrite(recv_buffer+4,1,ret_rec-4,f);
                    MAKE_ACK_PAC(ack,block,datalen);
                    memcpy(buf,ack,datalen );
                    dataUploaded += ret_rec - 4;                                                                      //计算总数据量

                    int send_error_times = 1;
                    
                    int ret_send = sendto(sock,ack,datalen,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //需要套接字和服务器地址信息，如果正确发送，返回发送的字节数

                    while(ret_send != datalen  && send_error_times <= 10)
                    {
                        printf("Send %d package ,Error %d times!\n",block,send_error_times);
                        print_time(fp);
                        fprintf(fp,"Send %d package ,Error %d times!\n",block,send_error_times);

                        send_error_times++;
                        int ret_send = sendto(sock,buf,datalen,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //重传请求包
                    }
                    if(send_error_times > 10)
                    {
                        printf("Send %d package faild\n",block);
                        print_time(fp);
                        fprintf(fp,"Send %d package faild\n",block);

                        break;
                    }
                    if(ret_rec - 4 < 512)                                                                                           //已经发送完最后一个包，关闭连接
                    {
                        clock_t end = clock();

                        // 计算执行时间（以秒为单位）
                        double elapsedTime = static_cast<double>(end - start) / CLOCKS_PER_SEC;
                        double downloadRate = static_cast<double>(dataUploaded) / elapsedTime; // 字节/秒


                        // 输出上传速率
                        if(downloadRate == INFINITE)
                        {
                            cout<<"It`s too fast"<<endl;

                        }
                        else
                        {
                            std::cout << "Download Rate: " << downloadRate << " bytes/second" << std::endl;
                        }
                        printf("File %s download is complete\n",filename);
                        print_time(fp);
                        fprintf(fp,"File %s download is complete\n",filename);
                        break;
                    }
                    else                                                                                                        //还有数据需要传输，继续打包下一个包
                    {
                        
                        block++;

                        continue;
                    }
                }
                else                                                                                                            //否则重传原来的包
                {
                    int send_error_times = 1;
                    
                    int ret_send = sendto(sock,buf,datalen,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //需要套接字和服务器地址信息，如果正确发送，返回发送的字节数

                    while(ret_send != datalen && send_error_times <= 10)
                    {
                        printf("Send %d package ,Error %d times!\n",block,send_error_times);
                        print_time(fp);
                        fprintf(fp,"Send %d package ,Error %d times!\n",block,send_error_times);
                        send_error_times++;
                        int ret_send = sendto(sock,buf,datalen,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //重传ack
                    }
                    if(send_error_times > 10)
                    {
                        printf("Send %d ack faild\n",block);
                        print_time(fp);
                        fprintf(fp,"Send %d ack faild\n",block);

                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
        }
        else if(ret_rec == 0)                                                                                        //链接已经关闭
        {
            printf("Connect has closed!\n");
            print_time(fp);
            fprintf(fp,"Connect has closed!\n");
            break;
        }
        else                                                                                                        //超时或者产生丢包
        {
            int send_error_times = 1;
            
            int ret_send = sendto(sock,buf,datalen,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //需要套接字和服务器地址信息，如果正确发送，返回发送的字节数
            printf("reSend %d package!\n",block,send_error_times);
            print_time(fp);
            fprintf(fp,"reSend %d package!\n",block,send_error_times);


            while(ret_send != datalen && send_error_times <= 10)
            {
                printf("Send %d package ,Error %d times!\n",block,send_error_times);
                print_time(fp);
                fprintf(fp,"Send %d package ,Error %d times!\n",block,send_error_times);
                send_error_times++;
                int ret_send = sendto(sock,buf,datalen,0,(const sockaddr*)&server_addr,sizeof(server_addr));               //重传请求包
            }
            if(send_error_times > 10)
            {
                printf("Send %d package faild\n",block);
                print_time(fp);
                fprintf(fp,"Send %d package faild\n",block);
                break;
            }
            else
            {
                continue;
            }
        }
    }

    fclose(f);
    closesocket(sock);
    WSACleanup();
}
void ShowMenu() {
cout<<"  __  __"<<endl;
cout<<" / / / /___  __"<<endl;
cout<<"/ /_/ / __ \\/ / "<<endl;
cout<<"\\__, / /_/ / / "<<endl; 
cout<<"  /_/ \\____/_/ "<<endl;


    cout << "TFTP Client Menu:" << endl;
    cout << "1. Upload File" << endl;
    cout << "2. Download File" << endl;
    cout << "3. Exit" << endl;
    cout << " (1/2/3): ";
}

int main() {
    FILE* fp = fopen("TFTP_client.log", "a");

    while (true) {
        ShowMenu();

        int choice;
        cin >> choice;

        if (choice == 1) {
            // 上传文件
            WRITE_REQUEST(fp);
        } else if (choice == 2) {
            // 下载文件
            READ_REQUEST(fp);
        } else if (choice == 3) {
            // 退出程序
            break;
        } else {
            cout << "choose again" << endl;
        }
    }

    fclose(fp);
    return 0;
}

// int main()
// {
//     FILE * fp = fopen("TFTP_client.log","a");

//     //WRITE_REQUEST(fp);
//     //READ_REQUEST(fp);
    

//     fclose(fp);
//     return 0;
// }