#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <random>
#include <string>
#include <map>
#include <stack>

#include <opencv2/opencv.hpp>

//TODO: Score function needs more criteria

/************************************CONSTANTS*********************************/
const bool VERBOSE=true;//Set to true to print debug info
const int UPDATE_INTERVAL=10;//how many generations between printing stats

#define MUTATIONS_PER_10K 500
#define MAX_MUTATIONS 1
#define POPULATION_SIZE 1000
#define MAX_GENERATIONS 100
#define NUM_PARENTS 3
#define ZERO_CHANCE 0.000

#define ROOM_TYPE_CHANGES 0

const char USE_OPTS[]={'B','M','K','S','L','C','U','O','W','T'};
#define USE_OPTS_SIZE 10
const char B_DIR_OPTS[]={'N','S','W','E'};
#define B_DIR_OPTS_SIZE 4
const char R_DIR_OPTS[]={'R','L'};
#define R_DIR_OPTS_SIZE 2
const int ENSUITE_OPTS_MAX=3;
/******************************************************************************/

/**********************************PARAMETERS**********************************/
//This sets the types of rooms in the plan (e.g 2 bdr with kitchen and 2 wc)
static const char CFG_TYPES_[]={'B','K','W','C'};
std::vector<char> CFG_TYPES;
//this sets the connections of each room(is populated in init())
std::vector<std::vector<int> > CFG_CONNS;

//This one sets if you want ensuite (0) or not (1)
static const char CFG_ENSUITE_[]={1,1,1,1};
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

