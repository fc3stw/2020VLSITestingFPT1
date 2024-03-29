/**********************************************************************/
/*           This is the podem test pattern generator for tdf atpg    */
/*                                                                    */
/*           Author: Mu-Ting (Tim) Wu                                 */
/*           last update : 06/01/2020                                 */
/**********************************************************************/

#include "atpg.h"

void ATPG::tdf_test() {
    string vec;
    int current_detect_num = 0;
    int total_detect_num = 0;
    int total_no_of_backtracks = 0;  // accumulative number of backtracks
    int current_backtracks = 0;
    int no_of_aborted_faults = 0;
    int no_of_redundant_faults = 0;
    int no_of_calls = 0;

    fptr fault_under_test = flist_undetect.front();
    fault_under_test->test_tried = true;

    /* test generation mode */
    /* Figure 5 in the PODEM paper */
    atpg_iter = 0;
    while (atpg_iter < detected_num) {
        switch (gen_tdf_vector(fault_under_test, current_backtracks)) {
            case TRUE:
                /* form a vector */
                vec.clear();
                for (int val : tdf_vec) {
                    vec.push_back(itoc(val));
                }
                // add vector into test vector list
                vectors.push_back(vec);
                
                /*by defect, we want only one pattern per fault */
                /*run a fault simulation, drop ALL detected faults */
                if (total_attempt_num == 1) {
                    tdfault_sim_a_vector(vec, current_detect_num);
                    total_detect_num += current_detect_num;
                }
                    /* If we want mutiple petterns per fault,
                     * NO fault simulation.  drop ONLY the fault under test */
                else {
                    fault_under_test->detect = TRUE;
                    /* drop fault_under_test */
                    // n-detect
                    // if the fault is detected, undo the detection count if the fault::detected_time < atpg::detected_num
                    if(fault_under_test->detect == TRUE){
                        // fault_under_test->detected_time++;
                        if(fault_under_test->detected_time < detected_num) fault_under_test->detect = FALSE;
                    }
                    if(fault_under_test->detect == TRUE){
                        flist_undetect.remove(fault_under_test);
                    }
                }
                in_vector_no++;
                break;
            case FALSE:
                fault_under_test->detect = REDUNDANT;
                flist_undetect.remove(fault_under_test);
                no_of_redundant_faults++;
                break;

            case MAYBE:
                no_of_aborted_faults++;
                // cout<<"exceed backtrack limits\n";
                break;
        }

        bool all_tried = true;
        for (fptr fptr_ele: flist_undetect) {
            if (!fptr_ele->test_tried && fptr_ele->detected_time==atpg_iter) {
            // if (!fptr_ele->test_tried && (fptr_ele->detected_time==atpg_iter || fptr_ele->detected_time+1==atpg_iter)) {
                fault_under_test = fptr_ele;
                fault_under_test->test_tried = true;
                all_tried = false;
                break;
            }
        }
        total_no_of_backtracks += current_backtracks; // accumulate number of backtracks
        no_of_calls++;
        if(flist_undetect.empty()) break;

        // retry for another iteration
        if(all_tried){
            for(fptr fptr_ele: flist_undetect){
                fptr_ele->test_tried = false;
            }
            atpg_iter++;
            fault_under_test = flist_undetect.front();
        }
    }

    display_undetect();
    // fprintf(stdout, "\n");
    // fprintf(stdout, "#number of aborted faults = %d\n", no_of_aborted_faults);
    // fprintf(stdout, "\n");
    // fprintf(stdout, "#number of redundant faults = %d\n", no_of_redundant_faults);
    // fprintf(stdout, "\n");
    // fprintf(stdout, "#number of calling podem1 = %d\n", no_of_calls);
    // fprintf(stdout, "\n");
    // fprintf(stdout, "#total number of backtracks = %d\n", total_no_of_backtracks);

    // static compression
    if (compress_flag) {
        static_test_compress();
    }
    
    // tdfsim
    total_detect_num = 0;
    flist_undetect.clear();
    flist_undetect.clear();
    flist.clear();
    generate_tdfault_list();
    transition_delay_fault_simulation(total_detect_num, false);

    // print all vectors
    int num_pis = cktin.size();
    for (int i = vectors.size() - 1; i >= 0; i--) {
        vectors[i].insert(num_pis, " ");
		fprintf(stdout, "T\'%s\'\n", vectors[i].c_str());
	}
	in_vector_no = vectors.size();

    fprintf(stdout, "\n# Result:\n");
    fprintf(stdout, "-----------------------\n");
    fprintf(stdout, "# number of test vectors = %d\n", in_vector_no);
    fprintf(stdout, "# fault coverage: %lf %\n\n", (double) total_detect_num / (double) num_of_tdf_fault * 100);

}/* end of test */
