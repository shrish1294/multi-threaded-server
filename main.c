#include "main_module.c"




/**
 * This is the driver function you can use to test client-server
 * communication using sockets.
 * @param argc
 * @param argv
 * @return
 */

// gcc main_module.c -lpthread -ldl -o ipc_demo


int main(int argc, char *argv[]) {
    printf("argc -> %d\n", argc);
    /* assuming the queue size = 10*/


    int queue_size = 50;
    q = createQueue(queue_size);

    // thread size =8 
    thread_pool = 2; 
    
    if (argc < 3) {
        printf("Usage: %s [server|client] [Local socket file path] [comma separated following arguents(needed only in case of client)->dll_name, function_name,function_arg-1,function_arg-2(only needed when functions needs 2 arguments)]\n",argv[0]);
        exit(-1);
    }
    if (0 == strcmp("server", argv[1])) {
    
        
        start_server_socket(argv[2], 10);
        

    } else {
    
        send_message_to_socket(argv[3], argv[2]);
    }
}