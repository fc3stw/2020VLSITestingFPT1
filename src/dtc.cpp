/**********************************************************************/
/*           This is the podem test pattern generator for tdf atpg    */
/*                                                                    */
/*           Author: Pei-Chen(Jenny) Yen                              */
/*           last update : 06/04/2020                                 */
/**********************************************************************/
#include "atpg.h"
#include <cmath>

void ATPG::dynamic_test_compress(int &current_backtracks)
{
    int max_fault = 2000;
    int num_tried_fault = 0;
    fptr second_fault;
    int gen_result;
    int i;
    // vector generated by primary fault is given in the form of n+1 bits
    // the last bit represents an extra shift-in bit for LOS
    vector<int> tmp_vec = tdf_vec;
    while(true){
        tmp_vec = tdf_vec;
        // select secondary fault
        // TODO
        second_fault = get_second_fault();
        // end TODO
        // terminate dtc if no more fault to choose
        if(second_fault==nullptr) break;
        num_tried_fault++;
        if(num_tried_fault >= max_fault) break;
        // load v2
        i = 0;
        for(wptr pi : cktin){
            if(i == 0) pi->value = tmp_vec.back();
            else pi->value = tmp_vec[i-1];
            pi->set_changed();
            i++;
        }
        sim();
        // expand v2
        gen_result = tdf_podem(second_fault, current_backtracks, false);
        if(gen_result!=TRUE) continue;
        // collect v2 on tmp_vec
        i = 0;
        for(wptr pi : cktin){
            if(i==0) tmp_vec.back() = pi->value;
            else tmp_vec[i-1] = pi->value;
            i++;
        }
        // load v1
        i = 0;
        for(wptr pi : cktin){
            pi->value = tmp_vec[i];
            pi->set_changed();
            i++;
        }
        sim();
        // expand v1
        reverse_fault_type(second_fault);
        gen_result = tdf_podem_activate(second_fault, current_backtracks, false);
        reverse_fault_type(second_fault);
        if(gen_result!=TRUE) continue;
        // collect v1 on tmp_vec
        i = 0;
        for(wptr pi : cktin){
            tmp_vec[i] = pi->value;
            i++;
        }
        tdf_vec = tmp_vec;
    }
    for(fptr f: flist_undetect){
        f->dtc_tried = false;
    }
}

// TODO
ATPG::fptr ATPG::get_second_fault()
{
    int i = 0;
    int j = 0;
    bool _fail;
    vector<int> tmp_vec = tdf_vec;
    // analyze x-rate
    // double x_rate = 0.;
    // for(int b: tmp_vec){
    //     if(b==U) x_rate += 1.;
    // }
    // x_rate = x_rate/(double)tmp_vec.size();
    // double alpha = 3.;
    // double slope = 0.15;
    // double choose_prob = 100.*(0.1-slope*(1.-x_rate));
    // double choose_prob = 100.*exp(-alpha*(1.-x_rate));
    // if(choose_prob < 0.01) return nullptr;

    for(fptr fault : flist_undetect){
    	_fail = false;
        // cout<<"DTC on fault #"<<fault->index<<endl;
        if(fault->dtc_tried == true){
            // cout<<"fault already tried"<<endl;
            continue;
        }
        // int choose = rand() % 100;
        int choose = rand() % 40;
        // if(choose > choose_prob) continue;
        if(choose != 0) continue;
        fault->dtc_tried = true;

    	// cout<<"get faulty wire"<<endl;
    	wptr faulty_w = fault->get_faulty_wire();

    	// cout<<"load v1"<<endl;
    	// load v1
    	i = 0;
    	for(wptr pi : cktin){
        	pi->value = tmp_vec[i];
        	pi->set_changed();
        	i++;
    	}
    	// cout<<"sim v1"<<endl;
    	sim();
    	// cout<<"check conflict"<<endl;
    	_fail = conflict(fault->fault_type, faulty_w->value);
    	if(_fail)	continue;

    	// cout<<"load v2"<<endl;
		// load v2
		i = 0;
		for(wptr pi : cktin){
    		if(i == 0) pi->value = tmp_vec.back();
    		else pi->value = tmp_vec[i-1];
    		pi->set_changed();
    		i++;
		}
		// cout<<"sim v2"<<endl;
		sim();

		// cout<<"check conflict"<<endl;
		reverse_fault_type(fault);
		_fail = conflict(fault->fault_type, faulty_w->value);
		reverse_fault_type(fault);
		if(_fail)	continue;

		// cout<<"trace x path"<<endl;
		_fail = !trace_unknown_path(faulty_w);
		if(_fail)	continue;

        // cout<<"secondary fault selected"<<endl;
		return fault;
    }
    return nullptr;
}