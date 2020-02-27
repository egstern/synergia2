#include <string>
#include <iostream>
#include "diagnostics_bulk_track.h"
#include "synergia/bunch/bunch.h"
#include "synergia/utils/parallel_utils.h"
#include "synergia/utils/simple_timer.h"


Diagnostics_bulk_track::Diagnostics_bulk_track(
        int num_tracks, 
        int offset )
    : Diagnostics("diagnostis_bulk_track", true)
    , total_num_tracks(num_tracks)
    , offset(offset)
    , first_search(true)
    , diag_track_status()
    , track_coords("local_coords", 0, 0)
{
}

void
Diagnostics_bulk_track::do_update(Bunch const& bunch)
{
    auto const& ref = bunch.get_reference_particle();

    pz = ref.get_momentum();
    s = ref.get_s();
    s_n = ref.get_s_n();
    repetition = ref.get_repetition();

    simple_timer_start("diag_bulk_track_idx");

    if (first_search)
    {
        ref_charge = ref.get_charge();
        ref_mass   = ref.get_four_momentum().get_mass();
        ref_pz     = ref.get_four_momentum().get_momentum();

        auto const& comm = bunch.get_comm();

        auto local_num_tracks = decompose_1d_local(comm, total_num_tracks);
        auto local_offset = decompose_1d_local(comm, offset);

        if (local_num_tracks + local_offset > bunch.get_local_num()) 
            local_num_tracks = bunch.get_local_num() - local_offset;

        // reszie the arrays
        Kokkos::resize(track_coords, local_num_tracks, 7);
        diag_track_status.resize(local_num_tracks);

        // loop over my local tracks
        for (int idxtrk = 0; idxtrk < local_num_tracks; ++idxtrk) 
        {
            // here's one
            diag_track_status[idxtrk].index = idxtrk + local_offset;
            diag_track_status[idxtrk].particle_id = -1; 
        }

        // done with the first search
        first_search = false;
    }
    else
    {
        // update all the track index
        for (auto & dts : diag_track_status)
        {
            if (dts.index != Bunch::particle_index_null) 
                dts.index = bunch.search_particle(dts.particle_id, dts.index);
        }
    }

    simple_timer_stop("diag_bulk_track_idx");

    simple_timer_start("diag_bulk_track_part");

    // loop over my local tracks
    for (int idxtrk = 0; idxtrk < diag_track_status.size(); ++idxtrk) 
    {
        auto & dts = diag_track_status[idxtrk];

        if (dts.index != Bunch::particle_index_null)
        {
            auto coord = bunch.get_particle(dts.index);

            track_coords(idxtrk, 0) = coord(0);
            track_coords(idxtrk, 1) = coord(1);
            track_coords(idxtrk, 2) = coord(2);
            track_coords(idxtrk, 3) = coord(3);
            track_coords(idxtrk, 4) = coord(4);
            track_coords(idxtrk, 5) = coord(5);
            track_coords(idxtrk, 6) = coord(6);

            // particle id is not set in the first search
            dts.particle_id = coord(6);
        }
        else
        {
            track_coords(idxtrk, 0) = 0.0;
            track_coords(idxtrk, 1) = 0.0;
            track_coords(idxtrk, 2) = 0.0;
            track_coords(idxtrk, 3) = 0.0;
            track_coords(idxtrk, 4) = 0.0;
            track_coords(idxtrk, 5) = 0.0;
            track_coords(idxtrk, 6) = -dts.particle_id;
        }
    }

    simple_timer_stop("diag_bulk_track_part");
}

void
Diagnostics_bulk_track::do_first_write(Hdf5_file& file)
{
    file.write("charge", ref_charge);
    file.write("mass", ref_mass);
    file.write("pz", ref_pz);
}

void
Diagnostics_bulk_track::do_write(Hdf5_file& file)
{   
    scoped_simple_timer("diag_bulk_track_write");

    // write serial
    file.append_single("track_pz", pz);
    file.append_single("track_s", s);
    file.append_single("track_s_n", s_n);
    file.append_single("track_repetition", repetition);

    file.append_collective("track_coords", track_coords);
}


#if 0
void
Diagnostics_bulk_track::init_writers(Hdf5_file_sptr file_sptr)
{
    if (!have_writers) {
        Four_momentum fourp( get_bunch().get_reference_particle().get_four_momentum() );
        int chg = get_bunch().get_reference_particle().get_charge();
        file_sptr->write(chg, "charge");
        double pmass = fourp.get_mass();
        file_sptr->write(pmass, "mass");

        writer_coords = new Hdf5_serial_writer<MArray2d_ref >(file_sptr,
                                                              "track_coords");
        writer_s_n = new Hdf5_serial_writer<double >(file_sptr, "s_n");
        writer_repetition = new Hdf5_serial_writer<int >(file_sptr,
                                                         "repetition");
        writer_s = new Hdf5_serial_writer<double >(file_sptr,
                                                   "s");
        writer_pz = new Hdf5_serial_writer<double >(file_sptr, "pz");
        have_writers = true;
    }
}
#endif

