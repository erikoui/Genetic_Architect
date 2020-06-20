#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//TODO: fix segfault on 2 parents (should fix random cutting of last char
const char goal[]="hello wsapones";
#define MUTATIONS_PER_10K 500
#define MAX_MUTATIONS 1
#define POPULATION_SIZE 1000
#define GENOME_SIZE strlen(goal)
#define MAX_GENERATIONS 1000
#define PARENTS 5
#define ZERO_CHANCE 0.002

int dist_from_str(const char* str1, const char* str2){
    int i;
    int dist=0;
    for(i=0;i<strlen(str1);i++){
        if(str1[i]==str2[i]){
            dist++;
        }
    }
    return dist;
}

//This is the score function
float score(const char* s){
    return ((float)dist_from_str(s,goal)/GENOME_SIZE);
}

char random_gene(){
    return rand()%('z'-' ')+' ';
}

//mutates a random gene to another (' '-'z')
void mutate(char** p){
    int m,j,i;
    for(i=0;i<POPULATION_SIZE;i++){
        m=rand()%MAX_MUTATIONS+1;
        for(j=0;j<m;j++){
            if(rand()%10000<=MUTATIONS_PER_10K){
                p[i][rand()%GENOME_SIZE]=random_gene();
                //printf("mutated %4d\n",i);
            }
        }
    }
}

//generates a population
void init(char** p){
    int i,j;
    for(i=0;i<POPULATION_SIZE;i++){
        for(j=0;j<GENOME_SIZE;j++){//this loop should be a generator function
            p[i][j]=random_gene();
        }
    }
}

void print_sample(char** p,int n){
    int index;
    for(;n>=0;n--){
        index=rand()%POPULATION_SIZE;
        printf("%d %s: %.3f\n",index,p[index],score(p[index]));
    }
}

//selection
int eugenics(char** p,float* s){
    int i;
    float sum=0;
    float max=0;
    int maxi;
    for(i=0;i<POPULATION_SIZE;i++){
        s[i]=score(p[i]);
        sum+=s[i];
        if(s[i]>max){
            max=s[i];
            maxi=i;
        }
    }
    printf("Best: %s: %.3f\n",p[maxi],max);
    return sum;
}

//chooses an index of population randomly with more chance the higher its score
int choose_parent(float* s,float tot){
    int i,k;
    float r,p;
    do{
        k=rand()%POPULATION_SIZE;
        r=(float)(rand()%100000)/100000;
        p=s[k]/tot+ZERO_CHANCE;
    }
    while(r>p);
    //printf("Chose parent %d with score %.5f (r=%.5f,p=%.5f)\n",k,s[k],r,p);
    return k;
}

void random_shuffle(int* l,int size){
    int i,r1,r2,t;

    for(i=0;i<100;i++){
        r1=rand()%size;
        r2=rand()%size;
        t=l[r1];
        l[r1]=l[r2];
        l[r2]=t;
    }
}

void crossover(char** p,float* s,float tot){
    int i,j,k,c;
    int start=0;
    int end;
    
    int* list;
    list=(int*)malloc(GENOME_SIZE*sizeof(int));
    for(i=0;i<GENOME_SIZE;i++){
        list[i]=i;
    }
   
    char** new_population;
    new_population=(char**)malloc(POPULATION_SIZE*sizeof(char*));
    for(i=0;i<POPULATION_SIZE;i++){
        new_population[i]=(char*)malloc(GENOME_SIZE*sizeof(char));
    }
    
    for(i=0;i<POPULATION_SIZE;i++){
        random_shuffle(list,GENOME_SIZE);
        for(j=0;j<PARENTS;j++){
            k=choose_parent(s,tot);
            end=start+GENOME_SIZE/PARENTS;
            for(c=start;c<=end;c++){
                new_population[i][list[c]]=p[k][list[c]];
            }
            start=end;
        }
        k=choose_parent(s,tot);
        
        for(c=start;c<GENOME_SIZE;c++){
           new_population[i][list[c]]=p[k][list[c]];
        }
        
        start=0;
        p[i][end+1]='\0';
    }
    
    //move new population to the old one
    for(i=0;i<POPULATION_SIZE;i++){
        *(p+i)=*(new_population+i);
    }
}

int main(){
    time_t t;
    srand((unsigned) time(&t));

    float total_score;
    int i;
    
    char* st1;
    st1=(char*)malloc(GENOME_SIZE*sizeof(char));
    
    char** population;
    population=(char**)malloc(POPULATION_SIZE*sizeof(char*));
    for(i=0;i<POPULATION_SIZE;i++){
        population[i]=(char*)malloc(GENOME_SIZE*sizeof(char));
    }

    char** survivors;
    survivors=(char**)malloc(POPULATION_SIZE*sizeof(char*));
    for(i=0;i<POPULATION_SIZE;i++){
        survivors[i]=(char*)malloc(GENOME_SIZE*sizeof(char));
    }
    
    float* scores;
    scores=(float*)malloc(POPULATION_SIZE*sizeof(float));
    
    //initialization
    init(population);
    //print_sample(population,10);
    printf("intialized %d genomes. Genome size: %d\n",POPULATION_SIZE,GENOME_SIZE);
    printf("Generation     0 preview:\n");
    print_sample(population,9);
    //loop over generations
    for(i=0;i<MAX_GENERATIONS;i++){
        total_score=eugenics(population,scores);
        crossover(population,scores,total_score);
        if(i%1==0){
            printf("Gen %d: %.3f average\n",i,total_score/POPULATION_SIZE);
            printf("Generation %5d preview:\n",i);
            print_sample(population,9);
        }
        mutate(population);
       
    }
    return 0;
}
    
