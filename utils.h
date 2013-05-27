/*
 *  utils.h
 *  this header file is for helper functions as well as for some common functionality
 *
 *  Created by Mikhail Yudelson on 5/3/12.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */
#include <map>
#include <string>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <algorithm>
#include "StripedArray.h"
#include <time.h>

using namespace std;

#ifndef _UTILS_H
#define _UTILS_H

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))
#define Calloc(type,n) (type *)calloc(n,sizeof(type))
#define Realloc(ptr,n) (void *)realloc(ptr,n)
#define SAFETY 1e-12 // value to substitute for zero for safe math
#define pi_const 3.14159265358979323846

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define NPAR_MAX  SCHAR_MAX
#define NCAT_MAX  SHRT_MAX
#define NCAT2_MAX INT_MAX
#define NDAT_MAX  UINT_MAX
//http://stackoverflow.com/questions/2053843/min-and-max-value-of-data-type-in-c

typedef signed char NPAR;    // number of observations or states, now 128 max, KEEP THIS SIGNED, we need -1 code for NULL
typedef short NCAT;          // number of categories, groups or skills, now 32K max; LEAVE THIS UNSIGNED, we need -1 code for NULL
typedef int NCAT2;           // number of categories 2 (larger), items, now 2 bill max
typedef unsigned int NDAT;   // number of data rows, now 4 bill max
typedef double NUMBER;       // numeric float format

//enum SOLVER { // deprecating
//    BKT_NULL     =  0, // 0 unassigned
//    BKT_CGD      =  1, // 1 - Conjugate Gradient Descent by skill
//    BKT_GD       =  2, // 2 - Gradient Descent by skill
//    BKT_BW       =  3, // 3 - Baum Welch  by skill
//    BKT_GD_BW    =  4, // 4 - Gradient Descent then Expectation Maximization (Baum-Welch) by skill
//    BKT_BW_GD    =  5, // 5 - Expectation Maximization (Baum-Welch) then Gradient Descent by skill
//    BKT_GD_G     =  6, // 6 - Gradient Descent by group
//    BKT_GD_PIg   =  7, // 7 - Gradient Descent: PI by group, A,B by skill
//    BKT_GD_PIgk  =  8, // 8 - Gradient Descent, PI=logit(skill,group), other by skill
//    BKT_GD_APIgk =  9, // 9 - Gradient Descent, PI=logit(skill,group), A=logit(skill,group), B by skill
//    BKT_GD_Agk   = 10, //10 - Gradient Descent, A=logit(skill,group), PI,B by skill
//    BKT_GD_T     = 11  //11 - Gradient Descent by skill with transfer matrix
//};

// Fitting method
enum METHOD {
    METHOD_BW   = 1,  // 1 - Baum Welch (expectation maximization)
    METHOD_GD   = 2,  // 2 - Gradient Descent
    METHOD_CGD  = 3   // 3 - Conjugate Gradient Descent
};

enum CROSS_VALIDATION {
    CV_GROUP = 'g',  // 1 - group (student) stratified
    CV_ITEM  = 'i',  // 2 - item stratified
    CV_NSTR  = 'n'   // 3 - non-stratified
};

// "Structure" of the model we are fitting with a method
enum STRUCTURE {
    STRUCTURE_SKILL   = 1,  // 1 - all by skill
    STRUCTURE_GROUP   = 2,  // 2 - all by group (user)
    STRUCTURE_PIg     = 3,  // 3 - PI by group, A,B by skill
    STRUCTURE_PIgk    = 4,  // 4 - PI by skll&group, A,B by skill
    STRUCTURE_PIAgk   = 5,  // 5 - PI, A by skll&group, B by skill
    STRUCTURE_Agk     = 6,   // 6 - A by skll&group, PI,B by skill
    STRUCTURE_PIABgk  = 7,  // 5 - PI, A, B by skll&group
    STRUCTURE_SKILL_T = 8   // 6 - by skill with transfer matrix
};

// return type cummrizing a result of a fir of the [subset of the] data
struct FitResult {
    int iter;   // interations
    NUMBER pO0; // starting log-likelihood
    NUMBER pO;  // final log-likelihood
    int conv;   // converged? (maybe just went to max-iter)
};

// a sequence of observations (usually belonging to a student practicing a skill)
struct data {
	NDAT ndat; // number of data points (observations)
	NDAT cnt;  // help counter, used for building the data and "banning" data from being fit when cross-valudating based on group
    //	NPAR *obs; // onservations array - will become the pointer array to the big data
    NDAT *idx; // these are 'ndat' indices to the through arrays (e.g. param.dat_obs and param.dat_item)
	NUMBER *c; // nS  - scaling factor
    int *time;
	NUMBER **alpha; // ndat x nS
	NUMBER **beta;  // ndat x nS
	NUMBER **gamma; // ndat x nS
	NUMBER ***xi; // ndat x nS x nS
	NUMBER p_O_param; // ndat
    NUMBER loglik; // loglikelihood
	NCAT k,g; // pointers to skill (k) and group (g)
};

