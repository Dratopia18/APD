#include <mpi.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <algorithm>
#include <cstring>

#define TRACKER_RANK 0
#define MAX_FILES 10
#define MAX_FILENAME 15
#define HASH_SIZE 32
#define MAX_CHUNKS 100

#define UPDATE_INTERVAL 10
#define MSG_REQUEST_SWARM 1
#define MSG_SEGMENT_REQUEST 2
#define MSG_SEGMENT_RESPONSE 3
#define MSG_FILE_COMPLETED 4
#define MSG_ALL_COMPLETED 5

struct file {
    std::string name;
    int num_segments;
    std::vector<std::string> segments;
    std::unordered_set<int> downloaded_segments;
};

struct ThreadData {
    int rank;
    std::vector<file> owned_files;
    std::vector<std::string> wanted_files;
    std::unordered_map<std::string, file> downloaded_files;
    bool should_exit;
    pthread_mutex_t mutex;
};

void read_input_file(int rank, ThreadData* data) {
    std::stringstream ss;
    ss << "in" << rank << ".txt";
    std::ifstream input_file(ss.str());

    int num_files_owned;
    input_file >> num_files_owned;

    for (int i = 0; i < num_files_owned; i++) {
        file f;
        input_file >> f.name >> f.num_segments;

        for (int j = 0; j < f.num_segments; j++) {
            std::string hash;
            input_file >> hash;
            f.segments.push_back(hash);
            f.downloaded_segments.insert(j);
        }
        data->owned_files.push_back(f);
    }

    int num_files_wanted;
    input_file >> num_files_wanted;

    for (int i = 0; i < num_files_wanted; i++) {
        std::string file_name;
        input_file >> file_name;
        data->wanted_files.push_back(file_name);
    }

    input_file.close();
}

void save_downloaded_files(int rank, const std::string& file_name, const file& f) {
    std::stringstream ss;
    ss << "client" << rank << "_" << file_name;
    std::ofstream output_file(ss.str());

    for (const std::string& hash : f.segments) {
        output_file << hash << std::endl;
    }
    output_file.close();
}

