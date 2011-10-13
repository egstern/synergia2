#ifndef STANDARD_DIAGNOSTICS_ACTIONS_H_
#define STANDARD_DIAGNOSTICS_ACTIONS_H_

#include "synergia/simulation/propagate_actions.h"
#include "synergia/bunch/diagnostics.h"

class Standard_diagnostics_actions : public Propagate_actions
{
private:
    std::list<Diagnostics_sptr > per_turns, per_steps;
public:
    Standard_diagnostics_actions();
    void
    add_per_turn(Diagnostics_sptr diagnostics_sptr);
    void
    add_per_step(Diagnostics_sptr diagnostics_sptr);
    void
    update_and_write_all(std::list<Diagnostics_sptr > & diag_list);
    virtual void
    first_action(Stepper & stepper, Bunch & bunch);
    virtual void
    turn_end_action(Stepper & stepper, Bunch & bunch, int turn_num);
    virtual void
    step_end_action(Stepper & stepper, Step & step, Bunch & bunch,
            int turn_num, int step_num);
    virtual
    ~Standard_diagnostics_actions();

};
typedef boost::shared_ptr<Standard_diagnostics_actions > Standard_diagnostics_actions_sptr;
#endif /* STANDARD_DIAGNOSTICS_ACTIONS_H_ */
