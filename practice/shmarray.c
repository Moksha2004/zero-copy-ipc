#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#define MAX_PLAYERS 5
struct Player {
    int id;
    int health;
};

int main(int argc, char *argv[]){
    key_t key;
    int shmid;
    struct Player *player_ptr;
    if((key=ftok("shmarray.c",'R'))==-1) {
        perror("ftok");
        exit(1);
    }
    size_t total_size= sizeof(struct Player)*MAX_PLAYERS;
    if((shmid=shmget(key,total_size,0644 | IPC_CREAT)) ==-1) {
        perror("shmget");
        exit(1);
    }
    void *raw=shmat(shmid, (void*)0,0);
    if(raw==(void*)(-1)) {
        perror("shamt");
        exit(1);}
    player_ptr=(struct Player *)raw;
    if(argc>2){
        printf("Chnaging players health...\n");
        int id=atoi(argv[1]);
        player_ptr[id].health=atoi(argv[2]);
        printf("PLayer set: ID=%d, Health=%d\n",id, player_ptr[id].health);
    } 
    else if(argc>1){
        printf("Initializing %d playes in the shared memory...\n",MAX_PLAYERS);
        for(int i=0;i<MAX_PLAYERS;i++){
            player_ptr[i].id=i;
            player_ptr[i].health=100-10*i;
        }
        printf("Done writing\n");
    }
    else {
        printf("Reading from shared memory...\n");
        for(int i=0;i<MAX_PLAYERS;i++){
        printf("Player ID: %d ", player_ptr[i].id);
        printf("Player Health: %d\n", player_ptr[i].health);}
    }
    if(shmdt(player_ptr)==-1){
        perror("shmdt");
        exit(1);
    }
    return 0;
}