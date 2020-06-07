/**********************************************************************/
/*           This is the podem test pattern generator for tdf atpg    */
/*                                                                    */
/*           Author: Mu-Ting (Tim) Wu                                 */
/*           last update : 06/01/2020                                 */
/**********************************************************************/
#include "atpg.h"

#define CONFLICT 2

// return type to be decided
int ATPG::gen_tdf_vector(const fptr fault, int &current_backtracks)
{
    int gen_result = FALSE;
    int i = 0;
    tdf_vec = vector<int>(cktin.size()+1, U);

    // generate v2 for equivalent SAF of TDF
    gen_result = tdf_podem(fault, current_backtracks, true);
    if (gen_result != TRUE) return gen_result;
    // collect v2
    i = 0;
    for (wptr pi : cktin){
        if(i == 0) tdf_vec.back() = pi->value;
        else tdf_vec[i-1] = pi->value;
        i++;
    }

    // load shifted v2 as initial v1
    i = 0;
    for(wptr pi : cktin){
        pi->value = tdf_vec[i];
        pi->set_changed();
        i++;
    }
    sim();

    // complete v1 to activate TDF
    reverse_fault_type(fault);
    gen_result = tdf_podem_activate(fault, current_backtracks, false);
    reverse_fault_type(fault);
    if (gen_result != TRUE) return gen_result;
    // collect v1
    i = 0;
    for (wptr pi : cktin){
        tdf_vec[i] = pi->value;
        i++;
    }

    // DTC
    if(get_compress()){
        dynamic_test_compress(current_backtracks);
    }

    // random fill
    for (int &b : tdf_vec){
        if (b == U) b = rand() % 2;
    }

    if(!get_compress()){ 
        print_vec(tdf_vec, true);
    }
    return TRUE;
}

