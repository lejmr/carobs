package carobs;

@license(LGPL);

// Addressing:
// Endsides/Endnodes has address > 1000
// CoreNodes has address <= 1000

// Simulator expects switching time of SOA lower than processing time of SOA Manager !!! (d_s < d_p)

// CTA is not implemented because of programming difficulties and it does not 
// really grand so increase of performance only reduces time spent in buffers
//	- Might be implemented in TS overloading BubbleSort algorithm with extra buffers for each AQ 


//  AON grooming will be implemented such that soaManager sends launch time to AQ for a given #AQ and it fills up
//	 till the moment and send it to TS which truncates to CAR if needed and sends as a CAR to MAC which 
//	 sends onto a given lambda .. for current implementation must be changed the SOAentry in soaManager 
//	 to handle properly OT times and not train_start