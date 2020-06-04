#include <stdlib.h>
#include <time.h>
#include "atpg.h"

void ATPG::static_test_compress() {
	// generate undetected fault list again
	flist_undetect.clear();
	for (auto pos = flist.cbegin(); pos != flist.cend(); ++pos) {
		fptr f = (*pos).get();
		f->detected_time = 0;
		f->detect = false;
		flist_undetect.push_front(f);	
	}
	
	// simulate test vectors in reverse order
	// OPTION1 : reverse order + random order
	// reverse order
	int total_detect_num = 0;
	int current_detect_num = 0;
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
	srand(time(NULL));
	int iter = 0;
	int remove_pattern = -1; 
	while (remove_pattern!=0) {
		remove_pattern = 0;
		flist_undetect.clear();
		for (auto pos = flist.cbegin(); pos != flist.cend(); ++pos) {
			fptr f = (*pos).get();
			f->detected_time = 0;
			f->detect = false;
			flist_undetect.push_front(f);	
		}

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
	}
	
	// print vector
	for (int i = vectors.size() - 1; i >= 0; i--) {
		fprintf(stdout, "T\'");
		fprintf(stdout, vectors[i].c_str());
		fprintf(stdout, "'\n");
	}
	in_vector_no = vectors.size();

	// OPTION2 : find essential test pattern
}