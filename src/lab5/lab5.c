#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>


#define BUFSIZE 1024

volatile int stop = 0;

/**
 * @brief 處理殭屍process用
 * 
 * @param signum 
 */
void handler(int signum)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}



int main(int argc, char *argv[]){
    signal(SIGCHLD, handler);
    signal(SIGINT, intHandler);

    if (argc != 2){
        printf("Please usage: %s <port>\n", argv[0]);
        exit(-1);
    }

    int connfd, sockfd;
    struct sockaddr_in cln_addr, info;
    socklen_t sLen = sizeof(cln_addr);
    memset(&info, 0, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_addr.s_addr = INADDR_ANY;

    /**
     * @brief     設定socket的資訊(傳輸協定、傳輸方式)
     * 
     * @param     PF_INET 表示建立IPv4的通訊
     * @param     SOCK_STREAM 表示傳輸的通訊方式是TCP
     * 
     */
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        perror("Error create socket\n");
        exit(-1);
    }

    int connection_port = (unsigned short)atoi(argv[1]);
    /**
     * @brief htons將port轉為socker用的字符
     * 
     * @param connection_port 我們連接要使用的port
     * 
     * @retval 將port轉為儲存在sin_port裡面的字符
     * 
     */
    info.sin_port = htons(connection_port);


    int reusable = 1;
    /**
     * @brief Construct a new setsockopt object
     * 
     * @param SOL_SOCKET 表示要接觸網路的哪一層, SOL_SOCKET表示要存取的是SOCKET那一層
     * @param SO_REUSEADDR 表示我們要設定這個地址是否可以重複地被使用
     * @param reusable 表示我們關於上述設定的設定值
     * @param sizeofXXX 表示上述設定值的長度
     * 
     */
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reusable, sizeof(reusable));


    /**
     * @brief 定位socket 目前還沒看懂OUO
     * @param sockfd 要被定位的socket，要先定位完才可以進到listen
     * 
     */
    bind(sockfd, (struct sockaddr *)&info, sizeof(info));

    /**
     * @brief 將socket設定為可以準備連線的狀態(真正進入等待連線狀態的是accept()函數)
     * 
     * @param sockfd 要設定的socket
     * @param 10 此socket最大可以接受的同時連線數量
     * 
     */
    listen(sockfd, 10);

    pid_t child_pid;


    while(!stop){

        /**
         * @brief 接受client的連線
         * 
         * @param sockfd 用來處理連線的socker，在accept之前一定需要先經過bind還有listen
         * 
         */
        connfd = accept(sockfd, (struct sockaddr *)&cln_addr, &sLen);
        if (connfd == -1)
        {
            perror("Error: accept()");
            continue;
        }

        /**
         * @brief 就是fork 沒什麼好說的ㄅ
         * 
         */
        child_pid = fork();
        if (child_pid >= 0)
        {
            /**
             * @brief 紫禁城會進到這
             * 
             */
            if (child_pid == 0){

                /**
                 * @brief dup2(oldFd,newFd) 將舊的file  descriptor的資訊複製到新的file descriptor上(兩者共享)
                 * 
                 * @param connfd 表示當前使用的connfd
                 * @param STDOUT_FILENO 表示std::cout的file descriptor
                 * 
                 */
                dup2(connfd, STDOUT_FILENO);
                /**
                 * @brief 因為在紫禁城裡面已經不需要再接收新的connection，所以把connfd關掉
                 * 
                 * @param connfd 要關掉的file descriptor
                 * 
                 */
                close(connfd);

                /**
                 * @brief 
                 * 
                 */
                execlp("sl", "sl", "-l", NULL);
                exit(-1);
            }
            /**
             * @brief parent process 會進到這
             * 
             */
            else{
                printf("Train ID: %d\n", (int)child_pid);
            }
        }
    }
}

void intHandler(int signum)
{
    stop = 1;
}