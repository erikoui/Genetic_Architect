#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

//TODO: fix segfault lmao good luck with all those pointers
#define MUTATIONS_PER_10K 500
#define MAX_MUTATIONS 1
#define POPULATION_SIZE 1000
#define GENOME_SIZE 10
#define MAX_GENERATIONS 1000
#define NUM_PARENTS 10
#define ZERO_CHANCE 0.0001

#define ROOM_TYPE_CHANGES 0

const char USE_OPTS[]={'B','M','K','S','L','C','U','O','W','T'};
#define USE_OPTS_SIZE 10
const char B_DIR_OPTS[]={'N','W','S','E'};
#define B_DIR_OPTS_SIZE 4
const char R_DIR_OPTS[]={'R','L'};
#define R_DIR_OPTS_SIZE 2
const int ENSUITE_OPTS_MAX=3;

//This array sets the types of rooms you want to have (e.g 2 bdr with kitchen and 2 wc)
const char CFG_TYPES[]={'B','B','K','W','T','T','M','L','C','C'};
//This one sets if you want ensuite (0) or not (1)
const char CFG_ENSUITE[]={1,1,1,1,0,3,1,3,3};

struct gene{
  /****************************************
  Variable [use] contains the type of room.
  [use] can be:
  B: Bedroom
  M: Master Bedroom
  K: Kitchen
  S: Store
  L: Living Room
  C: Covered Veranda
  U: Uncovered Veranda
  O: Other or Extra room
  W: Walkway
  T: Bathroom
  Remember to change USE_OPTS if you add another room type
  *****************************************/
  char use;

  /****************************************
  Variables [b_dir,r_dir] contains the Bearing and Rotational direction of
  the room relative to room [rel]. Rotational direction means left or right
  side of [rel], seen as if standing in the center of that room.
  [b_dir] can be:
  N: north (up)
  W: west (left)
  S: south (down)
  E: east (right)
  [r_dir] can be:
  L: left
  R: right
  Remember to change R_DIR_OPTS and B_DIR_OPTS if you add another direction (???)
  *****************************************/
  char b_dir,r_dir;

  /****************************************
  Variable [rel] contains the room index that the current room postion is
  based on.
  *****************************************/
  int rel;

  double width,height;

  /****************************************
  Variable [ensuite] defines a bathroom or store if it needs to be connected
  to a bedroom or kitchen.
  [ensuite] can be:
  0: MUST connect
  1: MUST NOT connect
  2: PREFER connect
  3: dont care if connected or not
  *****************************************/
  //TODO: Verandas must be hardcoded to connect to anything except walkways
  int ensuite;
};

/*Converts a genome to a int array for easier working.
  This imposes a significant overhead in exchange for codeability.
  I might rewrite the whole thing in c++ in the future where i can use a
  vector of objects je kani.
*/
void genome_to_int_arr(gene* g, unsigned long long* s){
  int i;
  for(i=0;i<GENOME_SIZE;i++){
    s[i]= 10000000000000*g[i]->use;//2 digits
    s[i]+=   10000000000*g[i]->b_dir;//2
    s[i]+=     100000000*g[i]->r_dir;//2
    s[i]+=       1000000*g[i]->rel;//2
    s[i]+=         10000*floor(g[i]->width*100);//3
    s[i]+=          1000*floor(g[i]->height*100)//3
    s[i]+=               g[i]->ensuite;//1
    printf("gene %d: %llu",i,s[i]);
  }
}

void random_shuffle(int* l,int size){
  int i,r1,r2,t;
  for(i=0;i<GENOME_SIZE*2;i++){
    r1=rand()%size;
    r2=rand()%size;
    t=l[r1];
    l[r1]=l[r2];
    l[r2]=t;
  }
}

void random_shuffle_gene(struct gene* l,int size){
  int i,r1,r2;
  struct gene t;
  for(i=0;i<GENOME_SIZE*2;i++){
    r1=rand()%size;
    r2=rand()%size;
    t=l[r1];
    l[r1]=l[r2];
    l[r2]=t;
  }
}

double normdist(double m,double s){
  double r=(double)(rand()%10000)/10000;
  return 1/(s*sqrt(3.283))*exp(-0.5*((r-m)/s)*((r-m)/s));
}

//This is the score function
double score(const struct gene* s){
  //Bedroom area increases score up to a point
  //Room aspect ratio increases score as it reaches a constant (propose 1.1)
  //Walkway area decreases score
  //more MORE MOOOOOOOOOOARRRRRRRRRRRR
  return 1;
}

double random_dim(char use){
  double w;
  const double BEDROOM_MEAN=3.0;
  const double BEDROOM_STD=0.5;

  const double MASTER_BEDROOM_MEAN=3.7;//ooh racist word
  const double MASTER_BEDROOM_STD=0.5;

  const double KITCHEN_MEAN=3.0;
  const double KITCHEN_STD=0.8;

  const double STORE_MEAN=2.0;
  const double STORE_STD=0.5;

  const double LIVING_ROOM_MEAN=4.5;
  const double LIVING_ROOM_STD=1;

  const double VERANDA_MEAN=5.0;
  const double VERANDA_STD=3;

  const double BATHROOM_MEAN=1.8;
  const double BATHROOM_STD=0.2;

  const double WALKWAY_MEAN=3.0;
  const double WALKWAY_STD=1;

  switch(use){
    case 'B':
    return normdist(BEDROOM_MEAN,BEDROOM_STD);
    break;
    case 'M':
    return normdist(MASTER_BEDROOM_MEAN,MASTER_BEDROOM_STD);
    break;
    case 'K':
    return normdist(KITCHEN_MEAN,KITCHEN_STD);
    break;
    case 'S':
    return normdist(STORE_MEAN,STORE_STD);
    break;
    case 'L':
    return normdist(LIVING_ROOM_MEAN,LIVING_ROOM_STD);
    break;
    case 'C':
    case 'U':
    return normdist(VERANDA_MEAN,VERANDA_STD);
    break;
    case 'O':
    return normdist(STORE_MEAN,STORE_STD);
    break;
    case 'T':
    return normdist(BATHROOM_MEAN,BATHROOM_STD);
    break;
    case 'W':
    return normdist(WALKWAY_MEAN,WALKWAY_STD);
    break;
    default:
    printf("BAD ROOM USE PLS FIX AAAAAA\n");
    exit(0);
    break;
  }
  return 100;
}