// a sequence of observations (usually belonging to a student with respect to a property /e.g. problem/)
struct pdata {
	NDAT ndat; // number of data points (observations)
	NDAT cnt;  // help counter, used for building the data and "banning" data from being fit when cross-valudating based on group
    //	NPAR *obs; // onservations array - will become the pointer array to the big data
    NDAT *idx; // these are 'ndat' indices to the through arrays (e.g. param.dat_obs and param.dat_item)
	NUMBER *c; // nS  - scaling factor
    int *time;
	NUMBER **alpha; // ndat x nS
	NUMBER **beta;  // ndat x nS
	NUMBER **gamma; // ndat x nS
	NUMBER ***xi; // ndat x nS x nS
	NUMBER p_O_param; // ndat
    NUMBER loglik; // loglikelihood
	NCAT p,g; // pointers to skill (k) and group (g)
};

// parameters of the problem, including configuration parameters, vocabularies of string values, and data
struct param {
	// configurable
	NUMBER *init_params;
	NUMBER *param_lo;
	NUMBER *param_hi;
	NUMBER tol;  // tolerance of termination criterion (0.0001 by default)
    NPAR time; // 1-read time data from 5th column as milliseconds since epoch (0-default)
	int maxiter; // maximum iterations (200 by default)
	NPAR quiet;   // quiet mode (no outputs)
	NPAR single_skill; // 0 - do nothing, 1 - fit all skills as skingle skill, to set a starting point, 2 - enforce fit single skill
	NPAR structure; // whether to fit by skill, by group, or mixture of them
	NPAR solver; // whether to first fit all skills as skingle skill, to set a starting point
	NPAR solver_setting; // to be used by individual solver
	NUMBER C;// weight of the L1 norm penalty
	int metrics;   // compute AIC, BIC, RMSE of training
	int metrics_target_obs;   // target observation for RMSE of training
    int predictions; // report predictions on training data
	NPAR cv_folds; // cross-validation folds
	NPAR cv_strat; // cross-validation stratification
	NPAR cv_target_obs; // cross-validation target observation to validate prediction of
    // data
    StripedArray<NPAR> *dat_obs;
    StripedArray<NCAT> *dat_group;
    StripedArray<NCAT> *dat_skill;
    StripedArray<NCAT2> *dat_item;
    StripedArray< NCAT* > *dat_multiskill;
    StripedArray<int> *dat_time;
	// derived from data
    NDAT   N;       // number of ALL data rows
	NPAR  nS;		// number of states
	NPAR  nO;		// number of unique observations
	NCAT  nG;       // number of subjects (sequences, groups)
	NCAT  nK;		// number of skills (subproblems)
	NCAT2 nI;		// number of items (problems)
    // null-skill data by student
    NDAT N_null; // total number of null skill instances
    struct data *null_skills;
    NCAT n_null_skill_group; // number of groups (students) with rows not labelled by any skill
	// data arrays, by skill
    NDAT ndata; // number of all data sequence where KC label is present ( nG*nK is an upper boundary )
	NCAT *k_numg; // num groups for skill
	struct data *all_data; // all data sequence pointers in one array
	struct data **k_data; // all skill-group data sequence pointers in one array (by skill)
	struct data ***k_g_data; // skill-group data pointer, it is a pointer itself
	// data arrays, by group
	NCAT *g_numk; // num skills for group
	struct data **g_data; // all group-skill data sequence pointers in one array (by group)
	struct data ***g_k_data; // group_skill data pointer, it is a pointer itself
	char multiskill; // multiskill per observation flag, 0 - single skill, [separator character] otherwise
    // vocabilaries
    map<string,NCAT> *map_group_fwd; // string to id
    map<NCAT,string> *map_group_bwd; // id to string
    map<string,NCAT2> *map_step_fwd; // string to id
    map<NCAT2,string> *map_step_bwd; // id to string
    map<string,NCAT> *map_skill_fwd; // string to id
    map<NCAT,string> *map_skill_bwd; // id to string
	// fitting specific
	NUMBER ArmijoC1;				// c1 param for Armijo rule (rf. http://en.wikipedia.org/wiki/Wolfe_conditions)
	NUMBER ArmijoC2;				// c2 param for 2nd Wolfe criterion (rf. http://en.wikipedia.org/wiki/Wolfe_conditions)
	NUMBER ArmijoReduceFactor;		// Reduction to the step if rule is not satisfied
	NUMBER ArmijoSeed;				// Seed step
	NUMBER ArmijoMinStep;			// Minimum step to consider before abandoing reducing it
    // coord descend
    NPAR first_iteration_qualify; // at what iteration to start considering parameter for "graduating" (converging) >=0
    NPAR iterations_to_qualify;   // how many interations of stable parameter values qualify it for "graduating" (converging)
};

