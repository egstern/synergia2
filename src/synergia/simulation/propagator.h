#ifndef PROPAGATOR_H_
#define PROPAGATOR_H_

#include "synergia/simulation/stepper.h"
#include "synergia/simulation/propagate_actions.h"
#include "synergia/simulation/bunch_simulator.h"
#include "synergia/simulation/bunch_train_simulator.h"
#include "synergia/foundation/multi_diagnostics.h"
#include "synergia/utils/serialization.h"
#include "synergia/utils/logger.h"

class Propagator
{
public:
    static const std::string default_checkpoint_dir;
    static const std::string description_file_name;
    static const std::string propagator_archive_name;
    static const std::string propagator_xml_archive_name;
    static const std::string state_archive_name;
    static const std::string state_xml_archive_name;
    static const std::string log_file_name;
    static const std::string stop_file_name;
    static const std::string alt_stop_file_name;
    static const int default_checkpoint_period;
    static const int default_concurrent_io;

    struct State
    {
        Bunch_simulator * bunch_simulator_ptr;
        Bunch_train_simulator * bunch_train_simulator_ptr;
        Propagate_actions * propagate_actions_ptr;
        int num_turns;
        int first_turn;
        int max_turns;
        int verbosity;
        State(Bunch_simulator * bunch_simulator_ptr,
                Propagate_actions * propagate_actions_ptr, int num_turns,
                int first_turn, int max_turns, int verbosity);
        State(Bunch_train_simulator * bunch_train_simulator_ptr,
                Propagate_actions * propagate_actions_ptr, int num_turns,
                int first_turn, int max_turns, int verbosity);
        State()
        {
        }
        template<class Archive>
            void
            serialize(Archive & ar, const unsigned int version);
    };

private:
    Stepper_sptr stepper_sptr;
    int checkpoint_period;
    std::string checkpoint_dir;
    bool checkpoint_with_xml;
    int concurrent_io;
    bool final_checkpoint;

	void
	construct();
	void
	do_before_start(State & state, double & t, Logger & logger);
	void
	do_step(Step & step, int step_count, int num_steps, int turn, State & state,
			double & t, Logger & logger);
	void
	do_start_repetition(State & state);
	bool
	check_out_of_particles(State & state, Logger & logger);
	void
	checkpoint(State & state, Logger & logger, double & t);
	void
	do_turn_end(int turn, State & state, double & t, double t_turn0, Logger & logger);
public:
    Propagator(Stepper_sptr stepper_sptr);

    // Default constructor for serialization use only
    Propagator();

    Stepper_sptr
    get_stepper_sptr();

    void
    set_checkpoint_period(int period);

    int
    get_checkpoint_period() const;

    void
    set_checkpoint_dir(std::string const& directory_name);

    std::string const&
    get_checkpoint_dir() const;

    void
    set_checkpoint_with_xml(bool with_xml);

    bool
    get_checkpoint_with_xml() const;

    void
    set_final_checkpoint(bool final_checkpoint);

    bool
    get_final_checkpoint() const;

    void
    set_concurrent_io(int max);

    int
    get_concurrent_io() const;

    void
    propagate(State & state);

    /// jfa note: the lifetime of the pointers in state must
    ///           be managed manually
    State
    get_resume_state(std::string const& checkpoint_dir);

    void
    resume(std::string const& checkpoint_dir, bool new_num_turns, int num_turns, bool new_max_turns,
            int max_turns, bool new_verbosity, int verbosity);

    void
    propagate(Bunch_simulator & bunch_simulator, int num_turns,
            int max_turns = 0, int verbosity = 1);

    void
    propagate(Bunch_simulator & bunch_simulator,
            Propagate_actions & general_actions, int num_turns,
            int max_turns = 0, int verbosity = 1);

    void
    propagate(Bunch_train_simulator & bunch_train_simulator, int num_turns,
            int max_turns = 0, int verbosity = 1);

    void
    propagate(Bunch_train_simulator & bunch_train_simulator,
            Propagate_actions & general_actions, int num_turns,
            int max_turns = 0, int verbosity = 1);

    template<class Archive>
        void
        serialize(Archive & ar, const unsigned int version);

    ~Propagator();
};

#endif /* PROPAGATOR_H_ */
