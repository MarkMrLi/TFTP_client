# TFTP_client
A simple TFTP_client
## 系统结构设计
### 每个函数的作用
- print_time() 在 log 中打印时间
- startUpsocket() 
    1. 初始化 wincock
    2. 创建套接字 socket
    3. 初始化套接字 
    4. 设置套接字的延迟时间为1s
- sockaddr_in get_addr() 根据IP 和 端口 port设置服务器地址
- write_request_pac() 制作**写请求包**
- Make_data_pac() 制作数据包
- read_request_pac() 制作**读请求包**
- MAKE_ACK_PAC() 制作ACK数据包
- WRITE_REQUEST() 一次完整的写入请求
- READ_REQUEST() 实现一次完整的读取请求
- ShowMenu() 展示菜单栏
- TODOTEST

以下是上述代码中模块之间的接口说明和模块划分的简化描述：

1. **`print_time` 模块：**
   - **接口：** `void print_time(FILE *fp)`
   - **功能：** 在日志中打印当前时间。

2. **`startUpsocket` 模块：**
   - **接口：** `SOCKET startUpsocket()`
   - **功能：** 初始化 Winsock，创建套接字，设置套接字的超时时间。

3. **`get_addr` 模块：**
   - **接口：** `struct sockaddr_in get_addr(const char *ip, int port)`
   - **功能：** 根据 IP 和端口号设置服务器地址。

4. **`write_request_pac` 模块：**
   - **接口：** `char *write_request_pac(const char *filename, int mode, int &datalen)`
   - **功能：** 生成写请求包。

5. **`Make_data_pac` 模块：**
   - **接口：** `int Make_data_pac(FILE *f, char *pac, int &datalen, int block)`
   - **功能：** 生成数据包。

6. **`read_request_pac` 模块：**
   - **接口：** `char *read_request_pac(const char *filename, int mode, int &datalen)`
   - **功能：** 生成读请求包。

7. **`MAKE_ACK_PAC` 模块：**
   - **接口：** `int MAKE_ACK_PAC(char *buffer, int block, int &datalen)`
   - **功能：** 生成 ACK 包。

8. **`WRITE_REQUEST` 模块：**
   - **接口：** `void WRITE_REQUEST(FILE *fp)`
   - **功能：** 实现一次完整的写入请求，包括数据包的生成和发送，错误处理，日志记录，吞吐量计算。

9. **`READ_REQUEST` 模块：**
   - **接口：** `void READ_REQUEST(FILE *fp)`
   - **功能：** 实现一次完整的读取请求，包括数据包的生成和发送，错误处理，日志记录，吞吐量计算。

10. **`ShowMenu` 模块：**
    - **接口：** `void ShowMenu()`
    - **功能：** 显示用户菜单，获取用户选择。

11. **`main` 模块：**
    - **接口：** `int main()`
    - **功能：** 主程序，通过用户选择调用上传或下载功能。

**模块之间的调用关系：**
- `WRITE_REQUEST` 和 `READ_REQUEST` 模块中调用了 `startUpsocket` 模块初始化套接字。
- `WRITE_REQUEST` 和 `READ_REQUEST` 模块中调用了 `get_addr` 模块设置服务器地址。
- `WRITE_REQUEST` 模块中调用了 `write_request_pac` 模块生成写请求包。
- `WRITE_REQUEST` 模块中调用了 `Make_data_pac` 模块生成数据包。
- `READ_REQUEST` 模块中调用了 `read_request_pac` 模块生成读请求包。
- `WRITE_REQUEST` 和 `READ_REQUEST` 模块中调用了 `MAKE_ACK_PAC` 模块生成 ACK 包。
- `WRITE_REQUEST` 和 `READ_REQUEST` 模块中调用了 `print_time` 模块打印时间。
- `main` 模块中调用了 `WRITE_REQUEST` 和 `READ_REQUEST` 模块实现文件上传和下载。

这些模块之间的调用关系构成了整个程序的执行流程。每个模块负责特定的功能，通过调用其他模块来完成整体的文件传输、错误处理和日志记录等任务。
### 工作流程
数据处理流程描述了程序中数据是如何被处理、传递和转换的。以下是该程序的简化数据处理流程：

1. **用户交互：**
   - 用户通过主菜单选择上传文件或下载文件的操作。

2. **写入请求（上传文件）流程：**
   - 用户选择上传文件后，调用 `WRITE_REQUEST` 函数。
   - `WRITE_REQUEST` 函数首先初始化套接字、生成写请求数据包，并向服务器发送请求。
   - 服务器收到请求后，开始等待数据包。
   - `WRITE_REQUEST` 函数读取文件数据，生成数据包，并将数据包发送给服务器。
   - 服务器收到数据包后，确认并等待下一个数据包。
   - 上述步骤循环，直到文件传输完成。

