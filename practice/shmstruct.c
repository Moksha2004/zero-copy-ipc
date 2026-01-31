#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
struct Player {
    int id;
    int health;
    float x;
};

int main(int argc, char *argv[]){
    key_t key;
    int shmid;
    struct Player *player_ptr;
    if((key=ftok("shmstruct.c",'R'))==-1) {
        perror("ftok");
        exit(1);
    }
    if((shmid=shmget(key,sizeof(struct Player),0644 | IPC_CREAT)) ==-1) {
        perror("shmget");
        exit(1);
    }
    void *raw=shmat(shmid, (void*)0,0);
    if(raw==(void*)(-1)) {
        perror("shamt");
        exit(1);}
    player_ptr=(struct Player *)raw;
    if(argc>1){
        printf("Writing to shared memory...\n");
        player_ptr->id=atoi(argv[1]);
        player_ptr->health=100;
        player_ptr->x=50.35;
        printf("PLayer set: ID=%d, Health=%d\n",player_ptr->id, player_ptr->health);
    } 
    else {
        printf("Reading from shared memory...\n");
        printf("Player ID: %d\n", player_ptr->id);
        printf("Player Health: %d\n", player_ptr->health);
        printf("Player Position: %.2f\n", player_ptr->x);
    }
    if(shmdt(player_ptr)==-1){
        perror("shmdt");
        exit(1);
    }
    return 0;
}