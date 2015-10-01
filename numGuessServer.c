#define _REENTRANT
#include  "server.h"

/*
 *Peter Haschke
 *Brandon Weigel
 * main()
 */

//pthread_mutex_t mutex_cache = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutex_client_socket = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_init(mutex_client_socket);

int main(int argc, char** argv) {
    int server_socket;                 // descriptor of server socket
    struct sockaddr_in server_address; // for naming the server's listening socket
    int client_socket;

    pthread_t threads;
    //Holds number result of the thread.
    int result;
    void *thread_result;

    // create unnamed network socket for server to listen on
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    
    // name the socket (making sure the correct network byte ordering is observed)
    server_address.sin_family      = AF_INET;           // accept IP addresses
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // accept clients on any interface
    server_address.sin_port        = htons(PORT);       // port to listen on
    
    // binding unnamed socket to a particular port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }
    
    // listen for client connections (pending connections get put into a queue)
    if (listen(server_socket, NUM_CONNECTIONS) == -1) {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }
    
    // server loop
    while (TRUE) {
        
	//pthread_mutex_lock(mutex_client_socket);
        // accept connection to client
        if ((client_socket = accept(server_socket, NULL, NULL)) == -1) {
            perror("Error accepting client");
        } else {
	   // int *test = &client_socket;
	    result = pthread_create(&threads, NULL, handle_client, (void *) &client_socket);
 	   
	   //If fails to create thread
	   if(result != 0){
		perror("Thread creation failed");
		exit(EXIT_FAILURE);
	   }
          //handle_client(client_socket);
        }
    }
}


/*
 * handle client
 */
void *handle_client(void *socket) {
    //Changing client_socket from void point back to int.
    int client_socket = *(int*)socket;

    //pthread_mutex_unlock(mutex_client_socket);
    char input;
    int keep_going = TRUE;
    int fence = 1024/2;
    int increment = 1024/2;
    int number = 0;
    
    //Create input buffer and number buffer
    char inputBuffer [50];
    char numBuffer [10];
    
    //Write starting text.
    write(client_socket, "Think of a number between 1 and 1024.\n", sizeof(char)*39);

    //Modified while loop.
    //While loop will break when the value of increment is 0.
    while (increment) {
	//Clearing number buffer to make sure they are empty
	memset (numBuffer, 0, sizeof(numBuffer));
	memset (inputBuffer, 0, sizeof(inputBuffer));

	//Convert int to char by writting to numBuffer array.
	sprintf (numBuffer, "%u", fence);

	//Prompt user.
	write(client_socket, "Is your number greater than ", sizeof(char)*28);
	write(client_socket, numBuffer, sizeof(numBuffer));
	write(client_socket, "\n", sizeof(char)*2);
	
        // read char from client into the input buffer
        switch (read(client_socket, inputBuffer, sizeof(inputBuffer))) {
            case 0:
                keep_going = FALSE;
                printf("\nEnd of stream, returning ...\n");
                break;
            case -1:
                perror("Error Reading from network!\n");
                keep_going = FALSE;
                break;
        }
	
	//Only care about the first character in the buffer.
	input = inputBuffer[0];
	
	//If input is q break out of while loop and end connection.
        if (input == 'q') {
            write(client_socket, &input, sizeof(char));
            break;
        }

	//If input is y increase number and fence by increment.
	//Else decrement the fence if value of increment is 1 or 
	//less it will break out of the while loop.
	if (input == 'y') {
		number += increment;
		increment /= 2;
		fence += increment;
	} else {
		increment /= 2;
		fence -= increment;
	}
    }
	//Increment final number by 1 to get correct result.
	number++;

	//Clearing number buffer to use.
	memset (numBuffer, 0, sizeof(numBuffer));

	//Convert int to char by writting to numBuffer array.
	sprintf (numBuffer, "%u", number);

	//Write final message.
	write(client_socket, "Your number: ", sizeof(char)*13);
	write(client_socket, numBuffer, sizeof(numBuffer));
	write(client_socket, "\n", sizeof(char)*2);
    
    // cleanup
    if (close(client_socket) == -1) {
        perror("Error closing socket");
        exit(EXIT_FAILURE); 
    } else {
        printf("\nClosed socket to client, exit");
    }
}
