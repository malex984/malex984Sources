#ifndef TGB_H
#define TGB_H

#include "mod2.h"
#include <omalloc.h>
#include "p_polys.h"

#include "ideals.h"
#include "ring.h"
#include "febase.h"
#include "structs.h"
#include "polys.h"
#include "stdlib.h"


#include "kutil.h"
#include "kInline.cc"
#include "kstd1.h"
#include "kbuckets.h"

#define FULLREDUCTIONS
//#define HALFREDUCTIONS
//#define HEAD_BIN
//#define HOMOGENEOUS_EXAMPLE
#define REDTAIL_S
#define PAR_N 1000
#define AC_NEW_MIN 5
//#define REDTAIL_PROT
//#define QUICK_SPOLY_TEST
struct sorted_pair_node{
  //criterium, which is stable 0. small lcm 1. small i 2. small j
  int i;
  int j;
  int deg;
  int expected_length;
  poly lcm_of_lm;
};


/** 
    reduction_accumulators are objects which are shared by several sums
 **/

class reduction_accumulator{
  
 public:
  /// (1/multiplied)*bucket=reduced original data
  number multiplied;
  ///the polynomial data
  kBucket_pt bucket;
  /// the short exponent vector
  unsigned long sev;
  /// the reference counter
  int counter;
  /// decrease the reference counter, at 0 it deletes the object
  void decrease_counter(){ 
    if((--counter)==0)
      {
	delete this; //self destruction
      }
  }
  int last_reduction_id;
  reduction_accumulator(){
    last_reduction_id=-1;
    bucket=kBucketCreate(currRing);
  }
  ~reduction_accumulator(){
    nDelete(&multiplied);
    kBucketDeleteAndDestroy(&bucket);
  }  

  
};
struct formal_sum_descriptor{
  number c_my;
  number c_ac;
  reduction_accumulator* ac;
};
struct int_pair_node{
  int_pair_node* next;
  int a;
  int b;
};
struct calc_dat
{
  int* rep;
  char** states;
  ideal S;
  ring r;
  int* lengths;
  long* short_Exps;
  kStrategy strat;
  int* T_deg;
  poly* gcd_of_terms;
  int_pair_node* soon_free;
  sorted_pair_node** apairs;
#ifdef HEAD_BIN
  struct omBin_s*   HeadBin;
#endif
  unsigned int reduction_steps;
  int n;
  int normal_forms;
  int current_degree;
  int Rcounter;
  int last_index;
  int max_pairs;
  int pair_top;
  int easy_product_crit;
  int extended_product_crit;
  BOOLEAN is_char0;
};
class red_object{
 public:
  kBucket_pt bucket;
  poly p;
  formal_sum_descriptor* sum;
  unsigned long sev;
  void flatten();
  void validate();
  void reduction_step(int reduction_id, poly reductor_full, int full_len, poly reductor_part, reduction_accumulator* join_to, calc_dat* c);
  void adjust_coefs(number c_r, number c_ac_r);
  int guess_quality(calc_dat* c);
  int clear_to_poly();
};


enum calc_state
  {
    UNCALCULATED,
    HASTREP,
    UNIMPORTANT,
    SOONTREP
  };

static int add_to_reductors(calc_dat* c, poly h, int len);
static int bucket_guess(kBucket* bucket);
static poly redNFTail (poly h,const int sl,kStrategy strat, int len);
static poly redNF2 (poly h,calc_dat* c , int &len);
static void free_sorted_pair_node(sorted_pair_node* s, ring r);
static void shorten_tails(calc_dat* c, poly monom);
static void replace_pair(int & i, int & j, calc_dat* c);
static sorted_pair_node** add_to_basis(poly h, int i, int j,calc_dat* c, int* ip=NULL);
static void do_this_spoly_stuff(int i,int j,calc_dat* c);
ideal t_rep_gb(ring r,ideal arg_I);
static BOOLEAN has_t_rep(const int & arg_i, const int & arg_j, calc_dat* state);
static int* make_connections(int from, poly bound, calc_dat* c);
static int* make_connections(int from, int to, poly bound, calc_dat* c);
static void now_t_rep(const int & arg_i, const int & arg_j, calc_dat* c);
static void soon_t_rep(const int & arg_i, const int & arg_j, calc_dat* c);
static int pLcmDeg(poly a, poly b);
static int simple_posInS (kStrategy strat, poly p,int len, BOOLEAN is_char0);
static BOOLEAN find_next_pair(calc_dat* c, BOOLEAN go_higher=TRUE);

static sorted_pair_node* pop_pair(calc_dat* c);
static BOOLEAN no_pairs(calc_dat* c);
static void clean_top_of_pair_list(calc_dat* c);
static void super_clean_top_of_pair_list(calc_dat* c);
static BOOLEAN state_is(calc_state state, const int & i, const int & j, calc_dat* c);
static BOOLEAN pair_better(sorted_pair_node* a,sorted_pair_node* b, calc_dat* c);
static int pair_better_gen(const void* ap,const void* bp);
static poly redTailShort(poly h, kStrategy strat);
static poly gcd_of_terms(poly p, ring r);
static BOOLEAN extended_product_criterion(poly p1, poly gcd1, poly p2, poly gcd2, calc_dat* c);
static poly kBucketGcd(kBucket* b, ring r);
static void multi_reduction(red_object* los, int & losl, calc_dat* c);
static sorted_pair_node* quick_pop_pair(calc_dat* c);
static sorted_pair_node* top_pair(calc_dat* c);
static int quality(poly p, int len, calc_dat* c);
/**
   makes on each red_object in a region a single_step
 **/
class reduction_step{
 public:
  /// we assume hat all occuring red_objects have same lm, and all
  /// occ. lm's in r[l...u] are the same, only reductor does not occur
  virtual void reduce(red_object* r, int l, int u);
  //int reduction_id;
  virtual ~reduction_step();
  calc_dat* c;
  int reduction_id;
};
class simple_reducer:public reduction_step{
 public:
  poly p;
  kBucket_pt fill_back;
  int p_len;

  void reduce(red_object* r, int l, int u);
  ~simple_reducer();
};
//class sum_canceling_reducer:public reduction_step {
//  void reduce(red_object* r, int l, int u);
//};
struct find_erg{
  poly expand;
  int expand_length;
  int to_reduce_u;
  int to_reduce_l;
  int reduce_by;//index of reductor
  BOOLEAN fromS;//else from los

};

reduction_step* create_reduction_step(find_erg & erg, red_object* r, calc_dat* c);
#endif
