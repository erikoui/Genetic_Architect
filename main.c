#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//TODO: fix segfault on 2 parents (should fix random cutting of last char
const char goal[]="haha benis this is very long see if you can do this one";
#define MUTATIONS_PER_10K 500
#define MAX_MUTATIONS 1
#define POPULATION_SIZE 1000
#define GENOME_SIZE strlen(goal)
#define MAX_GENERATIONS 1000
#define NUM_PARENTS 10
#define ZERO_CHANCE 0.0001

int count_same_chars(const char* str1, const char* str2){
    int i;
    int n=0;
    if(strlen(str1)!=strlen(str2)){
        printf("count_same_chars: Strings have different lengths (str1:%ld, str1:%ld, GENOME_SIZE:%d).\n",strlen(str1),strlen(str2),GENOME_SIZE);
        exit(1);
    }
    for(i=0;i<strlen(str1);i++){
        if(str1[i]==str2[i]){
            n++;
        }
    }
    return n;
}

//This is the score function
double score(const char* s){
    return ((double)count_same_chars(s,goal)/GENOME_SIZE);
}

char random_gene(){
    return rand()%('z'-' ')+' ';
}

//mutates a random gene to another (' '-'z')
void mutate(char** p){
    int n,m,j,i;
    n=rand()%POPULATION_SIZE*MUTATIONS_PER_10K/10000;//how many genomes to mutate
    for(i=0;i<n;i++){
        m=rand()%MAX_MUTATIONS+1;//how many genes to mutate
        for(j=0;j<m;j++){
            p[i][rand()%GENOME_SIZE]=random_gene();
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
        p[i][GENOME_SIZE]='\0';
    }
}

//prints n genomes randomly sampled from p
void print_sample(char** p,int n){
    int index;
    for(;n>=0;n--){
        index=rand()%POPULATION_SIZE;
        printf("%6d: %s (%.3f)\n",index,p[index],score(p[index]));
    }
}

// Fills s with the score of each genome and returns the sum of all scores.
int generate_score_array(char** p,double* s){
    int i;
    double sum=0;//total score
    double max=0;//best genome score
    int maxi;//index of max
    for(i=0;i<POPULATION_SIZE;i++){
        s[i]=score(p[i]);
        sum+=s[i];
        if(s[i]>max){
            max=s[i];
            maxi=i;
        }
    }
    printf("Best Genome\n%6d: %s (%.3f)\n",maxi,p[maxi],max);
    return sum;
}

// Chooses an index of population randomly with more chance the higher its score
int choose_parent(double* s,double tot){
    int i,k;
    double r,p;
    do{
        k=rand()%POPULATION_SIZE;
        r=(double)(rand()%100000)/100000;
        p=s[k]/tot+ZERO_CHANCE;
    }
    while(r>p);
    //printf("Chose parent %d with score %.5f (r=%.5f,p=%.5f)\n",k,s[k],r,p);
    return k;
}

void random_shuffle(int* l,int size){
    int i,r1,r2,t;
    // for(i=0;i<size;i++){
    //     printf("%2d ",l[i]);
    // }
    for(i=0;i<GENOME_SIZE*2;i++){
        r1=rand()%size;
        r2=rand()%size;
        t=l[r1];
        l[r1]=l[r2];
        l[r2]=t;
    }
    // printf(" --> ");
    // for(i=0;i<size;i++){
    //     printf("%2d ",l[i]);
    // }
    // printf("\n");
}

void crossover(char** p,char** new_population,int* unique_randoms,double* s,double tot){
    int i,j,parent_index,c;
    int start=0;
    int end;

    for(i=0;i<POPULATION_SIZE;i++){//for each genome
        random_shuffle(unique_randoms,GENOME_SIZE);//make a list of unique randoms from 0 to GENOME_SIZE-1
        for(j=1;j<=NUM_PARENTS;j++){//for each parent
            parent_index=choose_parent(s,tot);
            end=start+GENOME_SIZE/NUM_PARENTS-1;
            for(c=start;c<end;c++){
                new_population[i][unique_randoms[c]]=p[parent_index][unique_randoms[c]];
            }
            start=end;
        }
        //keep going on the last parent until you make a full genome
        for(c=start;c<GENOME_SIZE;c++){
            new_population[i][unique_randoms[c]]=p[parent_index][unique_randoms[c]];
        }
        start=0;
        p[i][GENOME_SIZE]='\0';
    }

    //move new population to the old one
    for(i=0;i<POPULATION_SIZE;i++){
        p[i]=new_population[i];
    }
}

int main(){
    time_t t;
    srand((unsigned) time(&t));

    double total_score;
    int i;

    char* st1;
    st1=(char*)malloc(GENOME_SIZE*sizeof(char));

    char** population;
    population=(char**)malloc(POPULATION_SIZE*sizeof(char*));
    for(i=0;i<POPULATION_SIZE;i++){
        population[i]=(char*)malloc(GENOME_SIZE*sizeof(char));
    }

    char** new_population;
    new_population=(char**)malloc(POPULATION_SIZE*sizeof(char*));
    for(i=0;i<POPULATION_SIZE;i++){
        new_population[i]=(char*)malloc(GENOME_SIZE*sizeof(char));
    }

    int* unique_randoms;
    unique_randoms=(int*)malloc(GENOME_SIZE*sizeof(int));
    for(i=0;i<GENOME_SIZE;i++){
        unique_randoms[i]=i;
    }

    double* scores;
    scores=(double*)malloc(POPULATION_SIZE*sizeof(double));

    //initialization
    init(population);
    //print_sample(population,10);
    printf("intialized %d genomes. Genome size: %d\n",POPULATION_SIZE,GENOME_SIZE);
    printf("Generation     0 preview:\n");
    print_sample(population,9);
    //loop over generations
    for(i=0;i<MAX_GENERATIONS;i++){
        total_score=generate_score_array(population,scores);
        crossover(population,new_population,unique_randoms,scores,total_score);
        if(i%1==0){
            printf("Gen %d: %.3f average\n",i,total_score/POPULATION_SIZE);
            //printf("Generation %5d preview:\n",i);
            //print_sample(population,9);
        }
        mutate(population);
    }
    return 0;
}
