#ifndef PROPAGATE_ACTIONS_H_
#define PROPAGATE_ACTIONS_H_

//#include "synergia/simulation/stepper.h"
#include "synergia/bunch/bunch.h"
#include "synergia/bunch/bunch_train.h"
#include "synergia/utils/logger.h"


class Stepper;
class Step;
class Operator;

class Propagate_actions
{ 
public:
    Propagate_actions(); 
    Propagate_actions(std::string const& type);
    std::string const&
    get_type() const;        
    virtual void
    first_action(Stepper & stepper, Bunch & bunch);
    virtual void
    first_action(Stepper & stepper, Bunch_train & bunch_train);
    virtual void
    turn_end_action(Stepper & stepper, Bunch & bunch, int turn_num);
    virtual void
    turn_end_action(Stepper & stepper, Bunch_train & bunch_train, int turn_num);
    virtual void
    step_end_action(Stepper & stepper, Step & step, Bunch & bunch,
            int turn_num, int step_num);
    virtual void
    step_end_action(Stepper & stepper, Step & step, Bunch_train & bunch_train,
            int turn_num, int step_num);             
    virtual void
    before_resume_action(Stepper & stepper, Bunch & bunch);
    virtual void
    before_resume_action(Stepper & stepper, Bunch_train & bunch_train);
    virtual void
    operator_begin_action(Stepper & stepper, Step & step, Operator & op, int step_num, int turn_num, 
                   int bunch_num);  
   

    template<class Archive>
    void
    serialize(Archive & ar, const unsigned int version)
    {
    }

    virtual
    ~Propagate_actions() = default;
};

typedef boost::shared_ptr<Propagate_actions > Propagate_actions_sptr; // syndoc:include

#endif /* PROPAGATE_ACTIONS_H_ */
