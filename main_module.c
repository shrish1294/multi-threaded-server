
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/un.h>
#include <stddef.h>
#include "queue.h"
#include "dlfcn.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var = PTHREAD_COND_INITIALIZER;
#define inf -100000

int COUNT =0;
int thread_pool = 2;
Queue* q;
 
bool create_worker_thread(int fd);


void log_msg(const char *msg, bool terminate) {
    printf("%s\n", msg);
    if (terminate) exit(-1); /* failure */
}

// cheacks whether the given string can be converted to valid double or not 
bool check_double(char *arr)
{
    int n = strlen(arr);
    int c = 0, d = 0;
    for(int i =0;i<n; i++){
    if(arr[i]=='.')
    {
        c++ ; 
        if(c > 1)
        return false;
    }
    else if(arr[i]=='-')
    {
        d++;
        if(d > 1)
        return false;
    }
    else if(arr[i]-'0' < 0 || arr[i]-'0' > 9)
    return false;
    }
    return true;
}

// calls the dll function and returns the output 
double dll_invoker(char* file_path,    const char* fun, double a, double b)
{
        // const char* fun = Fun;
        void *handle;
        if(b==inf)
        {
        double (*function)(double);
        char *error;

        handle = dlopen (file_path, RTLD_LAZY);
        if (!handle) {
            log_msg("invalid dll name",false);
            fputs (dlerror(), stderr);
            exit(1);
        }

        function = dlsym(handle, fun);
        if ((error = dlerror()) != NULL)  {
            log_msg("invalid funtion", false);
            fputs(error, stderr);
            exit(1);
        }
             return  (*function)(a);
        }
        else{
        double (*function)(double, double);
        char *error;

        handle = dlopen (file_path, RTLD_LAZY);
        if (!handle) {
            log_msg("invalid dll name", false);
            fputs (dlerror(), stderr);
            exit(1);
        }

        function = dlsym(handle, fun);
        if ((error = dlerror()) != NULL)  {
            log_msg("invalid function", false);
            fputs(error, stderr);
            exit(1);
        }
        return  (*function)(a, b);
         }
         return 0;
}



void handle_connection(int* sockfd)
{
    int sock_fd = *sockfd;
    // sleep(2);

    log_msg("SERVER: thread_function: starting", false);
    char buffer[5000];
    memset(buffer, '\0', sizeof(buffer));
    int count = read(sock_fd, buffer, sizeof(buffer));

    
    if (count > 0) {
        printf("SERVER: Received from client: %s\n", buffer);
        //   printf("thread->%d \n", COUNT);
        int n = strlen(buffer);
        char num1[100], num2[100] = "#";
        char file_path[5000];
        char function_name[1000];
        
        int j = 0, i = 0;
        while(i < n && buffer[i]!='\0')
        {
            if(buffer[i]==',')
            break;
            file_path[j] = buffer[i];
            j++;
            i++;
        }
        i++;
        j = 0;
        while(i < n && buffer[i]!='\0')
        {
             if(buffer[i]==',')
            break;
            function_name[j] = buffer[i];
            j++;
            i++;
        }
        i++;
        j = 0;
         while(i < n && buffer[i]!='\0')
        {
             if(buffer[i]==',')
            break;
            num1[j] = buffer[i];
            j++;
            i++;
        }
        i++;
        j = 0;
         while(i < n && buffer[i]!='\0')
        {
             if(buffer[i]==',')
            break;
            num2[j] = buffer[i];
            j++;
            i++;
        }

        
        printf("num1-->%s\n", num1);
        printf("num2-->%s\n", num2);
        if(!check_double(num1))
        {
            log_msg("invalid function argument: expected  a double/float  value", false);
            return ;
        }
        double n1 = atof(num1);
        if(num2[0]!='#'){
        if(!check_double(num2))
        {
            log_msg("invalid funtion argument: expected a double/float value", false);
            return ;
        }
        double n2 = atof(num2);
        // printf("this-> %f ", n2);
         n1 = dll_invoker(file_path, function_name, n1,n2);
        }
        else
         n1 = dll_invoker(file_path, function_name, n1, inf);
        //  prinf
       
        // n2 = dll_invoker(file_path, function_name, n2);
        // printf("%f - \n", n1);
        snprintf( buffer,5000, "%f", n1);
          write(sock_fd, buffer, sizeof(buffer));
          close(sock_fd);
    }
    else
        log_msg("Nothing to read :(", false);
        // printf("the sum is->");
}


// this thread function has actually been used in multi threading 
void* thread_fun(void *arg)
{
    // printf("grggr");
    while(true)
    {
        // sleep(1);
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&condition_var , &mutex);
        int* pclient = dequeue(q);
        sleep(10);
        pthread_mutex_unlock(&mutex);
        if(pclient!=NULL)
        {
            handle_connection(pclient);
        }
       
    }
}


/**

 * Compile it using: gcc main_module.c -lpthread -o ipc_demo
 
 */

/**
 * Create a named (AF_LOCAL) socket at a given file path.
 * @param socket_file
 * @param is_client whether to create a client socket or server socket
 * @return Socket descriptor
 */
