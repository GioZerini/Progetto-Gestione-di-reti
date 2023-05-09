#define _GNU_SOURCE   
#include <stdio.h>    
#include <stdlib.h>  
#include <stdbool.h> 
#include <assert.h>  
#include <string.h>   
#include <errno.h>  
#include <math.h>
#include <unistd.h>
#include "ndpi_api.h"

int ndpi_predict_linear(u_int32_t *values, u_int32_t num_values, u_int32_t predict_period, u_int32_t *predicted_value, float *c, float *m){
	u_int i;
	float meanX, meanY, meanXY, stddevX, stddevY, covariance, r, alpha, beta;
	struct ndpi_analyze_struct a, b, d;

	if(!values || predict_period < 1 || num_values < 2)
		return -1;

	ndpi_init_data_analysis(&a, 0);
	ndpi_init_data_analysis(&b, 0);
	ndpi_init_data_analysis(&d, 0);

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

	r	= covariance / (stddevX * stddevY);
	beta	= r * (stddevY / stddevX);
	alpha	= meanY - (beta * meanX);

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
	char memTotal[15], memFree[15], buffers[15], cached[15];
	u_int32_t mem[10];


	for(i=0; i<10; i++){
		fp = popen("grep MemTotal /proc/meminfo | awk '{print $2}'", "r");
		if(fgets(memTotal, sizeof(memTotal), fp) == NULL)
			return;
		pclose(fp);
		fp = popen("grep MemFree /proc/meminfo | awk '{print $2}'", "r");
		if(fgets(memFree, sizeof(memFree), fp) == NULL)
			return;
		pclose(fp);
		fp = popen("grep Buffers /proc/meminfo | awk '{print $2}'", "r");
		if(fgets(buffers, sizeof(buffers), fp) == NULL)
			return;
		pclose(fp);
		fp = popen("grep Cached /proc/meminfo | awk '{print $2}'", "r");
		if(fgets(cached, sizeof(cached), fp) == NULL)
			return;
		pclose(fp);
		mem[i] = atoi(memTotal) - atoi(memFree) - atoi(buffers) - atoi(cached);

		sleep(10);
	}

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
}

int main(int argc, char *argv[]){
	testTool();
	return 0;
}
