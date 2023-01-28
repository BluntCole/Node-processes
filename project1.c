#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_MESSAGE_LENGTH 100

int k;

int pidc[100];
int pipes[100][2];

struct appAndMess {
    char apple;
    char message[MAX_MESSAGE_LENGTH];
    int intendedNodeId;
};

void sigHandler(int sig){

	for (int i = 0; i < k; i++){
		kill(pidc[i], SIGINT);
	}

	for (int i = 0; i<k; i++){
		if(close(pipes[i][0]) < 0){
			perror("errpr closing pipe read end");
		}
		if (close(pipes[i][1]) < 0){
			perror("Error closin pipe write end");
		}

		wait(NULL);
	}
	exit(0);
}

void processFunction(int nodeId) {

    struct appAndMess appAndMess;

    while (1) {
        // Receive apple from previous node
        read(pipes[(nodeId + k - 1) % k][0], &appAndMess, sizeof(appAndMess));

        printf("Node %d received apple from node %d\n", nodeId, (nodeId + k - 1) % k);
        // Check if message is intended for this node
        if (appAndMess.intendedNodeId == nodeId) {
            // Copy message and set header to 'empty'
            printf("Node %d received message: %s\n", nodeId, appAndMess.message);
            appAndMess.intendedNodeId = -1;
        } else {
            // Send apple to next node after a small delay
            usleep(1000000);
            write(pipes[nodeId][1], &appAndMess, sizeof(appAndMess));
            printf("Node %d sent apple to node %d\n", nodeId, (nodeId + 1) % k);
        }
    }
}

int main() {

    // Get value for k from user
    printf("How many nodes to you want:  ");
    scanf("%d", &k);
	
    signal(SIGINT, sigHandler);	

    // Create pipes
    for (int i = 0; i < k; i++) {
        pipe(pipes[i]);
    }

    // Spawn k processes
    for (int i = 0; i < k; i++) {
       int pid = fork();
        if (pid == 0) {
            // Child process
            processFunction(i);
            exit(0);
        } else {
            // Parent process
	    pidc[i] = pid;
            printf("Spawned process with ID %d\n", pid);
        }
    }
    // Parent process prompts user for message and node to send to
    char message[MAX_MESSAGE_LENGTH];
    int nodeId;
    int next = 1;

    while(next){
    printf("Enter message to send: ");
    scanf("%s", message);
    printf("Enter node to send message to: ");
    scanf("%d", &nodeId);

    struct appAndMess appAndMess;

    appAndMess.apple = 'A';
    strcpy(appAndMess.message, message);
    appAndMess.intendedNodeId = nodeId;
    // Send apple to specified node with message

    write(pipes[nodeId][1], &appAndMess, sizeof(appAndMess));

    // Wait for processes to complete
    printf("press 1 to send another message or CTRL C to end process: \n");
    scanf("%d", &next);
    }
    for (int i = 0; i < k; i++) {
        wait(NULL);
    }
    return 0;
}
