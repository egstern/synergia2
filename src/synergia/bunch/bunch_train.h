#ifndef BUNCH_TRAIN_H_
#define BUNCH_TRAIN_H_

#include "synergia/bunch/bunch.h"

class Bunch_train
{

private:

    std::vector<Bunch> bunches;
    std::vector<double> spacings;

    Commxx comm;
    int index;

    int num_bunches;
    int num_buckets;

    /// counts and offsets are needed for impedance, counts.size()=offsets.size()=num_procs
    /// they are meaningfull only on the local rank=0 of every bunch communicator 
    /// for example:  3 bunches on 5 processors: proc 2 and 4 has local rank different form zero
    ///              (count[0]=1, offset[0]=0), (count[1]=1, offset[3]=1), (count[3]=1, offset[3]=2)
    /// for example:  5 bunches on 3 processors: 
    ///                (count[0]=1, offset[0]=0), (count[1]=2, offset[3]=1), (count[2]=2, offset[2]=3)
    std::vector< int> proc_counts_imped; 
    std::vector< int> proc_offsets_imped; 

    void set_bucket_indices();
    void find_parent_comm();
    void calculates_counts_and_offsets_for_impedance();

public:

    //Bunch_train(Bunches const& bunches, double spacing);
    //Bunch_train(Bunches const& bunches, std::vector<double > const& spacings);

    Bunch_train(
            Reference_particle const & ref,
            size_t num_bunches,
            size_t num_particles_per_bunch,
            double num_real_particles_per_bunch,
            double spacing,
            Commxx const & comm = Commxx(),
            int index = 0 );

    // index of the train (0 or 1)
    int get_index() const
    { return index; }

    // number of bunches on this rank (size of the bunch array)
    size_t get_size() const
    { return bunches.size(); }

    // total number of bunches in the train. it is not always
    // the same as the size of the bunch array
    int get_num_bunches() const
    { return num_bunches; }

    // number of buckets. it is not necessary for the bunches to
    // occupy consecutive buckets, so there might be some empty
    // buckets in-between bunches
    // for now it is always the same as the number of bunches
    int get_num_buckets() const
    { return num_buckets; }

    Bunch & operator[](size_t idx)
    { return bunches[idx]; }

    Bunch const & operator[](size_t idx) const
    { return bunches[idx]; }

    std::vector<Bunch> & get_bunches()
    { return bunches; }

    std::vector<Bunch> const & get_bunches() const
    { return bunches; }

    std::vector<double > & get_spacings();

    // update the total particle number for all bunches in the bunch train
    // note that calling each bunch's update_total_num() wont do the actual
    // work if the caller's rank is not part of the bunch's communicator.
    // on the other hand, calling bunch_train's update_bunch_total_num() 
    // gurantees that all bunches are updated for all ranks
    void update_bunch_total_num();
    
    std::vector< int> & get_proc_counts_for_impedance();
    std::vector< int> & get_proc_offsets_for_impedance();

#if 0
    template<class Archive>
        void
        serialize(Archive & ar, const unsigned int version);
#endif
};

#endif /* BUNCH_TRAIN_H_ */