#if 0
void
Diagnostics_bulk_track::receive_other_local_coords(
        std::vector<int > const& local_nums)
{
        int myrank = get_bunch().get_comm().get_rank();
        int size = get_bunch().get_comm().get_size();
        MArray2d all_coords(boost::extents[num_tracks][7]);
        int array_offset = 0;

        for (int rank = 0; rank < size; ++rank) 
        {
            int local_num = local_nums[rank];
            if (rank == myrank) 
            {
                for (int i = array_offset; i < array_offset + local_num; ++i) 
                {
                    for (int j = 0; j < 7; ++j) 
                    {
                        all_coords[i][j] = track_coords[i - array_offset][j];
                    }
                }
            } 
            else 
            {
                MPI_Status status;
                int message_size = 7 * local_num;
                MPI_Comm comm = get_bunch().get_comm().get();
                int error = MPI_Recv((void*) &all_coords[array_offset][0],
                                     message_size, MPI_DOUBLE, rank, rank, comm, &status);
                if (error != MPI_SUCCESS) {
                    throw std::runtime_error(
                                "Diagnostics_bulk_track::receive_other_local_coords: MPI_Recv failed.");
                }
                array_offset += local_num;
            }
        }

        writer_coords->append(all_coords);
}

void
Diagnostics_bulk_track::send_local_coords()
{
    if (get_bunch().get_comm().has_this_rank()){
        void * send_buffer = (void*) track_coords.origin();
        int status;
        int message_size = 7 * local_num_tracks;
        int receiver = get_write_helper().get_writer_rank();
        int rank = get_bunch().get_comm().get_rank();
        MPI_Comm comm = get_bunch().get_comm().get();
        status = MPI_Send(send_buffer, message_size, MPI_DOUBLE, receiver, rank,
                          comm);
        if (status != MPI_SUCCESS) {
            throw std::runtime_error(
                        "Diagnostics_bulk_track::send_local_coords: MPI_Send failed.");
        }
    }
}
#endif

#if 0
template<class Archive>
void
Diagnostics_bulk_track::serialize(Archive & ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Diagnostics);
    ar & BOOST_SERIALIZATION_NVP(num_tracks);
    ar & BOOST_SERIALIZATION_NVP(local_num_tracks);
    ar & BOOST_SERIALIZATION_NVP(offset);
    ar & BOOST_SERIALIZATION_NVP(local_offset);
    ar & BOOST_SERIALIZATION_NVP(have_writers);
    ar & BOOST_SERIALIZATION_NVP(first_search);
    ar & BOOST_SERIALIZATION_NVP(diag_track_status);
    ar & BOOST_SERIALIZATION_NVP(s_n);
    ar & BOOST_SERIALIZATION_NVP(writer_s_n);
    ar & BOOST_SERIALIZATION_NVP(repetition);
    ar & BOOST_SERIALIZATION_NVP(writer_repetition);
    ar & BOOST_SERIALIZATION_NVP(s);
    ar & BOOST_SERIALIZATION_NVP(writer_s);
    ar & BOOST_SERIALIZATION_NVP(pz);
    ar & BOOST_SERIALIZATION_NVP(writer_pz);
    ar & BOOST_SERIALIZATION_NVP(track_coords);
    ar & BOOST_SERIALIZATION_NVP(writer_coords);
}

template<class Archive>
void
Diagnostics_bulk_track::Track_status::serialize(Archive & ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_NVP(found) &
            BOOST_SERIALIZATION_NVP(last_index) &
            BOOST_SERIALIZATION_NVP(particle_id);
}

template
void
Diagnostics_bulk_track::Track_status::serialize<boost::archive::binary_oarchive >(
boost::archive::binary_oarchive &ar, const unsigned int version);

template
void
Diagnostics_bulk_track::Track_status::serialize<boost::archive::xml_oarchive >(
boost::archive::xml_oarchive &ar, const unsigned int version);

template
void
Diagnostics_bulk_track::Track_status::serialize<boost::archive::binary_iarchive >(
boost::archive::binary_iarchive &ar, const unsigned int version);

template
void
Diagnostics_bulk_track::Track_status::serialize<boost::archive::xml_iarchive >(
boost::archive::xml_iarchive &ar, const unsigned int version);


template
void
Diagnostics_bulk_track::serialize<boost::archive::binary_oarchive >(
boost::archive::binary_oarchive & ar, const unsigned int version);

template
void
Diagnostics_bulk_track::serialize<boost::archive::xml_oarchive >(
boost::archive::xml_oarchive & ar, const unsigned int version);

template
void
Diagnostics_bulk_track::serialize<boost::archive::binary_iarchive >(
boost::archive::binary_iarchive & ar, const unsigned int version);

template
void
Diagnostics_bulk_track::serialize<boost::archive::xml_iarchive >(
boost::archive::xml_iarchive & ar, const unsigned int version);

#endif

