#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <random>
#include <string>
#include <map>

/************************************CONSTANTS*********************************/
const bool VERBOSE=true;//Set to true to print debug info
const int UPDATE_INTERVAL=10;//how many generations between printing stats

#define MUTATIONS_PER_10K 500
#define MAX_MUTATIONS 1
#define POPULATION_SIZE 1000
#define MAX_GENERATIONS 1000
#define NUM_PARENTS 10
#define ZERO_CHANCE 0.000

#define ROOM_TYPE_CHANGES 0

const char USE_OPTS[]={'B','M','K','S','L','C','U','O','W','T'};
#define USE_OPTS_SIZE 10
const char B_DIR_OPTS[]={'N','W','S','E'};
#define B_DIR_OPTS_SIZE 4
const char R_DIR_OPTS[]={'R','L'};
#define R_DIR_OPTS_SIZE 2
const int ENSUITE_OPTS_MAX=3;
/******************************************************************************/

/**********************************PARAMETERS**********************************/
//This sets the types of rooms in the plan (e.g 2 bdr with kitchen and 2 wc)
static const char CFG_TYPES_[]={'B','B','K','W','T','T','M','L','C','C'};
std::vector<char> CFG_TYPES;
//This one sets if you want ensuite (0) or not (1)
static const char CFG_ENSUITE_[]={1,1,1,1,0,3,1,3,3};
std::vector<char> CFG_ENSUITE;
/******************************************************************************/

/************************************GLOBALS***********************************/
std::random_device rd;  //seed for the random number engine
std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
std::uniform_real_distribution<> rand_uni(0, 1);
std::normal_distribution<> rand_norm(0,1);
int GENOME_SIZE;
std::map<char,double> mean_aspect;
std::map<char,double> std_aspect;
/******************************************************************************/

/*****************************DATA STRUCTURES**********************************/
class Gene{
public:
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

  Gene(){
    use='O';
    b_dir='N';
    r_dir='L';
    rel=0;
    width=3.0;
    height=3.5;
    ensuite=3;
  }
};

class Genome{
public:
  std::vector<Gene> genome;

  Genome(){
    genome.resize(GENOME_SIZE);
  }
};

class Population{
public:
  std::vector<Genome> population;

  Population(){
    population.resize(POPULATION_SIZE);//this also runs the genome constructor
  }
};
/******************************************************************************/

/*****************************HELPER FUNCTONS**********************************/
double normdist(double m,double s){
  return rand_norm(gen)*s+m;
}

double unidist(double m){
  return rand_uni(gen)*m;
}

void random_shuffle(std::vector<int>& l){
  int i,r1,r2,t;
  for(i=0;i<l.size()*2;i++){
    r1=std::floor(unidist(l.size()));
    r2=std::floor(unidist(l.size()));
    t=l[r1];
    l[r1]=l[r2];
    l[r2]=t;
  }
}

void random_shuffle_genome(Genome& g){
  int i,r1,r2;
  Gene t;
  for(i=0;i<g.genome.size()*2;i++){
    r1=std::floor(unidist(g.genome.size()));
    r2=std::floor(unidist(g.genome.size()));
    t=g.genome[r1];
    g.genome[r1]=g.genome[r2];
    g.genome[r2]=t;
  }
}

