
// Calibration values for all the different boards
#ifndef CALIBRATION_H
#define CALIBRATION_H

//maximum battery capacity
#define BATTERY_CAPACITY 		36000000

#define PMB_no				 3

#if PMB_no == 1
	//cubic approximation coefficients
	#define coef_a              4.69753
	#define coef_b              -446.67745
	#define coef_c              15910.16368
	#define coef_d              -251561.5118
	#define coef_e				1489641.436

	//current measurement caliberation
	#define CURRENT_RATIO       3.3583
	#define CURRENT_OFFSET      -94.338

	//voltage readings caliberation
	#define cell6_adc_ratio     31.94
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    192.22 
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624  

#elif PMB_no == 2
	//cubic approximation coefficients
	#define coef_a              4.69753
	#define coef_b              -446.67745
	#define coef_c              15910.16368
	#define coef_d              -251561.5118
	#define coef_e				1489641.436

	//current measurement caliberation
	#define CURRENT_RATIO       2.7797 
	#define CURRENT_OFFSET      -62.246

	//voltage readings caliberation
	#define cell6_adc_ratio     31.998
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    192.74 
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624  

#elif PMB_no == 3  //HARDCODED CAUSE THE PMB READS CELL5
	//cubic approximation coefficients
	#define coef_a              4.42455
	#define coef_b              -420.46384
	#define coef_c              14966.72708
	#define coef_d              -236478.77089
	#define coef_e				1399267.88461

	//current measurement caliberation
	#define CURRENT_RATIO       2.9219
	#define CURRENT_OFFSET      -166.11

	//voltage readings caliberation
	#define cell6_adc_ratio     38.531
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    178.61
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624 

#elif PMB_no == 4
	//cubic approximation coefficients
	#define coef_a              4.69753
	#define coef_b              -446.67745
	#define coef_c              15910.16368
	#define coef_d              -251561.5118
	#define coef_e				1489641.436

	//current measurement caliberation
	#define CURRENT_RATIO       2.3196  
	#define CURRENT_OFFSET      -73.755

	//voltage readings caliberation
	#define cell6_adc_ratio     32.095
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    89.271 
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624

#elif PMB_no == 5
	//cubic approximation coefficients
	#define coef_a              4.69753
	#define coef_b              -446.67745
	#define coef_c              15910.16368
	#define coef_d              -251561.5118
	#define coef_e				1489641.436

	//current measurement caliberation 
	#define CURRENT_RATIO       4.6445
	#define CURRENT_OFFSET      -234.74

	//voltage readings caliberation
	#define cell6_adc_ratio     32.512
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    88.27  
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624

#elif PMB_no == 6
	//cubic approximation coefficients
	#define coef_a              4.69753
	#define coef_b              -446.67745
	#define coef_c              15910.16368
	#define coef_d              -251561.5118
	#define coef_e				1489641.436

	//current measurement caliberation 
	#define CURRENT_RATIO       2.1304  
	#define CURRENT_OFFSET      -152.86

	//voltage readings caliberation
	#define cell6_adc_ratio     32.324
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    86.55
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624  

#elif PMB_no == 9
	//cubic approximation coefficients
	#define coef_a              4.69753
	#define coef_b              -446.67745
	#define coef_c              15910.16368
	#define coef_d              -251561.5118
	#define coef_e				1489641.436

	//current measurement caliberation 
	#define CURRENT_RATIO       1.4157 
	#define CURRENT_OFFSET      -121.05

	//voltage readings caliberation
	#define cell6_adc_ratio     32.105
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    -121.05
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624  

#elif PMB_no == 10
	//cubic approximation coefficients
	#define coef_a              4.69753
	#define coef_b              -446.67745
	#define coef_c              15910.16368
	#define coef_d              -251561.5118
	#define coef_e				1489641.436

	//current measurement caliberation 
	#define CURRENT_RATIO       1.8842
	#define CURRENT_OFFSET      -53.25

	//voltage readings caliberation
	#define cell6_adc_ratio     32.843
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    -286.22
	#define cell5_adc_offset    0.05269355 
	#define cell4_adc_offset    -0.1076     
	#define cell3_adc_offset    -0.1076     
	#define cell2_adc_offset    0.0078684   
	#define cell1_adc_offset    0.00739624  

#endif

#endif
