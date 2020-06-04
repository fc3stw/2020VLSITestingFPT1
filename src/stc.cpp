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
	// OPTION1 : x-fill
	srand(time(NULL));
	int total_detect_num = 0;
	int current_detect_num = 0;
	for (int i = vectors.size() - 1; i >= 0; i--) {
		bool redundant = tdfault_sim_a_vector(vectors[i], current_detect_num);
		total_detect_num += current_detect_num;
		// fprintf(stdout, "vector[%d] detects %d faults (%d)\n", i, current_detect_num, total_detect_num);
		
		if (redundant) {
			// remove the redundant vectors
			vectors.erase(vectors.begin()+i);
		} else {
			cout << vectors[i] << endl;
			// print vector
			fprintf(stdout, "T\'");
			fprintf(stdout, vectors[i].c_str());
			fprintf(stdout, "'\n");
		}
	}

	// OPTION2 : find essential test pattern
}