// Calibration values for all the different boards
#ifndef CALIBRATION_H
#define CALIBRATION_H

//maximum battery capacity
#define BATTERY_CAPACITY 		10000

#define PMB_no				6

#if PMB_no == 1
	//cubic approximation coefficients
	#define coef_a              7.5665
	#define coef_b              -4.8897
	#define coef_c              1.07299
	#define coef_d              -7996.28471

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
	#define coef_a              -0.000000003
	#define coef_b              0.0002
	#define coef_c              -4.5181
	#define coef_d              33946

	//current measurement caliberation
	#define CURRENT_RATIO       1.5173 
	#define CURRENT_OFFSET      77.834

	//voltage readings caliberation
	#define cell6_adc_ratio     31.95
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    171.27 
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624  

#elif PMB_no == 3
	//cubic approximation coefficients
	#define coef_a              -0.000000002
	#define coef_b              0.00001
	#define coef_c              -2.7809
	#define coef_d              20103

	//current measurement caliberation
	#define CURRENT_RATIO       1.4851
	#define CURRENT_OFFSET      104.61

	//voltage readings caliberation
	#define cell6_adc_ratio     32.289
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    141.18
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624 

#elif PMB_no == 4
	//cubic approximation coefficients
	#define coef_a              7.5665
	#define coef_b              -4.8897
	#define coef_c              1.07299
	#define coef_d              -7996.28471

	//current measurement caliberation
	#define CURRENT_RATIO       0.7216811031  
	#define CURRENT_OFFSET      0.03

	//voltage readings caliberation
	#define cell6_adc_ratio     34.25 
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    -592.62 
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624

#elif PMB_no == 5
	//cubic approximation coefficients
	#define coef_a              7.5665
	#define coef_b              -4.8897
	#define coef_c              1.07299
	#define coef_d              -7996.28471

	//current measurement caliberation 
	#define CURRENT_RATIO       1.5452
	#define CURRENT_OFFSET      49.307

	//voltage readings caliberation
	#define cell6_adc_ratio     32.214
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    25.385  
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624

#elif PMB_no == 6
	//cubic approximation coefficients
	#define coef_a              7.5665
	#define coef_b              -4.8897
	#define coef_c              1.07299
	#define coef_d              -7996.28471

	//current measurement caliberation 
	#define CURRENT_RATIO       1.7759  
	#define CURRENT_OFFSET      303.9

	//voltage readings caliberation
	#define cell6_adc_ratio     32.531 
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    -85.304  
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624  

#endif

#endif