void randomize_gene(struct gene* g){
  //genes are essentially room definitions.
  if(ROOM_TYPE_CHANGES){
    g->use=USE_OPTS[rand()%(USE_OPTS_SIZE)];
  }
  g->b_dir=B_DIR_OPTS[rand()%(B_DIR_OPTS_SIZE)];
  g->r_dir=R_DIR_OPTS[rand()%(R_DIR_OPTS_SIZE)];
  g->rel=rand()%GENOME_SIZE;
  g->width=random_dim(g->use);
  g->height=random_dim(g->use);
}

//mutates a random gene to another (' '-'z')
void mutate(struct gene** p){
  int n,m,j,i;
  n=rand()%POPULATION_SIZE*MUTATIONS_PER_10K/10000;//how many genomes to mutate
  for(i=0;i<n;i++){
    m=rand()%MAX_MUTATIONS+1;//how many genes to mutate
    for(j=0;j<m;j++){
      randomize_gene(&p[i][rand()%GENOME_SIZE]);
    }
  }
}

//generates a population
void init(struct gene*** p){
  int i,j;
  for(i=0;i<POPULATION_SIZE;i++){
    for(j=0;j<GENOME_SIZE;j++){//this loop is the generator function
      p[i][j]->use=CFG_TYPES[j];
      p[i][j]->ensuite=CFG_ENSUITE[j];
      randomize_gene(*p[i]);
    }
    random_shuffle_gene(*p[i],GENOME_SIZE);
  }
}

void print_genome(struct gene** g){
  int i;
  for(i=0;i<GENOME_SIZE;i++){
    printf("%c%c%c %2d %.2f %.2f %d",g[i]->use,
                                     g[i]->b_dir,
                                     g[i]->r_dir,
                                     g[i]->rel,
                                     g[i]->width,
                                     g[i]->height,
                                     g[i]->ensuite);
  }
}

//prints n genomes randomly sampled from p
void print_sample(struct gene*** p,int n){
  int index;
  printf("Sample Genes:\n");
  for(;n>=0;n--){
    index=rand()%POPULATION_SIZE;
    printf("%6d: ",index);
    print_genome(p[index]);
    printf(" (%.3f)\n",score(*p[index]));
  }
}

// Fills s with the score of each genome and returns the sum of all scores.
int generate_score_array(struct gene*** p,double* s){
  int i;
  double sum=0;//total score
  double max=0;//best genome score
  int maxi;//index of max
  for(i=0;i<POPULATION_SIZE;i++){
    s[i]=score(*p[i]);
    sum+=s[i];
    if(s[i]>max){
      max=s[i];
      maxi=i;
    }
  }
  printf("Best Genome\n%6d: ",maxi);
  print_genome(p[maxi]);
  printf(" (%.3f)\n",max);
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

void crossover(struct gene*** p,struct gene*** new_population,int* unique_randoms,double* s,double tot){
  int i,j,parent_index,c;
  int start=0;
  int end;

  //printf("Started Crossover.\n");
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
  srand((unsigned) time(NULL));

  double total_score;
  int i;

  printf("Allocating memory...");
  fflush(stdout);
  struct gene** population;
  population=( struct gene**)malloc(POPULATION_SIZE*sizeof( struct gene*));
  for(i=0;i<POPULATION_SIZE;i++){
    population[i]=( struct gene*)malloc(GENOME_SIZE*sizeof( struct gene));
  }

  struct gene** new_population;
  new_population=( struct gene**)malloc(POPULATION_SIZE*sizeof( struct gene*));
  for(i=0;i<POPULATION_SIZE;i++){
    new_population[i]=( struct gene*)malloc(GENOME_SIZE*sizeof( struct gene));
  }

  int* unique_randoms;
  unique_randoms=(int*)malloc(GENOME_SIZE*sizeof(int));
  for(i=0;i<GENOME_SIZE;i++){
    unique_randoms[i]=i;
  }

  double* scores;
  scores=(double*)malloc(POPULATION_SIZE*sizeof(double));
  printf("Allocated.\n");

  print_sample(&population,9);
  //initialization
  init(&population);
  //print_sample(population,10);
  printf("intialized %d genomes. Genome size: %d\n",(int)POPULATION_SIZE,(int)GENOME_SIZE);
  printf("Generation     0 preview:\n");
  print_sample(&population,9);
  //loop over generations
  for(i=0;i<MAX_GENERATIONS;i++){
    total_score=generate_score_array(&population,scores);
    crossover(&population,&new_population,unique_randoms,scores,total_score);
    if(i%1==0){
      printf("Gen %d: %.3f average\n",i,total_score/POPULATION_SIZE);
      //printf("Generation %5d preview:\n",i);
      //print_sample(population,9);
    }
    mutate(population);
  }
  return 0;
}