void destroy_input_data(struct param *param);

// reading/writing solver info
void writeSolverInfo(FILE *fid, struct param *param);
void readSolverInfo(FILE *fid, struct param *param, NDAT *line_no);

// projection of others
int compareNumber (const void * a, const void * b);
void qsortNumber(NUMBER* ar, NPAR size);
void qsortNumberRev(NUMBER* ar, NPAR size);
void projsimplex(NUMBER* ar, NPAR size);

// projection of my own
bool issimplex(NUMBER* ar, NPAR size);
bool issimplexbounded(NUMBER* ar, NUMBER *lb, NUMBER *ub, NPAR size);
void projectsimplex(NUMBER* ar, NPAR size);
void projectsimplexbounded(NUMBER* ar, NUMBER *lb, NUMBER *ub, NPAR size);


void toZero1DNumber(NUMBER *ar, NDAT size);
void toZero2DNumber(NUMBER **ar, NDAT size1, NPAR size2);
void toZero3DNumber(NUMBER ***ar, NDAT size1, NPAR size2, NPAR size3);

NUMBER* init1DNumber(NDAT size);
NUMBER** init2DNumber(NDAT size1, NPAR size2);
NUMBER*** init3DNumber(NCAT size1, NDAT size2, NPAR size3);
NPAR** init2DNCat(NCAT size1, NCAT size2);

void free2DNumber(NUMBER** ar, NDAT size1);
void free3DNumber(NUMBER ***ar, NCAT size1, NDAT size2);
void free2DNCat(NPAR**ar, NCAT size1);

void cpy1DNumber(NUMBER* source, NUMBER* target, NDAT size);
void cpy2DNumber(NUMBER** source, NUMBER** target, NDAT size1, NPAR size2);
void cpy3DNumber(NUMBER*** source, NUMBER*** target, NDAT size1, NPAR size2, NPAR size3);

void swap1DNumber(NUMBER* source, NUMBER* target, NDAT size);
void swap2DNumber(NUMBER** source, NUMBER** target, NDAT size1, NPAR size2);
void swap3DNumber(NUMBER*** source, NUMBER*** target, NDAT size1, NPAR size2, NPAR size3);

NUMBER safe01num(NUMBER val); // convert number to a safe [0, 1] range
NUMBER safe0num(NUMBER val); // convert number to a safe (0, inf) or (-inf, 0) range
NUMBER itself(NUMBER val);
NUMBER logit(NUMBER val);
NUMBER sigmoid(NUMBER val);
NUMBER deprecated_fsafelog(NUMBER val); // fast and safe log for params
NUMBER safelog(NUMBER val); // safe log for prediction
NUMBER sgn(NUMBER val);
			 
void add1DNumbersWeighted(NUMBER* sourse, NUMBER* target, NPAR size, NUMBER weight);

void add2DNumbersWeighted(NUMBER** sourse, NUMBER** target, NPAR size1, NPAR size2, NUMBER weight);

// scaling params for HMM
bool isPasses(NUMBER* ar, NPAR size);
bool isPassesLim(NUMBER* ar, NPAR size, NUMBER* lb, NUMBER *ub);

NUMBER doLog10Scale1D(NUMBER *ar, NPAR size);
NUMBER doLog10Scale2D(NUMBER **ar, NPAR size1, NPAR size2);
NUMBER doLog10Scale1DGentle(NUMBER *grad, NUMBER *par, NPAR size);
NUMBER doLog10Scale2DGentle(NUMBER **grad, NUMBER **par, NPAR size1, NPAR size2);

void zeroLabels(NCAT xdat, struct data** x_data); // for skill of group
void zeroLabels(struct param* param); // set counts in all data sequences to zero

// whether one value is no less than 20% of the sum
bool isBalancedArray(NUMBER *ar, NPAR size);

//http://bozeman.genome.washington.edu/compbio/mbt599_2006/hmm_scaling_revised.pdf
#define LOGZERO -1e10
NUMBER eexp(NUMBER x);
NUMBER eln(NUMBER x);
NUMBER elnsum(NUMBER eln_x, NUMBER eln_y);
NUMBER elnprod(NUMBER eln_x, NUMBER eln_y);


//
// The heavy end - common functionality
//
void set_param_defaults(struct param *param);
void RecycleFitData(NCAT xndat, struct data** x_data, struct param *param);

//
// working with time
//

// limits are the borders of time bins, there are nlimits+1 bins total
NPAR sec_to_linear_interval(int time, int *limits, NPAR nlimits);
// 8 categories: <20m, <1h, same day, next day, same week, next week, <30d, >=30d
NPAR sec_to_9cat(int time1, int time2, int *limits, NPAR nlimits);
// write time intervals to file
void write_time_interval_data(param* param, const char *file_name);

// penalties
//NUMBER penalty_offset = 0.5;
NUMBER L2penalty(param* param, NUMBER w);


#endif


