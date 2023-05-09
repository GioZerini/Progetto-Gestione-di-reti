#define _GNU_SOURCE   
#include <stdio.h>    
#include <stdlib.h>  
#include <stdbool.h> 
#include <assert.h>  
#include <string.h>   
#include <errno.h>  
#include <math.h>
#include <unistd.h>

int ndpi_predict_linear(u_int32_t *values, u_int32_t num_values, u_int32_t predict_period, u_int32_t *predicted_value, float *c, float *m);

struct ndpi_analyze_struct {
  u_int32_t *values;
  u_int32_t min_val, max_val, sum_total, num_data_entries, next_value_insert_index;
  u_int16_t num_values_array_len /* length of the values array */;

  struct {
    u_int64_t sum_square_total;
  } stddev;
};

#define DEFAULT_SERIES_LEN  64
#define MAX_SERIES_LEN      512
#define MIN_SERIES_LEN      8


static void *(*_ndpi_malloc)(size_t size);

static void (*_ndpi_free)(void *ptr);

static volatile long int ndpi_tot_allocated_memory;

void *ndpi_malloc(size_t size) {
  __sync_fetch_and_add(&ndpi_tot_allocated_memory, size);
  return(_ndpi_malloc ? _ndpi_malloc(size) : malloc(size));
}


void ndpi_free(void *ptr) {
  if(_ndpi_free) {
    if(ptr)
      _ndpi_free(ptr);
  } else {
    if(ptr)
      free(ptr);
  }
}

void ndpi_init_data_analysis(struct ndpi_analyze_struct *ret, u_int16_t _max_series_len) {
  u_int32_t len;

  // Inizializza ret con tutti 0
  memset(ret, 0, sizeof(*ret));

  if(_max_series_len > MAX_SERIES_LEN) _max_series_len = MAX_SERIES_LEN;
  ret->num_values_array_len = _max_series_len;

  if(ret->num_values_array_len > 0) {
    // Lunghezza array in byte
    len = sizeof(u_int32_t) * ret->num_values_array_len;
    if((ret->values = ndpi_malloc(len)) != NULL)
      // Inizializza array valori con tutti 0
      memset(ret->values, 0, len);
    else
      ret->num_values_array_len = 0;
  }
}


void ndpi_free_data_analysis(struct ndpi_analyze_struct *d, u_int8_t free_pointer) {
  if(d && d->values) ndpi_free(d->values);
  if(free_pointer) ndpi_free(d);
}


void ndpi_data_add_value(struct ndpi_analyze_struct *s, const u_int32_t value) {
  if(!s)
    return;

  if(s->sum_total == 0)
    s->min_val = s->max_val = value;
  else {
    if(value < s->min_val) s->min_val = value;
    if(value > s->max_val) s->max_val = value;
  }

  s->sum_total += value, s->num_data_entries++;

  if(s->num_values_array_len) {
    s->values[s->next_value_insert_index] = value;

    if(++s->next_value_insert_index == s->num_values_array_len)
      s->next_value_insert_index = 0;
  }

  s->stddev.sum_square_total += (u_int64_t)value * (u_int64_t)value;
}

float ndpi_data_average(struct ndpi_analyze_struct *s) {
  if((!s) || (s->num_data_entries == 0))
    return(0);
  
  return((s->num_data_entries == 0) ? 0 : ((float)s->sum_total / (float)s->num_data_entries));
}


float ndpi_data_mean(struct ndpi_analyze_struct *s) {
  return(ndpi_data_average(s));
}

float ndpi_data_variance(struct ndpi_analyze_struct *s) {
  if(!s)
    return(0);
  float v = s->num_data_entries ?
    ((float)s->stddev.sum_square_total - ((float)s->sum_total * (float)s->sum_total / (float)s->num_data_entries)) / (float)s->num_data_entries : 0.0;
  
  return((v < 0  /* rounding problem */) ? 0 : v);
}

float ndpi_data_stddev(struct ndpi_analyze_struct *s) {
  return(sqrt(ndpi_data_variance(s)));
}

