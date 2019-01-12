#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h> 
#include <time.h>

#define BLUE(string) "\x1b[34m" string "\x1b[0m"
#define RED(string) "\x1b[31m" string "\x1b[0m"
#define GREEN(string) "\x1b[32m" string "\x1b[0m"
#define YELLO(string) "\x1b[33m" string "\x1b[0m"

typedef struct babycows{
    int can_awake;
    int which_crow;
}crow;
typedef struct feed_pot{
    int inUse;
    int which_pot;
}pot;

int count = 0;
sem_t pot_mutex, mom_awake , CS_mutex; //semaphores.
crow *crows; //struct of crows.
pot *pots; //struct of pots.
int crow_num,pot_num,feed_num,mom_age = 0 , pot_count=0;
// int r;

void delay(int milliseconds){
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}
void pots_cons(pot *x){
    x->inUse = 0;
    x->which_pot = count;
}
void crows_cons(crow *x){
    // x->can_awake = 0;
    x->which_crow = count;
    count ++;
}

void *baby_fun(void * baby){
    srand(time(0));
    crow *tmp = (crow*)baby;
    int i=0, *val , mutex_val;
    printf("Baby crow %d is started.\n",tmp->which_crow);
    delay(1000);
    while(1){
        printf("  Baby crow %d is" BLUE("playing.")"\n",tmp->which_crow);
        delay(rand()%500+200);
        printf("\tBaby crow %d is " BLUE("ready to eat.")"\n",tmp->which_crow);
        //awake mother if need!!!
        if (pot_count == pot_num){
            pot_count=0;
            sem_wait(&CS_mutex);
            printf(YELLO("\t\t\t\tBaby crow %d found all pots empty!!   So awaked Mother.\n"),tmp->which_crow);
            printf(YELLO("\t\t\t\tMother crow is awake by baby crow %d and started prepairing food . . .\n"),tmp->which_crow);
            sem_post(&mom_awake);
            sem_post(&CS_mutex);
        }
        //eating
        sem_wait(&pot_mutex);
        sem_wait(&CS_mutex);
        pot_count++;
        for(i = 0; i < pot_num; i++){
            if (pots[i].inUse == 0){
                pots[i].inUse = 1;
                break;
            }
        }
        printf(GREEN("\t\tBaby crow %d is using pot %d for eating.\n"),tmp->which_crow,i);
        delay(rand()%400+100);
        printf(GREEN("\t\tBaby crow %d ")RED("finished eating.\n"),tmp->which_crow);
        sem_post(&CS_mutex);
    }

}
void *mom_fun(void *argv){
    srand(time(0));
    printf("Mother crow is started.\n");
    while(mom_age < feed_num){
        printf(BLUE("\t\t\t\tMother crow is taking a nap!!\n"));
        delay(rand()%500+500);
        sem_wait(&mom_awake); // 
        sem_wait(&CS_mutex);
        mom_age++; // mother is aging.
        delay(rand()%1000+1000);
        for(int i = 0; i < pot_num; i++){
            pots[i].inUse = 0;
            sem_post(&pot_mutex); // prepairing pots;
        }
        printf(YELLO("\t\t\t\tMother crow says" RED("'feeding (%d)' \n")),mom_age);
        sem_post(&CS_mutex);
    }
    printf(RED("\n\n\t\t\t\tMother crow retires after seving %d feedings.  GAME ENDS!!!!\n"),mom_age);  
    return NULL;  
}

int main(){
    pthread_t *baby_t , mom_t;
    //get values.
    printf("Enter the number of Baby Crows:  ");
    scanf("%d",&crow_num);
    printf("Enter the number of pot(must be less than Baby Crows '%d'):   ",crow_num);
    scanf("%d",&pot_num);
    while(crow_num<pot_num){
        printf("Enter the number of pot Again(must be less than Baby Crows '%d'):   ",crow_num);
        scanf("%d",&pot_num);
    }
    printf("Enter the number of feeding:   ");
    scanf("%d",&feed_num);
    if (crow_num == 0) crow_num = 10;
    if (pot_num == 0) pot_num = 10;
    if (feed_num == 0) feed_num = 10;
    printf("\n\n\nMAIN: There are %d baby crows, %d feeding pot_num, %d feeding.\n",crow_num,pot_num,feed_num);
    ///////////////////////////////////////// initializing !!
    sem_init(&pot_mutex,0,pot_num); //init pot semaphore.
    sem_init(&mom_awake,0,0); //init mom semaphore.
    sem_init(&CS_mutex,0,1); //init cs semaphore.

    crows = (crow*)malloc(crow_num*sizeof(crow)); //make crows.
    pots = (pot*)malloc(pot_num*sizeof(pot)); //make pots.
    for(int i = 0; i < crow_num; i++){
        if (i < pot_num){
            pots_cons(&pots[i]); 
        }
        crows_cons(&crows[i]);
    }
    baby_t = (pthread_t*)malloc(crow_num*sizeof(pthread_t)); //make babies thread.
    //////////////////////////////////////// start game!!
    printf("MAIN: Game Started!!!!!\n\n\n");
    for(int i = 0; i < crow_num; i++){ 
        pthread_create(&baby_t[i],NULL,baby_fun,&crows[i]); //start i'th crow's thread. 
    }
    pthread_create(&mom_t,NULL,mom_fun,NULL);
    pthread_join(mom_t,NULL);
    return 0;
}