std::vector<cv::Scalar> color_pallete;
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
  the room relative to room [next_room_index]. Rotational direction means left or right
  side of [next_room_index], seen as if standing in the center of that room.
  [b_dir] can be:
  N: north (up)
  W: west (left)
  S: south (down)
  E: east (right)
  [r_dir] is:
  The x or y offset (depending on b_dir) as a percentage of the width or height
  it is always up-> down or left->right positive.
  *****************************************/
  char b_dir;
  double r_dir;

  /****************************************
  Vector [next_room_index] contains the room indeices that b_dir and r_dir
  affect the position of.
  this implies the genome is a many-to-one directed graph(node leads to many
  others, but is lead to only by one). indegree<=1, outdegree>=0
  *****************************************/
  std::vector<int> next_room_index;
  int previous_room_index;


  double width,height;

  //Default constructor
  Gene(){
    use='O';
    b_dir='N';
    r_dir='L';
    previous_room_index=0;
    width=3.0;
    height=3.5;
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

//TODO: replace rand() with unidist
void randomize_gene(Gene& g){
  //genes are essentially room definitions.
  g.b_dir=B_DIR_OPTS[rand()%(B_DIR_OPTS_SIZE)];
  g.r_dir=unidist(2)-1;//random from -1 to 1
  //g.next_room_index=rand()%GENOME_SIZE;
  g.width=random_dim(g.use);
  g.height=random_dim(g.use);
}

//TODO: better formatting
void print_genome(Genome g){
  int i,j;
  std::cout.precision(4);
  std::cout.width(8);
  for(i=0;i<GENOME_SIZE;i++){
    std::cout<<"\n\t"<<g.genome[i].use<<g.genome[i].b_dir<<"\t"<<g.genome[i].r_dir<<"\t[";
    for(j=0;j<g.genome[i].next_room_index.size();j++){
      std::cout<<g.genome[i].next_room_index[j]<<", ";
    }
    std::cout<<"]("<<g.genome[i].previous_room_index<<")\t"<<g.genome[i].width<<"\t"<<g.genome[i].height;
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

  std::vector<int> tmpconn;
  CFG_CONNS.resize(GENOME_SIZE);
  //('B','K','W','C')
  tmpconn.push_back(3);//connect bedroom to covered veranda
  CFG_CONNS[0]=tmpconn;
  tmpconn.clear();
  std::cout<<"segfault"<<std::endl;

  //connect kitchen to nothing
  CFG_CONNS[1]=tmpconn;

  tmpconn.push_back(0);//connect walkway to bedroom
  tmpconn.push_back(1);//and kitchen
  CFG_CONNS[2]=tmpconn;
  tmpconn.clear();

  //connect veranda to nothing
  CFG_CONNS[3]=tmpconn;

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

  color_pallete.push_back(cv::Scalar(110,68,255));
  color_pallete.push_back(cv::Scalar(184,146,255));
  color_pallete.push_back(cv::Scalar(255,194,226));
  color_pallete.push_back(cv::Scalar(255,144,179));
  color_pallete.push_back(cv::Scalar(239,122,133));
}

void generate_prevs(Genome& g){
  int i,j;
  for(i=0;i<GENOME_SIZE;i++){
    for(j=0;j<g.genome[i].next_room_index.size();j++){
      g.genome[g.genome[i].next_room_index[j]].previous_room_index=i;
    }
  }
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
      randomize_gene(p.population[i].genome[j]);
      p.population[i].genome[j].next_room_index=CFG_CONNS[j];
      p.population[i].genome[j].previous_room_index=-1;
    }
    //random_shuffle_genome(p.population[i]);
    generate_prevs(p.population[i]);
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
    if(g.genome[i].width<0){
      score=0;
    }
    if(g.genome[i].height<0){
      score=0;
    }
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
  return i;
}

void crossover(Population& p,Population& new_population,
          std::vector<int>& unique_randoms,std::vector<double>& s,
          std::vector<double>& c_s,double tot){
  int i,j,parent_index,c;
  int start=0;
  int end;

  for(i=0;i<p.population.size();i++){//for each genome
    random_shuffle(unique_randoms);//make a list of unique randoms from 0 to GENOME_SIZE-1
    for(j=1;j<=NUM_PARENTS;j++){//for each parent
      parent_index=choose_parent(c_s,tot);
      //std::cout<<"Parent "<<j<<" ("<<parent_index<<"): "<<std::endl;
      //print_genome(p.population[parent_index]);
      end=start+GENOME_SIZE/NUM_PARENTS;
      for(c=start;c<end;c++){
        //std::cout<<unique_randoms[c]<<" ";
        new_population.population[i].genome[unique_randoms[c]]=p.population[parent_index].genome[unique_randoms[c]];
      }
      start=end;
    }
    //keep going on the last parent until you make a full genome
    for(c=start;c<GENOME_SIZE;c++){
      //std::cout<<unique_randoms[c]<<" ";
      new_population.population[i].genome[unique_randoms[c]]=p.population[parent_index].genome[unique_randoms[c]];
    }
    //std::cout<<std::endl<<"Child: ("<<i<<"): "<<std::endl;
    //print_genome(new_population.population[i]);
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
  unique_randoms.resize(GENOME_SIZE);
  for(i=0;i<unique_randoms.size();i++){
    unique_randoms[i]=i;
  }

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

void draw_room_and_move(cv::Mat& img,Population& p,int p_index,int g_index,int prev_index,std::stack<double>& x,std::stack<double>& y){
  double w=p.population[p_index].genome[g_index].width*100;
  double h=p.population[p_index].genome[g_index].height*100;
  cv::String use(1,p.population[p_index].genome[g_index].use);//make the use char into a cv string
  cv::Scalar color(color_pallete[g_index%color_pallete.size()]);

  cv::Rect r=cv::Rect(x.top(),y.top(),w,h);
  cv::rectangle(img,r,color,5,8,0);
  cv::putText(img, use, cv::Point(x.top()+w/2,y.top()+h/2), cv::FONT_HERSHEY_DUPLEX, /*size=*/2.0, color,2);

  if(prev_index!=-1){
    double next_w=  p.population[p_index].genome[prev_index].width*100;
    double next_h=  p.population[p_index].genome[prev_index].height*100;
    char b_dir=     p.population[p_index].genome[g_index].b_dir;
    double r_dir=     p.population[p_index].genome[g_index].r_dir;

    switch(b_dir){
      case 'N':
        y.push(y.top()-h);
        break;
      case 'S':
        y.push(y.top()+next_h);
        break;
      case 'W':
        x.push(x.top()-w);
        break;
      case 'E':
        x.push(x.top()+next_w);
        break;
      }
  }
}

std::vector<cv::Rect> room_rects;
std::vector<bool> visited;
//this fills room_rects, which can also be used to get distances between rooms.
void traverse_genome(Population& p,int p_index,int root_node,double x,double y){
  if(visited[root_node]){
    return;
  }
  visited[root_node]=true;
  int i;
  double new_x=x,new_y=y;

  double w=p.population[p_index].genome[root_node].width*100;
  double h=p.population[p_index].genome[root_node].height*100;

  for(i=0;i<p.population[p_index].genome[root_node].next_room_index.size();i++){
    int next_index=p.population[p_index].genome[root_node].next_room_index[i];
    double next_h=p.population[p_index].genome[next_index].height*100;
    double next_w=p.population[p_index].genome[next_index].width*100;

    char b_dir=p.population[p_index].genome[next_index].b_dir;
    double r_dir=p.population[p_index].genome[next_index].r_dir;
    switch(b_dir){
     case 'N':
       new_y=y-next_h;
       new_x=x+w*r_dir;
       break;
     case 'S':
       new_y=y+h;
       new_x=x+w*r_dir;
       break;
     case 'W':
       new_x=x-next_w;
       new_y=y+h*r_dir;
       break;
     case 'E':
       new_x=x+w;
       new_y=y+h*r_dir;
       break;
    }
    traverse_genome(p,p_index,next_index,new_x,new_y);
  }
  std::cout<<x<<" "<<y<<" "<<w<<" "<<h<<std::endl;
  cv::Rect r(x,y,w,h);
  room_rects.push_back(r);
}

/*this is the image generator. it makes a graphical representation of a genome*/
void generate_representation(Population& p, int index,bool save){
  std::cout<<"Making image..."<<std::endl;
  cv::Mat img(2000,2000, CV_8UC3, cv::Scalar(50,50,50)); //This will store the image
  int k;

  visited.clear();
  visited.resize(GENOME_SIZE);

  for(k=0;k<GENOME_SIZE;k++){
    if(p.population[index].genome[k].previous_room_index==-1){//room has no dependencies
      traverse_genome(p,index,k,1000,1000);
    }
  }

  for(k=0;k<room_rects.size();k++){
      cv::Scalar color(color_pallete[k%color_pallete.size()]);
      cv::rectangle(img,room_rects[k],color,5,8,0);
  }

  print_genome(p.population[index]);
  cv::namedWindow( "window", CV_WINDOW_NORMAL );
  cv::resizeWindow("window", 500, 500);
  cv::imshow( "window", img );
  if(save)
    cv::imwrite("image.jpg",img);
}

int main(){
  init();
  Population p;

  generate_population(p);
  printf("Intialized %d genomes. Genome size: %d\n",(int)POPULATION_SIZE,(int)GENOME_SIZE);
  printf("Generation     0 preview:\n");
  print_sample(p,9);
  run_GA(p);
  generate_representation(p,2,false);
  cv::waitKey(0);
  return 0;
}