/* generates a single pattern for a single fault */
// do not random fill
int ATPG::tdf_podem(const fptr fault, int &current_backtracks, bool clear_input)
{
    int i, ncktin;
    wptr wpi;                         // points to the PI currently being assigned
    forward_list<wptr> decision_tree; // design_tree (a LIFO stack)
    wptr wfault;
    int attempt_num = 0; // counts the number of pattern generated so far for the given fault

    /* initialize all circuit wires to unknown */
    ncktin = cktin.size();
    if(clear_input){
        for (i = 0; i < sort_wlist.size(); i++){
            sort_wlist[i]->value = U;
        }
    }

    no_of_backtracks = 0;
    find_test = false;
    no_test = false;

    mark_propagate_tree(fault->node);

    /* Fig 7 starts here */
    /* set the initial objective, assign the first PI.  Fig 7.P1 */
    switch (set_uniquely_implied_value(fault))
    {
    case TRUE: // if a  PI is assigned
        sim(); // Fig 7.3
        wfault = fault_evaluate(fault);
        if (wfault != nullptr)
            forward_imply(wfault); // propagate fault effect
        if (check_test())
            find_test = true; // if fault effect reaches PO, done. Fig 7.10
        break;
    case CONFLICT:
        no_test = true; // cannot achieve initial objective, no test
        break;
    case FALSE:
        break; //if no PI is reached, keep on backtracing. Fig 7.A
    }

    /* loop in Fig 7.ABC
   * quit the loop when either one of the three conditions is met:
   * 1. number of backtracks is equal to or larger than limit
   * 2. no_test
   * 3. already find a test pattern AND no_of_patterns meets required total_attempt_num */
    while ((no_of_backtracks < backtrack_limit) && !no_test &&
           !(find_test && (attempt_num == total_attempt_num)))
    {

        /* check if test possible.   Fig. 7.1 */
        if (wpi = test_possible(fault))
        {
            wpi->set_changed();
            /* insert a new PI into decision_tree */
            decision_tree.push_front(wpi);
        }
        else
        { // no test possible using this assignment, backtrack.

            while (!decision_tree.empty() && (wpi == nullptr))
            {
                /* if both 01 already tried, backtrack. Fig.7.7 */
                if (decision_tree.front()->is_all_assigned())
                {
                    decision_tree.front()->remove_all_assigned(); // clear the ALL_ASSIGNED flag
                    decision_tree.front()->value = U;             // do not assign 0 or 1
                    decision_tree.front()->set_changed();         // this PI has been changed
                    /* remove this PI in decision tree.  see dashed nodes in Fig 6 */
                    decision_tree.pop_front();
                }
                /* else, flip last decision, flag ALL_ASSIGNED. Fig. 7.8 */
                else
                {
                    decision_tree.front()->value = decision_tree.front()->value ^ 1; // flip last decision
                    decision_tree.front()->set_changed();                            // this PI has been changed
                    decision_tree.front()->set_all_assigned();
                    no_of_backtracks++;
                    wpi = decision_tree.front();
                }
            } // while decision tree && ! wpi
            if (wpi == nullptr)
                no_test = true; //decision tree empty,  Fig 7.9
        }                       // no test possible

        /* this again loop is to generate multiple patterns for a single fault 
 * this part is NOT in the original PODEM paper  */
    again:
        if (wpi)
        {
            sim();
            if (wfault = fault_evaluate(fault))
                forward_imply(wfault);
            if (check_test())
            {
                find_test = true;
                /* if multiple patterns per fault, print out every test cube */
                if (total_attempt_num > 1)
                {
                    if (attempt_num == 0)
                    {
                        display_fault(fault);
                    }
                    display_io();
                }
                attempt_num++; // increase pattern count for this fault

                /* keep trying more PI assignments if we want multiple patterns per fault
         * this is not in the original PODEM paper*/
                if (total_attempt_num > attempt_num)
                {
                    wpi = nullptr;
                    while (!decision_tree.empty() && (wpi == nullptr))
                    {
                        /* backtrack */
                        if (decision_tree.front()->is_all_assigned())
                        {
                            decision_tree.front()->remove_all_assigned();
                            decision_tree.front()->value = U;
                            decision_tree.front()->set_changed();
                            decision_tree.pop_front();
                        }
                        /* flip last decision */
                        else
                        {
                            decision_tree.front()->value = decision_tree.front()->value ^ 1;
                            decision_tree.front()->set_changed();
                            decision_tree.front()->set_all_assigned();
                            no_of_backtracks++;
                            wpi = decision_tree.front();
                        }
                    }
                    if (!wpi)
                        no_test = true;
                    goto again; // if we want multiple patterns per fault
                }               // if total_attempt_num > attempt_num
            }                   // if check_test()
        }                       // again
    }                           // while (three conditions)

    /* clear everything */
    for (wptr wptr_ele : decision_tree)
    {
        wptr_ele->remove_all_assigned();
    }
    decision_tree.clear();

    current_backtracks = no_of_backtracks;
    unmark_propagate_tree(fault->node);

    if (find_test)
    {
        /* normally, we want one pattern per fault */
        if (total_attempt_num == 1)
        {

            for (i = 0; i < ncktin; i++)
            {
                switch (cktin[i]->value)
                {
                case 0:
                case 1:
                    break;
                case D:
                    cktin[i]->value = 1;
                    break;
                case D_bar:
                    cktin[i]->value = 0;
                    break;
                case U:
                    cktin[i]->value = U;
                    break; // random fill U
                }
            }
            // display_io();
        }
        else
            fprintf(stdout, "\n"); // do not random fill when multiple patterns per fault
        return (TRUE);
    }
    else if (no_test)
    {
        /*fprintf(stdout,"redundant fault...\n");*/
        return (FALSE);
    }
    else
    {
        /*fprintf(stdout,"test aborted due to backtrack limit...\n");*/
        return (MAYBE);
    }
} /* end of tdfpodem */

