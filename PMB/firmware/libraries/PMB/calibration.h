// Calibration values for all the different boards
#ifndef CALIBRATION_H
#define CALIBRATION_H

//maximum battery capacity
#define BATTERY_CAPACITY 		10000

#define PMB_no					1

#if PMB_no == 1
	//cubic approximation coefficients
	#define coef_a              2.801388537
	#define coef_b              -208.6689126
	#define coef_c              5196.993309
	#define coef_d              -43182.43852

	//current measurement caliberation
	#define CURRENT_RATIO       1.7123 
	#define CURRENT_OFFSET      21.202

	//voltage readings caliberation
	#define cell6_adc_ratio     37.793
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    102.67 
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624  

#elif PMB_no == 2
	//cubic approximation coefficients
	#define coef_a              2.801388537
	#define coef_b              -208.6689126
	#define coef_c              5196.993309
	#define coef_d              -43182.43852

	//current measurement caliberation
	#define CURRENT_RATIO       0.7216811031  
	#define CURRENT_OFFSET      0.03

	//voltage readings caliberation
	#define cell6_adc_ratio     0.0332 
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    -0.2543  
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624  

#elif PMB_no == 3
	//cubic approximation coefficients
	#define coef_a              2.801388537
	#define coef_b              -208.6689126
	#define coef_c              5196.993309
	#define coef_d              -43182.43852

	//current measurement caliberation
	#define CURRENT_RATIO       0.7216811031  
	#define CURRENT_OFFSET      0.03

	//voltage readings caliberation
	#define cell6_adc_ratio     0.0332 
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    -0.2543  
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624  

#elif PMB_no == 4
	//cubic approximation coefficients
	#define coef_a              2.801388537
	#define coef_b              -208.6689126
	#define coef_c              5196.993309
	#define coef_d              -43182.43852

	//current measurement caliberation
	#define CURRENT_RATIO       0.7216811031  
	#define CURRENT_OFFSET      0.03

	//voltage readings caliberation
	#define cell6_adc_ratio     0.0332 
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    -0.2543  
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624

#elif PMB_no == 5
	//cubic approximation coefficients
	#define coef_a              2.801388537
	#define coef_b              -208.6689126
	#define coef_c              5196.993309
	#define coef_d              -43182.43852

	//current measurement caliberation 
	#define CURRENT_RATIO       0.7216811031  
	#define CURRENT_OFFSET      0.03

	//voltage readings caliberation
	#define cell6_adc_ratio     0.0332 
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    -0.2543  
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624

#elif PMB_no == 6
	//cubic approximation coefficients
	#define coef_a              2.801388537
	#define coef_b              -208.6689126
	#define coef_c              5196.993309
	#define coef_d              -43182.43852

	//current measurement caliberation 
	#define CURRENT_RATIO       0.7216811031  
	#define CURRENT_OFFSET      0.03

	//voltage readings caliberation
	#define cell6_adc_ratio     0.0332 
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    -0.2543  
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624  

#endif

#endif