void *download_thread_func(void *arg)
{
    ThreadData* data = static_cast<ThreadData*>(arg);
    MPI_Status status;

    int num_files = data->owned_files.size();
    MPI_Send( &num_files , 1 , MPI_INT , TRACKER_RANK , 0 , MPI_COMM_WORLD);

    for (const file& f : data->owned_files) {
        char file_name[MAX_FILENAME];
        std::strcpy(file_name, f.name.c_str());
        MPI_Send(file_name, MAX_FILENAME, MPI_CHAR, TRACKER_RANK, 0, MPI_COMM_WORLD);
        MPI_Send(&f.num_segments, 1, MPI_INT, TRACKER_RANK, 0, MPI_COMM_WORLD);

        for (const std::string& hash : f.segments) {
            char segment_hash[HASH_SIZE + 1];
            std::strcpy(segment_hash, hash.c_str());
            MPI_Send(segment_hash, HASH_SIZE + 1, MPI_CHAR, TRACKER_RANK, 0, MPI_COMM_WORLD);
        }
    }

    int segments_downloaded = 0;

    for (const std::string& wanted_file : data->wanted_files) {
        file current_file;
        current_file.name = wanted_file;

        MPI_Send(wanted_file.c_str(), MAX_FILENAME, MPI_CHAR, TRACKER_RANK, MSG_REQUEST_SWARM, MPI_COMM_WORLD);

        int peer_count;
        MPI_Recv(&peer_count, 1, MPI_INT, TRACKER_RANK, MSG_REQUEST_SWARM, MPI_COMM_WORLD, &status);

        if (peer_count == 0) continue;

        std::vector<int> peers(peer_count);
        MPI_Recv(peers.data(), peer_count, MPI_INT, TRACKER_RANK, MSG_REQUEST_SWARM, MPI_COMM_WORLD, &status);

        int selected_peer = peers[rand() % peers.size()];

        char request[15] = "CHUNK_REQUEST";
        MPI_Send(request, 15, MPI_CHAR, selected_peer, MSG_SEGMENT_REQUEST, MPI_COMM_WORLD);
        MPI_Send(wanted_file.c_str(), MAX_FILENAME, MPI_CHAR, selected_peer, MSG_SEGMENT_REQUEST, MPI_COMM_WORLD);
        int metadata_request = -1;
        MPI_Send(&metadata_request, 1, MPI_INT, selected_peer, MSG_SEGMENT_REQUEST, MPI_COMM_WORLD);

        MPI_Recv(&current_file.num_segments, 1, MPI_INT, selected_peer, MSG_SEGMENT_RESPONSE, MPI_COMM_WORLD, &status);

        for (int i = 0; i < current_file.num_segments; i++) {
            selected_peer = peers[rand() % peers.size()];

            std::strcpy(request, "CHUNK_REQUEST");
            MPI_Send(request, 15, MPI_CHAR, selected_peer, MSG_SEGMENT_REQUEST, MPI_COMM_WORLD);
            MPI_Send(wanted_file.c_str(), MAX_FILENAME, MPI_CHAR, selected_peer, MSG_SEGMENT_REQUEST, MPI_COMM_WORLD);
            MPI_Send(&i, 1, MPI_INT, selected_peer, MSG_SEGMENT_REQUEST, MPI_COMM_WORLD);

            char segment_hash[HASH_SIZE + 1] = {0};
            MPI_Recv(segment_hash, HASH_SIZE + 1, MPI_CHAR, selected_peer, MSG_SEGMENT_RESPONSE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            current_file.segments.push_back(segment_hash);
            current_file.downloaded_segments.insert(i);

            segments_downloaded++;
            if (segments_downloaded % UPDATE_INTERVAL == 0) {
                MPI_Send(wanted_file.c_str(), MAX_FILENAME, MPI_CHAR, TRACKER_RANK, MSG_REQUEST_SWARM, MPI_COMM_WORLD);
                MPI_Recv(&peer_count, 1, MPI_INT, TRACKER_RANK, MSG_REQUEST_SWARM, MPI_COMM_WORLD, &status);

                peers.resize(peer_count);
                MPI_Recv(peers.data(), peer_count, MPI_INT, TRACKER_RANK, MSG_REQUEST_SWARM, MPI_COMM_WORLD, &status);
            }
        }

        save_downloaded_files(data->rank, wanted_file, current_file);

        pthread_mutex_lock(&data->mutex);
        data->downloaded_files[wanted_file] = current_file;
        pthread_mutex_unlock(&data->mutex);

        MPI_Send(wanted_file.c_str(), MAX_FILENAME, MPI_CHAR, TRACKER_RANK, MSG_FILE_COMPLETED, MPI_COMM_WORLD);
    }

    char ready[15] = "READY";
    MPI_Send(ready, 15, MPI_CHAR, TRACKER_RANK, MSG_ALL_COMPLETED, MPI_COMM_WORLD);
    MPI_Recv(nullptr, 0, MPI_CHAR, TRACKER_RANK, MSG_ALL_COMPLETED, MPI_COMM_WORLD, &status);

    pthread_mutex_lock(&data->mutex);
    data->should_exit = true;
    pthread_mutex_unlock(&data->mutex);

    return nullptr;
}

void *upload_thread_func(void *arg)
{
    ThreadData* data = static_cast<ThreadData*>(arg);
    MPI_Status status;

    while (true) {
        char request[15];
        MPI_Recv(request, 15, MPI_CHAR, MPI_ANY_SOURCE, MSG_SEGMENT_REQUEST, MPI_COMM_WORLD, &status);

        if (strcmp(request, "CHUNK_REQUEST") == 0) {
            char filename[MAX_FILENAME];
            int chunk_index;

            MPI_Recv(filename, MAX_FILENAME, MPI_CHAR, status.MPI_SOURCE, MSG_SEGMENT_REQUEST, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&chunk_index, 1, MPI_INT, status.MPI_SOURCE, MSG_SEGMENT_REQUEST, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            std::string file_str(filename);

            pthread_mutex_lock(&data->mutex);
            const file* requested_file = nullptr;

            for (const auto& file : data->owned_files) {
                if (file.name == file_str) {
                    requested_file = &file;
                    break;
                }
            }

            if (!requested_file && data->downloaded_files.count(file_str) > 0) {
                requested_file = &data->downloaded_files[file_str];
            }

            if (requested_file) {
                if (chunk_index == -1) {
                    MPI_Send(&requested_file->num_segments, 1, MPI_INT, status.MPI_SOURCE, MSG_SEGMENT_RESPONSE, MPI_COMM_WORLD);
                } else if (static_cast<size_t>(chunk_index) < requested_file->segments.size()) {
                    const std::string& hash = requested_file->segments[chunk_index];
                    char hash_buffer[HASH_SIZE + 1] = {0};
                    std::strncpy(hash_buffer, hash.c_str(), HASH_SIZE);
                    MPI_Send(hash_buffer, HASH_SIZE + 1, MPI_CHAR, status.MPI_SOURCE, MSG_SEGMENT_RESPONSE, MPI_COMM_WORLD);
                }
            } else {
                if (chunk_index == -1) {
                    int zero = 0;
                    MPI_Send(&zero, 1, MPI_INT, status.MPI_SOURCE, MSG_SEGMENT_RESPONSE, MPI_COMM_WORLD);
                } else {
                    char dummy[HASH_SIZE + 1] = {0};
                    MPI_Send(dummy, HASH_SIZE + 1, MPI_CHAR, status.MPI_SOURCE, MSG_SEGMENT_RESPONSE, MPI_COMM_WORLD);
                }
            }

            pthread_mutex_unlock(&data->mutex);
        }

        if (strcmp(request, "DONE") == 0) {
            pthread_mutex_lock(&data->mutex);
            data->should_exit = true;
            pthread_mutex_unlock(&data->mutex);
            break;
        }
    }

    return nullptr;
}

void tracker(int numtasks, int rank) {
    std::unordered_map<std::string, std::unordered_set<int>> file_swarm;
    std::unordered_set<int> completed_clients;

    for (int i = 1; i < numtasks; i++) {
        int num_files_owned;
        MPI_Recv(&num_files_owned, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int j = 0; j < num_files_owned; j++) {
            char filename[MAX_FILENAME];
            MPI_Recv(filename, MAX_FILENAME, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            file_swarm[filename].insert(i);

            int num_segments;
            MPI_Recv(&num_segments, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int k = 0; k < num_segments; k++) {
                char segment_hash[HASH_SIZE + 1];
                MPI_Recv(segment_hash, HASH_SIZE + 1, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }

    for (int i = 1; i < numtasks; i++) {
        MPI_Send(nullptr, 0, MPI_CHAR, i, 0, MPI_COMM_WORLD);
    }

    while (static_cast<int>(completed_clients.size()) < numtasks - 1) {
        MPI_Status status;
        char filename[MAX_FILENAME];
        MPI_Recv(filename, MAX_FILENAME, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch (status.MPI_TAG) {
            case MSG_REQUEST_SWARM: {
                std::vector<int> peers(file_swarm[filename].begin(), file_swarm[filename].end());
                int peer_count = peers.size();
                MPI_Send(&peer_count, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD);
                if (peer_count > 0) {
                    MPI_Send(peers.data(), peer_count, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD);
                }
                break;
            }
            case MSG_FILE_COMPLETED: {
                file_swarm[filename].insert(status.MPI_SOURCE);
                break;
            }
            case MSG_ALL_COMPLETED: {
                completed_clients.insert(status.MPI_SOURCE);

                if (static_cast<int>(completed_clients.size()) == numtasks - 1) {
                    char done[15] = "DONE";
                    for (int i = 1; i < numtasks; i++) {
                        MPI_Send(done, 15, MPI_CHAR, i, MSG_SEGMENT_REQUEST, MPI_COMM_WORLD);
                        MPI_Send(nullptr, 0, MPI_CHAR, i, MSG_ALL_COMPLETED, MPI_COMM_WORLD);
                    }
                    break;
                }
                break;
            }
        }
    }
}

void peer(int numtasks, int rank) {
    ThreadData data;
    data.rank = rank;
    data.should_exit = false;
    pthread_mutex_init(&data.mutex, nullptr);

    read_input_file(rank, &data);

    pthread_t download_thread;
    pthread_t upload_thread;
    void *status;
    int r;

    r = pthread_create(&download_thread, nullptr, download_thread_func, (void *) &data.rank);
    if (r) {
        printf("Eroare la crearea thread-ului de download\n");
        exit(-1);
    }

    r = pthread_create(&upload_thread, nullptr, upload_thread_func, (void *) &data.rank);
    if (r) {
        printf("Eroare la crearea thread-ului de upload\n");
        exit(-1);
    }

    r = pthread_join(download_thread, &status);
    if (r) {
        printf("Eroare la asteptarea thread-ului de download\n");
        exit(-1);
    }

    r = pthread_join(upload_thread, &status);
    if (r) {
        printf("Eroare la asteptarea thread-ului de upload\n");
        exit(-1);
    }

    pthread_mutex_destroy(&data.mutex);
}
 
int main (int argc, char *argv[]) {
    int numtasks, rank;
 
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided < MPI_THREAD_MULTIPLE) {
        fprintf(stderr, "MPI nu are suport pentru multi-threading\n");
        exit(-1);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == TRACKER_RANK) {
        tracker(numtasks, rank);
    } else {
        peer(numtasks, rank);
    }

    MPI_Finalize();
}