/* generates a single pattern for a single fault */
// activate tdf
// do not random fill
int ATPG::tdf_podem_activate(const fptr fault, int &current_backtracks, bool clear_input)
{
    int i, ncktin;
    wptr wpi;                         // points to the PI currently being assigned
    forward_list<wptr> decision_tree; // design_tree (a LIFO stack)
    wptr wfault;
    int attempt_num = 0; // counts the number of pattern generated so far for the given fault

    /* initialize all circuit wires to unknown */
    ncktin = cktin.size();
    if(clear_input){
        for (i = 0; i < sort_wlist.size(); i++){
            sort_wlist[i]->value = U;
        }
    }

    no_of_backtracks = 0;
    find_test = false;
    no_test = false;

    mark_propagate_tree(fault->node);

    /* Fig 7 starts here */
    /* set the initial objective, assign the first PI.  Fig 7.P1 */
    switch (set_uniquely_implied_value(fault))
    {
    case TRUE: // if a  PI is assigned
        sim(); // Fig 7.3
        if (is_activated(fault)){
            find_test = true;
        }
        break;
    case CONFLICT:
        no_test = true; // cannot achieve initial objective, no test
        break;
    case FALSE:
        break; //if no PI is reached, keep on backtracing. Fig 7.A
    }

    /* loop in Fig 7.ABC
   * quit the loop when either one of the three conditions is met:
   * 1. number of backtracks is equal to or larger than limit
   * 2. no_test
   * 3. already find a test pattern AND no_of_patterns meets required total_attempt_num */
    while ((no_of_backtracks < backtrack_limit) && !no_test &&
           !(find_test && (attempt_num == total_attempt_num)))
    {

        /* check if test possible.   Fig. 7.1 */
        if (wpi = test_possible(fault))
        {
            wpi->set_changed();
            /* insert a new PI into decision_tree */
            decision_tree.push_front(wpi);
        }
        else
        { // no test possible using this assignment, backtrack.

            while (!decision_tree.empty() && (wpi == nullptr))
            {
                /* if both 01 already tried, backtrack. Fig.7.7 */
                if (decision_tree.front()->is_all_assigned())
                {
                    decision_tree.front()->remove_all_assigned(); // clear the ALL_ASSIGNED flag
                    decision_tree.front()->value = U;             // do not assign 0 or 1
                    decision_tree.front()->set_changed();         // this PI has been changed
                    /* remove this PI in decision tree.  see dashed nodes in Fig 6 */
                    decision_tree.pop_front();
                }
                /* else, flip last decision, flag ALL_ASSIGNED. Fig. 7.8 */
                else
                {
                    decision_tree.front()->value = decision_tree.front()->value ^ 1; // flip last decision
                    decision_tree.front()->set_changed();                            // this PI has been changed
                    decision_tree.front()->set_all_assigned();
                    no_of_backtracks++;
                    wpi = decision_tree.front();
                }
            } // while decision tree && ! wpi
            if (wpi == nullptr)
                no_test = true; //decision tree empty,  Fig 7.9
        }                       // no test possible
        sim();
        if (is_activated(fault)){
            find_test = true;
        }
    } // while (three conditions)

    /* clear everything */
    for (wptr wptr_ele : decision_tree)
    {
        wptr_ele->remove_all_assigned();
    }
    decision_tree.clear();

    current_backtracks = no_of_backtracks;
    unmark_propagate_tree(fault->node);

    if (find_test)
    {
        /* normally, we want one pattern per fault */
        if (total_attempt_num == 1)
        {

            for (i = 0; i < ncktin; i++)
            {
                switch (cktin[i]->value)
                {
                case 0:
                case 1:
                    break;
                case D:
                    cktin[i]->value = 1;
                    break;
                case D_bar:
                    cktin[i]->value = 0;
                    break;
                case U:
                    cktin[i]->value = U;
                    break; // random fill U
                }
            }
            // display_io();
        }
        else
            fprintf(stdout, "\n"); // do not random fill when multiple patterns per fault
        return (TRUE);
    }
    else if (no_test)
    {
        /*fprintf(stdout,"redundant fault...\n");*/
        return (FALSE);
    }
    else
    {
        /*fprintf(stdout,"test aborted due to backtrack limit...\n");*/
        return (MAYBE);
    }
} /* end of tdfpodem */

void ATPG::reverse_fault_type(const fptr fault)
{
    switch (fault->fault_type)
    {
    case STR:
        fault->fault_type = STF;
        break;
    case STF:
        fault->fault_type = STR;
        break;
    }
}

bool ATPG::conflict(int b1, int b2)
{
    if (b1 == b2) return false;
    else if (b1 == U || b2==U) return false;
    else return true;
}

bool ATPG::is_activated(const fptr fault)
{
    wptr faulty_wire = fault->get_faulty_wire();
    return (faulty_wire->value != U) && (faulty_wire->value != fault->fault_type);
}

void ATPG::print_vec(const vector<int> &vec, bool two_pat)
{
    int idx = 0;
    fprintf(stdout, "T\'");
    for (int val : vec)
    {
        if (idx == vec.size() - 1 && two_pat)
            fprintf(stdout, " ");
        switch (val)
        {
        case 0:
            fprintf(stdout, "0");
            break;
        case 1:
            fprintf(stdout, "1");
            break;
        case U:
            fprintf(stdout, "x");
            break;
        case D:
            fprintf(stdout, "1");
            break;
        case D_bar:
            fprintf(stdout, "0");
            break;
        }
        idx++;
    }
    fprintf(stdout, "'\n");
}