3. **读取请求（下载文件）流程：**
   - 用户选择下载文件后，调用 `READ_REQUEST` 函数。
   - `READ_REQUEST` 函数首先初始化套接字、生成读请求数据包，并向服务器发送请求。
   - 服务器收到请求后，开始准备文件数据。
   - `READ_REQUEST` 函数等待接收数据包，读取数据包，生成 ACK 包，并将 ACK 包发送给服务器。
   - 服务器收到 ACK 包后，继续发送下一个数据包。
   - 上述步骤循环，直到文件传输完成。

4. **数据包处理：**
   - 数据包包括写请求包、读请求包、数据包和 ACK 包。
   - 写请求包包含文件名和传输模式，告诉服务器准备接收数据。
   - 读请求包包含文件名和传输模式，告诉服务器准备发送数据。
   - 数据包包含文件数据和块编号，服务器接收后写入文件。
   - ACK 包包含确认块编号，告知服务器已成功接收数据包。

5. **日志记录：**
   - 在程序运行过程中，通过调用 `print_time` 函数在日志文件中记录操作时间和事件。
   - 记录写请求（上传文件）和读取请求（下载文件）的开始和完成时间。

6. **错误处理：**
   - 在数据传输过程中，处理可能出现的错误情况，如数据包丢失、超时等。
   - 如果服务器返回错误码，程序将在日志中记录错误信息。

7. **吞吐量计算：**
   - 在文件传输完成后，计算文件上传或下载的吞吐量（文件传输速率）。
   - 吞吐量通过记录开始和结束时间、以及传输的总字节数来计算。

这是一个简化的描述，实际上，还有一些细节可能未在代码中展示，例如超时重传机制、网络异常处理等。此外，上述描述假设服务器和客户端之间的通信是可靠的，而实际上，UDP 协议可能导致一些不可靠的情况，需要通过程序中的错误处理来应对。
### 模块框图

## 详细设计
### 核心代码的流程
核心函数主要是 `WRITE_REQUEST` 和 `READ_REQUEST`，它们负责实现文件的上传和下载过程。以下是这两个核心函数的详细流程：

#### `WRITE_REQUEST` 函数流程：

1. **初始化套接字和服务器地址：**
   - 调用 `startUpsocket` 初始化 Winsock，并创建套接字。
   - 调用 `get_addr` 设置服务器地址。

2. **生成写请求包：**
   - 调用 `write_request_pac` 生成写请求包。

3. **发送写请求包：**
   - 使用 `sendto` 函数发送写请求包到服务器。
   - 进行错误处理，最多重试 10 次。

4. **打开文件：**
   - 根据用户选择的文件格式（字符串文件或二进制文件），打开相应的文件。

5. **发送数据包：**
   - 循环生成数据包，使用 `Make_data_pac` 函数。
   - 发送数据包到服务器，进行错误处理，最多重试 10 次。

6. **接收 ACK 包：**
   - 使用 `recvfrom` 函数接收服务器的 ACK 包。
   - 处理接收到的 ACK 包，如果收到的 ACK 包的 block number 与期望的一致，则继续发送下一个数据包。

7. **计算和打印吞吐量：**
   - 计算文件上传的吞吐量（上传速率）。
   - 打印吞吐量信息到控制台。

8. **关闭文件和套接字：**
   - 关闭打开的文件。
   - 关闭套接字。

#### `READ_REQUEST` 函数流程：

1. **初始化套接字和服务器地址：**
   - 调用 `startUpsocket` 初始化 Winsock，并创建套接字。
   - 调用 `get_addr` 设置服务器地址。

2. **生成读请求包：**
   - 调用 `read_request_pac` 生成读请求包。

3. **发送读请求包：**
   - 使用 `sendto` 函数发送读请求包到服务器。
   - 进行错误处理，最多重试 10 次。

4. **创建文件：**
   - 根据用户选择的文件格式（字符串文件或二进制文件），创建相应的文件。

5. **接收数据包：**
   - 使用 `recvfrom` 函数接收服务器的数据包。
   - 处理接收到的数据包，写入文件。

6. **生成并发送 ACK 包：**
   - 生成 ACK 包，使用 `MAKE_ACK_PAC` 函数。
   - 发送 ACK 包到服务器，进行错误处理，最多重试 10 次。