int make_named_socket(const char *socket_file, bool is_client) {
    printf("Creating AF_LOCAL socket at path %s\n", socket_file);
    
    if (!is_client && access(socket_file, F_OK) != -1) {
        log_msg("An old socket file exists, removing it.", false);
        if (unlink(socket_file) != 0) {
            log_msg("Failed to remove the existing socket file.", true);
        }
    }
    struct sockaddr_un name;
    /* Create the socket. */
    int sock_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        log_msg("Failed to create socket.", true);
    }

    /* Bind a name to the socket. */
    name.sun_family = AF_LOCAL;
    strncpy (name.sun_path, socket_file, sizeof(name.sun_path));
    name.sun_path[sizeof(name.sun_path) - 1] = '\0';

    /* The size of the address is
       the offset of the start of the socket_file,
       plus its length (not including the terminating null byte).
       Alternatively you can just do:
       size = SUN_LEN (&name);
   */
    size_t size = (offsetof(struct sockaddr_un, sun_path) +
                   strlen(name.sun_path));
    if (is_client) {
        if (connect(sock_fd, (struct sockaddr *) &name, size) < 0) {
            log_msg("connect failed  invalid file_path", 1);
        }
    } else {
        if (bind(sock_fd, (struct sockaddr *) &name, size) < 0) {
            log_msg("bind failed", 1);
        }
    }
    return sock_fd;
}

/**
 * Starts a server socket that waits for incoming client connections.
 * @param socket_file
 * @param max_connects
 */
_Noreturn void start_server_socket(char *socket_file, int max_connects) {
    int sock_fd = make_named_socket(socket_file, false);

    /* listen for clients, up to MaxConnects */
    if (listen(sock_fd, max_connects) < 0) {
        log_msg("Listen call on the socket failed. Terminating.", true); /* terminate */
    }
    log_msg("Listening for client connections...\n", false);
    printf("%d number of threads have been initiated\n", thread_pool);
     pthread_t threads[thread_pool];
    //  mulit-threads creation 
    for(int i =0;i<thread_pool;i++){
       
    pthread_create(&threads[i] , NULL, thread_fun, NULL);
    
    }
    /* Listens indefinitely */
    log_msg("threads created\n", false);
    
    while (1) {
        
        struct sockaddr_in caddr; /* client address */
        int len = sizeof(caddr);  /* address length could change */

        printf("Waiting for incoming connections...\n");
        int client_fd = accept(sock_fd, (struct sockaddr *) &caddr, &len);  /* accept blocks */

        if (client_fd < 0) {
            log_msg("accept() failed. Continuing to next.", 0); /* don't terminate, though there's a problem */
            continue;
        }
        int *pclient = malloc(sizeof(int));
        *pclient = client_fd;
        pthread_mutex_lock(&mutex);
        enqueue(q, pclient);
        pthread_cond_signal(&condition_var);
         pthread_mutex_unlock(&mutex);

        /* Start a worker thread to handle the received connection. */
        // if (!create_worker_thread(client_fd)) {
        //     log_msg("Failed to create worker thread. Continuing to next.", 0);
        //     continue;
        // }

    }  /* while(1) */
}


/**
 * This functions is executed in a separate thread.
 * @param sock_fd
 */
// void thread_function(int sock_fd) 
// {
 
//     COUNT+=1;
   
//     log_msg("SERVER: thread_function: starting", false);
//     char buffer[5000];
//     memset(buffer, '\0', sizeof(buffer));
//     int count = read(sock_fd, buffer, sizeof(buffer));

    
//     if (count > 0) {
//         printf("SERVER: Received from client: %s\n", buffer);
//           printf("thread->%d \n", COUNT);
//         int n = strlen(buffer);
//         char num1[100], num2[100];
//         int j = 0, i = 0;
//         while(i < n && buffer[i]!='\0')
//         {
//             if(buffer[i]==',')
//             break;
//             num1[j] = buffer[i];
//             j++;
//             i++;
//         }
//         i++;
//         j = 0;
//         while(i < n && buffer[i]!='\0')
//         {
//              if(buffer[i]==',')
//             break;
//             num2[j] = buffer[i];
//             j++;
//             i++;
//         }
//         // printf("num1--> %s\n", num1);
//         // printf("num2-->%s\n", num2);
//         int n1 = atoi(num1);
//         int n2 = atoi(num2);
//         n1 +=n2;
//         snprintf( buffer,5000, "%d", n1 );
//         // printf("the sum is->");
//         write(sock_fd, buffer, sizeof(buffer));
//        COUNT-=1;
       
//          /* echo as confirmation */
       
        
//     }
//     close(sock_fd); /* break connection */
//     log_msg("SERVER: thread_function: Done. Worker thread terminating.", false);
//     pthread_exit(NULL); // Must be the last statement
// }

/**
 * This function launches a new worker thread.
 * @param sock_fd
 * @return Return true if thread is successfully created, otherwise false.
 */
bool create_worker_thread(int sock_fd) {
    log_msg("SERVER: Creating a worker thread.", false);
    pthread_t thr_id;
    
    int rc = pthread_create(&thr_id,
            /* Attributes of the new thread, if any. */
                            NULL,
            /* Pointer to the function which will be
             * executed in new thread. */
                            thread_fun,
            /* Argument to be passed to the above
             * thread function. */
                            (void *) sock_fd);
    if (rc) {
        log_msg("SERVER: Failed to create thread.", false);
        return false;
    }
    return true;
}

/**
 * Sends a message to the server socket.
 * @param msg Message to send
 * @param socket_file Path of the server socket on localhost.
 */
void send_message_to_socket(char *msg, char *socket_file) {
    int sockfd = make_named_socket(socket_file, true);

    /* Write some stuff and read the echoes. */
    log_msg("CLIENT: Connect to server, about to write some stuff...", false);
    if (write(sockfd, msg, strlen(msg)) > 0) {
        /* get confirmation echoed from server and print */
        char buffer[5000];
        memset(buffer, '\0', sizeof(buffer));
        if (read(sockfd, buffer, sizeof(buffer)) > 0) {
            printf("CLIENT: Received from server:: %s\n", buffer);
        }
    }
    log_msg("CLIENT: Processing done, about to exit...", false);
    close(sockfd); /* close the connection */
}


/**
 * This is the driver function you can use to test client-server
 * communication using sockets.
 * @param argc
 * @param argv
 * @return
 */
 