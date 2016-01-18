/*
 FILE: Jacobson Algorithm 
 */
#define initial_rto 250.0 //in ms

static float SRTT, RTTVAR;
static float g = .125, h = .25;
static double RTT = 250;

struct timeval timehash[1000] ;

void set_time(int seq_num){
    struct timeval tv;
	gettimeofday(&tv,NULL);
	timehash[seq_num] = tv ;
}

double calc_rtt(struct timeval ack_time, struct timeval packet_time){
  double ack_time_in_mill = ((ack_time.tv_sec) * 1000) + ((ack_time.tv_usec) / 1000) ;
  double packet_time_in_mill = ((packet_time.tv_sec) * 1000) + ((packet_time.tv_usec) / 1000) ;
  RTT = (ack_time_in_mill - packet_time_in_mill);
  return RTT;
}

void initialize(float initial_RTT){

	SRTT = initial_RTT;
	RTTVAR = initial_RTT / 2;
	printf("\nDEFAULT: SRTT:%.1f \t RTTVAR: %.1f \n",SRTT,RTTVAR);
}

float computeRTO(){
	float err, RTO;
	
	if(RTT == -1.0){
		return initial_rto;
	}
	err = RTT - SRTT;
	SRTT = SRTT + g * err;
	RTTVAR = RTTVAR + h * (abs(err) - RTTVAR);
	RTO = SRTT + 4* RTTVAR;
	
	printf("***UPDATE:- RTT:%.1f \t RTO:%.1f \tSRTT:%.1f \t RTTVAR: %.1f \n",RTT, RTO, SRTT,RTTVAR);
			
	return RTO;
}