int ndpi_predict_linear(u_int32_t *values, u_int32_t num_values, u_int32_t predict_period, u_int32_t *predicted_value, float *c, float *m){
	
  u_int i;
  float meanX, meanY, meanXY, stddevX, stddevY, covariance, r, alpha, beta;
  struct ndpi_analyze_struct a, b, d;
  
  if(!values || predict_period < 1 || num_values < 2)
		return -1;

	ndpi_init_data_analysis(&a, 3);
  ndpi_init_data_analysis(&b, 3);
	ndpi_init_data_analysis(&d, 3);
  
	/* Add values */
  for(i=0; i<num_values; i++){
    ndpi_data_add_value(&a, i);
    ndpi_data_add_value(&b, values[i]);
		ndpi_data_add_value(&d, i * values[i]);
  }

	meanX      = ndpi_data_mean(&a);
  meanY      = ndpi_data_mean(&b);
	meanXY     = ndpi_data_mean(&d);
  stddevX    = ndpi_data_stddev(&a);
  stddevY    = ndpi_data_stddev(&b);
  covariance = meanXY - (meanX * meanY);

  if(fpclassify(stddevX) == FP_ZERO || fpclassify(stddevY) == FP_ZERO) {
    ndpi_free_data_analysis(&a, 0);
		ndpi_free_data_analysis(&b, 0);
		ndpi_free_data_analysis(&d, 0);
    return -1;
  }
  
  r          = covariance / (stddevX * stddevY);
	beta       = r * (stddevY / stddevX);
	alpha      = meanY - (beta * meanX);

  *predicted_value = alpha + (beta * (predict_period + num_values - 1));
  *c = alpha;
  *m = beta;

	ndpi_free_data_analysis(&a, 0);
	ndpi_free_data_analysis(&b, 0);
	ndpi_free_data_analysis(&d, 0);
  
	return 0;
}

void testTool() {
	int i;
  FILE *fp;
  char *memTotal, *memFree, *buffers, *cached;
  u_int32_t *mem;
	
  memTotal = malloc(sizeof(char)*15);
  memFree = malloc(sizeof(char)*15);
  buffers = malloc(sizeof(char)*15);
  cached = malloc(sizeof(char)*15);

  mem = malloc(sizeof(u_int32_int)*15);

	
  for(i=0; i<10; i++){
  	fp = popen("grep MemTotal /proc/meminfo | awk '{print $2}'", "r");
  	if(fgets(memTotal, sizeof(memTotal), fp) == NULL)
			return;

		fp = popen("grep MemFree /proc/meminfo | awk '{print $2}'", "r");
  	if(fgets(memFree, sizeof(memFree), fp) == NULL)
			return;

		fp = popen("grep Buffers /proc/meminfo | awk '{print $2}'", "r");
  	if(fgets(buffers, sizeof(buffers), fp) == NULL)
			return;

		fp = popen("grep Cached /proc/meminfo | awk '{print $2}'", "r");
  	if(fgets(cached, sizeof(cached), fp) == NULL)
			return;

    mem[i] = atoi(memTotal) - atoi(memFree) - atoi(buffers) - atoi(cached);

    sleep(10);
  }

	pclose(fp);
		
  for(i=0; i<10; i++){
    printf("%d KB\n", mem[i]);
	}

	u_int32_t predicted_value;
	float c, m;
	int ret_val;
	ret_val = ndpi_predict_linear(mem, 10, 6, &predicted_value, &c, &m);
	if(ret_val == 0){
		printf("Computation OK\n");
	}else{
		printf("Error in computation\n");
	}
	printf("The y-intercept is: %f\n", c);
	printf("The slope is: %f\n", m);
	printf("The predicted value is: %d\n", predicted_value);
	
	free(memTotal);
	free(memFree);
	free(buffers);
	free(cached);
	free(mem);
}

int main(int argc, char *argv[]){
	testTool();
	return 0;
}
