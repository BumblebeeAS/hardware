
// Calibration values for all the different boards
#ifndef CALIBRATION_H
#define CALIBRATION_H

//maximum battery capacity
#define BATTERY_CAPACITY 		36000000

#define PMB_no					9

#if PMB_no == 1
	//cubic approximation coefficients
	#define coef_a              4.69753
	#define coef_b              -446.67745
	#define coef_c              15910.16368
	#define coef_d              -251561.5118
	#define coef_e				1489641.436

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
	#define coef_a              4.69753
	#define coef_b              -446.67745
	#define coef_c              15910.16368
	#define coef_d              -251561.5118
	#define coef_e				1489641.436

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
	#define coef_a              4.42455
	#define coef_b              -420.46384
	#define coef_c              14966.72708
	#define coef_d              -236478.77089
	#define coef_e				1399267.88461

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
	#define coef_a              4.69753
	#define coef_b              -446.67745
	#define coef_c              15910.16368
	#define coef_d              -251561.5118
	#define coef_e				1489641.436

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
	#define coef_a              4.69753
	#define coef_b              -446.67745
	#define coef_c              15910.16368
	#define coef_d              -251561.5118
	#define coef_e				1489641.436

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

	#define cell6_adc_offset    55.385  
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
	#define CURRENT_RATIO       1.7759  
	#define CURRENT_OFFSET      303.9

	//voltage readings caliberation
	#define cell6_adc_ratio     32.481
	#define cell5_adc_ratio     0.197181814  
	#define cell4_adc_ratio     0.244377275  
	#define cell3_adc_ratio     0.325966493  
	#define cell2_adc_ratio     0.497397968  
	#define cell1_adc_ratio     0.993694855  

	#define cell6_adc_offset    -138.87
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
