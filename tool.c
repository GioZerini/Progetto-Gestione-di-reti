void testTool() {
	int i;
  FILE *fp;
  char memTotal[15], memFree[15], buffers[15], cached[15];
  u_int32_t mem[10];

	
  for(i=0; i<10; i++){
  	fp = popen("grep MemTotal /proc/meminfo | awk '{print $2}'", "r");
  	if(fgets(memTotal, sizeof(memTotal), fp) == NULL)
			return 0;

		fp = popen("grep MemFree /proc/meminfo | awk '{print $2}'", "r");
  	if(fgets(memFree, sizeof(memFree), fp) == NULL)
			return 0;

		fp = popen("grep Buffers /proc/meminfo | awk '{print $2}'", "r");
  	if(fgets(buffers, sizeof(buffers), fp) == NULL)
			return 0;

		fp = popen("grep Cached /proc/meminfo | awk '{print $2}'", "r");
  	if(fgets(cached, sizeof(cached), fp) == NULL)
			return 0;

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
}