7. **计算和打印吞吐量：**
   - 计算文件下载的吞吐量（下载速率）。
   - 打印吞吐量信息到控制台。

8. **关闭文件和套接字：**
   - 关闭创建的文件。
   - 关闭套接字。

这两个核心函数的流程主要包括了初始化、生成请求包、数据传输、错误处理、吞吐量计算以及资源关闭等步骤。其中，错误处理部分主要通过重传机制来保证数据的可靠传输。吞吐量的计算涉及到时间的记录和数据量的累积。
### 核心数据结构和关键变量
在提供的代码中，主要涉及到一些数据结构和关键变量。以下是其中的核心数据结构和变量：

1. **文件名和模式：**
   - `char filename[1000]`：存储用户输入的文件名。
   - `int mode`：存储用户选择的文件格式模式，用于区分字符串文件和二进制文件。

2. **套接字和地址结构：**
   - `SOCKET sock`：套接字，用于进行网络通信。
   - `struct sockaddr_in server_addr`：服务器地址结构。

3. **发送和接收缓冲区：**
   - `char buf[MAX_BUFFER_LEN]`：用于存储发送和接收数据的缓冲区。
   - `char recv_buffer[MAX_BUFFER_LEN]`：用于接收数据包的缓冲区。

4. **数据包和数据长度：**
   - `char* first_pac`：存储第一个请求包（写请求或读请求）的数据。
   - `char datapck[DATA_PAC_MAX]`：存储数据包的数据。
   - `int datalen`：记录数据包的长度。

5. **文件指针和文件操作：**
   - `FILE* f`：文件指针，用于打开、读取或写入文件。

6. **ACK 包和块编号：**
   - `char ack[4]`：存储 ACK 包的数据。
   - `int block`：记录当前数据块的编号。

7. **时间记录：**
   - `clock_t start`：记录开始时间，用于计算执行时间和吞吐量。

8. **发送错误次数：**
   - `int send_error_times`：记录发送数据包或 ACK 包时出现错误的次数。

9. **其他常量和配置：**
   - `#define DATA_PAC_MAX 515`：定义数据包的最大长度。
   - `#define Error_code -2`：定义错误码。
   - `#define MAX_BUFFER_LEN 1024`：定义发送和接收缓冲区的最大长度。

这些数据结构和变量主要用于存储用户输入、进行网络通信、处理文件操作、记录时间、处理错误等方面。在整个程序中，这些数据结构和变量相互配合，完成了文件传输的功能。
## 核心代码实现
### 超时重传功能实现
在给出的代码中，超时重传的逻辑主要是通过设置计时器、记录开始时间和判断经过的时间来实现的。以下是相关的代码片段：

#### 超时重传的实现 - `WRITE_REQUEST` 函数

```c
// 记录开始时间
clock_t start = clock();

while (ret_send != datalen && send_error_times < 10)
{
    printf("Send Error %d times!\n", send_error_times);
    send_error_times++;
    ret_send = sendto(sock, first_pac, datalen, 0, (const sockaddr*)&server_addr, sizeof(server_addr));

    // 超时重传判断
    if ((clock() - start) > TIMEOUT)
    {
        // 超时，重新发送
        printf("Timeout! Resending data...\n");
        send_error_times++;
        start = clock();
    }
}
```

在上述代码中，通过 `clock()` 函数获取当前时钟时间，计算发送时间是否超过了预设的超时时间 `TIMEOUT`。如果超时，就会重新发送数据包。

#### 超时重传的实现 - `READ_REQUEST` 函数

```c
// 记录开始时间
clock_t start = clock();

while (ret_send != datalen && send_error_times < 10)
{
    printf("Send RRQ Error %d times!\n", send_error_times);
    print_time(fp);
    fprintf(fp, "Send RRQ Error %d times!\n", send_error_times);

    send_error_times++;
    ret_send = sendto(sock, first_pac, datalen, 0, (const sockaddr*)&server_addr, sizeof(server_addr));

    // 超时重传判断
    if ((clock() - start) > TIMEOUT)
    {
        // 超时，重新发送
        printf("Timeout! Resending RRQ...\n");
        print_time(fp);
        fprintf(fp, "Timeout! Resending RRQ...\n");
        send_error_times++;
        start = clock();
    }
}
```

同样，在读请求的代码中也是通过判断经过的时间是否超过了预设的超时时间 `TIMEOUT` 来实现的。

需要注意的是，超时重传的实现可能还涉及到一些其他细节，比如在实际网络环境中可能需要考虑网络延迟的不确定性、重传次数的控制等。上述代码只是一个简单的示例，实际应用中可能需要根据具体情况进行调整。