double random_dim(char use){
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

void randomize_gene(Gene& g){
  //genes are essentially room definitions.
  if(ROOM_TYPE_CHANGES){
    g.use=USE_OPTS[rand()%(USE_OPTS_SIZE)];
  }
  g.b_dir=B_DIR_OPTS[rand()%(B_DIR_OPTS_SIZE)];
  g.r_dir=R_DIR_OPTS[rand()%(R_DIR_OPTS_SIZE)];
  g.rel=rand()%GENOME_SIZE;
  g.width=random_dim(g.use);
  g.height=random_dim(g.use);
}

void print_genome(Genome g){
  int i;
  for(i=0;i<GENOME_SIZE;i++){
    printf("\n\t%c%c%c %2d %.2f %.2f %d",g.genome[i].use,
                                     g.genome[i].b_dir,
                                     g.genome[i].r_dir,
                                     g.genome[i].rel,
                                     g.genome[i].width,
                                     g.genome[i].height,
                                     g.genome[i].ensuite);
  }
  std::cout<<std::endl;
}

double score(Genome& s);//declared here but defined later

//prints n genomes randomly sampled from p
void print_sample(Population& p,int n){
  int index;
  printf("Sample Genes:\n");
  for(;n>=0;n--){
    index=std::floor(unidist(POPULATION_SIZE));
    printf("%6d: ",index);
    print_genome(p.population[index]);
    printf("\t\t\t\t(%.3f)\n",score(p.population[index]));
  }
}
/******************************************************************************/

//set values to some globals
void init(){
  CFG_TYPES=std::vector<char>(CFG_TYPES_, CFG_TYPES_ + sizeof(CFG_TYPES_)  / sizeof(CFG_TYPES_[0]));

  CFG_ENSUITE=std::vector<char>(CFG_ENSUITE_, CFG_ENSUITE_ + sizeof(CFG_ENSUITE_) / sizeof(CFG_ENSUITE_[0]));
  if(VERBOSE){
    std::string str(CFG_TYPES.begin(), CFG_TYPES.end());
    std::cout<<"CFG_TYPES initialized to "<<str<<std::endl;
  }
  GENOME_SIZE=CFG_TYPES.size();

  //'B','M','K','S','L','C','U','O','W','T'
  mean_aspect['B']=1;
  std_aspect['B']=0.3;
  mean_aspect['M']=1;
  std_aspect['M']=0.3;
  mean_aspect['K']=0.83;
  std_aspect['K']=0.5;
  mean_aspect['S']=0.5;
  std_aspect['S']=1;
  mean_aspect['L']=0.8;
  std_aspect['L']=0.5;
  mean_aspect['C']=1;
  std_aspect['C']=3;
  mean_aspect['U']=1;
  std_aspect['U']=3;
  mean_aspect['O']=1;
  std_aspect['O']=1;
  mean_aspect['W']=3;
  std_aspect['W']=10;
  mean_aspect['T']=1;
  std_aspect['T']=0.2;
}

//generates a population
void generate_population(Population& p){
  int i,j;
  if(VERBOSE){
    std::cout<<"Generating Population.\n   Starting size of p: "<<p.population.size()<<std::endl;
    std::cout<<"   Starting size of p[0]: "<<p.population[0].genome.size()<<std::endl;
  }
  for(i=0;i<POPULATION_SIZE;i++){
    for(j=0;j<GENOME_SIZE;j++){//this loop is the generator function
      p.population[i].genome[j].use=CFG_TYPES[j];
      p.population[i].genome[j].ensuite=CFG_ENSUITE[j];
      randomize_gene(p.population[i].genome[j]);
    }
    random_shuffle_genome(p.population[i]);
  }
}

//This is the score function
double score(Genome& g){
  double score=0;
    //Room aspect ratio increases score as it reaches a constant (propose 1.1)
  double room_aspect,z_aspect;
  int i;
  for(i=0;i<GENOME_SIZE;i++){
    if(g.genome[i].width<g.genome[i].height){
      room_aspect=g.genome[i].width/g.genome[i].height;
    }else{
      room_aspect=g.genome[i].height/g.genome[i].width;
    }
    //Calculate the z-score of this aspect ratio (assuming normdist)
    double m=mean_aspect[g.genome[i].use];
    double s=std_aspect[g.genome[i].use];
    z_aspect=(room_aspect-m)/s+1;//x of normdist graph
    z_aspect=z_aspect*exp(-0.5*((z_aspect-m)/s)*((z_aspect-m)/s));//y
    if(VERBOSE){
      //std::cout<<"Room  ("<<g.genome[i].use<<") aspect: "<<room_aspect<<" (z="<<z_aspect<<")"<<std::endl;
    }
    score+=z_aspect;
  }
  score/=GENOME_SIZE;//normalize so that final score max=1

  //Bedroom area increases score up to a point
  //Walkway area decreases score
  //more MORE MOOOOOOOOOOARRRRRRRRRRRR
  // dont forget to score/=num_of_criteria;
  score/=1;
  return score;
}

//The entire mutation step for the whole population
void mutate(Population& p){
  int n,m,j,i;
  n=rand()%p.population.size()*MUTATIONS_PER_10K/10000;//how many genomes to mutate
  for(i=0;i<n;i++){
    m=rand()%MAX_MUTATIONS+1;//how many genes to mutate
    for(j=0;j<m;j++){
      randomize_gene(p.population[i].genome[unidist(GENOME_SIZE)]);
    }
  }
}

// Fills s with the score of each genome, c_s with the cumulative score
// and returns the sum of all scores.
int generate_score_array(Population& p,std::vector<double>& s,std::vector<double>& c_s){
  int i;
  double sum=0;//total score
  double max=0;//best genome score
  int maxi;//index of max
  for(i=0;i<p.population.size();i++){
    s[i]=score(p.population[i]);
    if(VERBOSE){
      //std::cout<<s[i]<<std::endl;
    }
    sum+=s[i];
    c_s[i]=sum;
    if(s[i]>max){
      max=s[i];
      maxi=i;
    }
  }
  printf("Best Genome\n%6d: ",maxi);
  print_genome(p.population[maxi]);
  printf(" (%.3f)\n",max);
  return sum;
}

// Chooses an index of population randomly with more chance the higher its score
int choose_parent(std::vector<double>& c_s,double tot){
  int i,k,count=0;
  double r,p=2;
  r=unidist(tot);

  for(i=0;i<c_s.size();i++){
    if(c_s[i]>=r){
      break;
    }
  }
  if(VERBOSE){
    //printf("Chose parent %d with cum score %.5f (r=%.5f,p=%.5f)\n",i,c_s[i],r,p);
  }
  return i;
}


void crossover(Population& p,Population& new_population,
          std::vector<int>& unique_randoms,std::vector<double>& s,
          std::vector<double>& c_s,double tot){
  int i,j,parent_index,c;
  int start=0;
  int end;

  //printf("Started Crossover.\n");
  for(i=0;i<p.population.size();i++){//for each genome
    random_shuffle(unique_randoms);//make a list of unique randoms from 0 to GENOME_SIZE-1
    for(j=1;j<=NUM_PARENTS;j++){//for each parent
      parent_index=choose_parent(c_s,tot);
      end=start+GENOME_SIZE/NUM_PARENTS-1;
      for(c=start;c<end;c++){
        new_population.population[i].genome[unique_randoms[c]]=p.population[parent_index].genome[unique_randoms[c]];
      }
      start=end;
    }
    //keep going on the last parent until you make a full genome
    for(c=start;c<GENOME_SIZE;c++){
      new_population.population[i].genome[unique_randoms[c]]=p.population[parent_index].genome[unique_randoms[c]];
    }
    start=0;
  }

  //move new population to the old one
  p.population.swap(new_population.population);
}


void run_GA(Population p){
  Population new_population;
  std::vector<double> scores,cumulative_scores;
  std::vector<int> unique_randoms;

  double total_score;
  int i;

  new_population.population.resize(p.population.size());
  scores.resize(p.population.size());
  cumulative_scores.resize(p.population.size());
  unique_randoms.resize(p.population.size());

  if(VERBOSE){
    std::cout<<"Running GA..."<<std::endl;
  }

  for(i=0;i<MAX_GENERATIONS;i++){
    total_score=generate_score_array(p,scores,cumulative_scores);
    crossover(p,new_population,unique_randoms,scores,cumulative_scores,total_score);
    mutate(p);
    if(VERBOSE){
      if(i%UPDATE_INTERVAL==0){
        printf("Gen %d: %.3f average\n",i,total_score/POPULATION_SIZE);
        printf("Generation %5d preview:\n",i);
        print_sample(p,9);
      }
    }
  }
}


int main(){
  init();
  Population p;
  generate_population(p);
  printf("Intialized %d genomes. Genome size: %d\n",(int)POPULATION_SIZE,(int)GENOME_SIZE);
  printf("Generation     0 preview:\n");
  print_sample(p,9);
  run_GA(p);
  return 0;
}
