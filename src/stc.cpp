#include <stdlib.h>
#include <time.h>
#include "atpg.h"

void ATPG::static_test_compress() {
	time_t start, end;
	start = time(NULL);	
	int total_detect_num = 0;
	int current_detect_num = 0;

	// step1 :reverse order sim and find essential test pattern
	// generate undetected fault list again
	flist_undetect.clear();
	for (auto pos = flist.cbegin(); pos != flist.cend(); ++pos) {
		fptr f = (*pos).get();
		f->detected_time = 0;
		f->detect = false;
		flist_undetect.push_front(f);	
	}

	// simulate test vectors in reverse order
 	vector<string> essential;
	vector<string> non_essential = vectors;
	bool find_essential = true;

	int essential_fault = 0;

	// fault sim non-essential patterns without fault drop
	bool essential_index[non_essential.size()];
	bool remove_index[non_essential.size()];
	for (int i = non_essential.size() - 1; i >= 0; i--) {
		essential_index[i] = false;
		bool redundant = tdfault_sim_a_vector(non_essential[i], current_detect_num, false, i);
		remove_index[i] = redundant;
	}
	// find essential pattern
	for (fptr f: flist_undetect) {
		// cout << f->index << ' ' << f->detect << ' ' << f->detected_time << ' ' << f->pattern.size()<< endl; 
		if (f->detect == TRUE ) {
			if (f->detected_time == detected_num) {
				find_essential = true;
				essential_fault++;
				for (int i = 0; i < f->pattern.size(); i++)	 
					essential_index[f->pattern[i]] = true;
			} 
		} else {
			f->detect = REDUNDANT;
		}
	}
	for (int i = non_essential.size() - 1; i >= 0; i--) {
		if (essential_index[i]) {
			// cout << non_essential[i] << endl;
			essential.push_back(non_essential[i]);
			non_essential.erase(non_essential.begin()+i);
		} else if (remove_index[i]){
			non_essential.erase(non_essential.begin()+i);
		}
	}


	// detect non-essential fault by reverse order and random order
	vectors.clear();
	vectors = non_essential;
	vectors.insert(vectors.end(), essential.begin(), essential.end());		
	
	gen_flist_undetect();
	
	// end = time(NULL);
	// cout << "time for essential fault : " << end-start << endl;
	// start = time(NULL);

	// step2 : reverse order + random order
	// reverse order
	total_detect_num = 0;
	current_detect_num = 0;
	for (int i = vectors.size() - 1; i >= 0; i--) {
		bool redundant = tdfault_sim_a_vector(vectors[i], current_detect_num);
		// total_detect_num += current_detect_num;
		// fprintf(stdout, "vector[%d] detects %d faults (%d)\n", i, current_detect_num, total_detect_num);
		
		if (redundant) {
			// remove the redundant vector
			vectors.erase(vectors.begin()+i);
		} 
	}

	// random order
	// srand(time(NULL));
	int iter = 0;
	int remove_pattern = -1;
	int converge = 0;
	int conv_limit = 10;
	while (converge < conv_limit) {
		remove_pattern = 0;
		gen_flist_undetect();

		vector<string> vectors_new;
		while (!vectors.empty()) {
			int index = rand()%vectors.size();
			bool redundant = tdfault_sim_a_vector(vectors[index], current_detect_num);
			// total_detect_num += current_detect_num;
			// fprintf(stdout, "vector[%d] detects %d faults (%d)\n", i, current_detect_num, total_detect_num);
			
			if (!redundant) {
				vectors_new.push_back(vectors[index]);
			} else {
				remove_pattern++;
			}
			vectors.erase(vectors.begin()+index);
		}		
		// cout << "iter:" << iter << " remove:"<< remove_pattern << endl;
		iter++;
		vectors = vectors_new;

		if (remove_pattern==0) converge++;
		else converge = 0;
	}

	// end = time(NULL);
	// cout << "time for ordering : " << end-start << endl;
}

void ATPG::gen_flist_undetect() {
	flist_undetect.clear();
	for (auto pos = flist.cbegin(); pos != flist.cend(); ++pos) {
		fptr f = (*pos).get();
		if (f->detect != REDUNDANT) {
			f->detected_time = 0;
			f->detect = false;
			flist_undetect.push_front(f);				
		}
	}
}