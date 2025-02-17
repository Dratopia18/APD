#include <pthread.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class WordEntry {
public:
    std::string word;
    std::vector<int> file_ids;

    WordEntry() = default;
    explicit WordEntry(const std::string& w) : word(w) {}

    void addFile(int file_id) {
        if (std::find(file_ids.begin(), file_ids.end(), file_id) == file_ids.end()) {
            file_ids.push_back(file_id);
        }
    }
};

class MapperResult {
public:
    std::vector<WordEntry> entries;
};

class SharedState {
public:
    std::vector<std::string> input_files;
    std::vector<std::vector<std::string>> file_contents;
    std::vector<MapperResult> mapper_results;
    pthread_mutex_t mutex;
    pthread_barrier_t barrier;

    explicit SharedState(size_t num_mappers, size_t num_reducers) : mapper_results(num_mappers) {
        pthread_mutex_init(&mutex, NULL);
        pthread_barrier_init(&barrier, NULL, num_mappers + num_reducers);
    }

    ~SharedState() {
        pthread_mutex_destroy(&mutex);
        pthread_barrier_destroy(&barrier);
    }
};

struct ThreadArg {
    SharedState* state;
    int thread_id;
    int num_threads;

    ThreadArg(SharedState* s, int id, int nt) : state(s), thread_id(id), num_threads(nt) {}
};

std::string cleanWord(const std::string& word) {
    std::string cleaned;
    size_t word_end = word.length();

    while (word_end > 0 && std::isdigit(word[word_end - 1])) {
        word_end--;
    }

    for (size_t i = 0; i < word_end; ++i) {
        if (std::isalpha(word[i])) {
            cleaned += std::tolower(word[i]);
        }
    }

    return cleaned;
}

void addWordtoResults(MapperResult& result, const std::string& word, int file_id) {
    bool found = false;
    for (size_t i = 0; i < result.entries.size(); i++) {
        if (result.entries[i].word == word) {
            result.entries[i].addFile(file_id);
            found = true;
            break;
        }
    }

    if (!found) {
        WordEntry entry(std::move(word));
        entry.addFile(file_id);
        result.entries.push_back(std::move(entry));
    }
}

void* mapper(void* arg) {
    ThreadArg* thread_arg = static_cast<ThreadArg*>(arg);
    SharedState* state = thread_arg->state;
    int mapper_id = thread_arg->thread_id;
    int num_mappers = thread_arg->num_threads;

    MapperResult local_result;

    for (size_t i = mapper_id; i < state->file_contents.size(); i += num_mappers) {
        const auto& lines = state->file_contents[i];
        for (const auto& line : lines) {
            const char* ptr = line.c_str();
            const char* end = ptr + line.length();

            while (ptr < end) {
                while (ptr < end && std::isspace(*ptr)) {
                    ++ptr;
                }

                const char* word_start = ptr;

                while (ptr < end && !std::isspace(*ptr)) {
                    ++ptr;
                }

                if (word_start < ptr) {
                    std::string word(word_start, ptr - word_start);
                    word = cleanWord(word);
                    if (!word.empty()) {
                        addWordtoResults(local_result, word, static_cast<int>(i) + 1);
                    }
                }
            } 
        }
    }

    pthread_mutex_lock(&state->mutex);
    state->mapper_results[mapper_id] = std::move(local_result);
    pthread_mutex_unlock(&state->mutex);

    pthread_barrier_wait(&state->barrier);
    return NULL;
}

void* reducer(void* arg) {
    ThreadArg* thread_arg = static_cast<ThreadArg*>(arg);
    SharedState* state = thread_arg->state;
    int reducer_id = thread_arg->thread_id;
    int num_reducers = thread_arg->num_threads;

    pthread_barrier_wait(&state->barrier);

    for (char c = 'a'; c <= 'z'; c++) {
        if ((c - 'a') % num_reducers == reducer_id) {
            std::string filename(1, c);
            filename += ".txt";
            std::ofstream out(filename);
            if (!out) {
                continue;
            }

            std::vector<WordEntry> combined_entries;
            combined_entries.reserve(1000);

            for (const auto& mapper_result : state->mapper_results) {
                for (const auto& entry : mapper_result.entries) {
                    if (!entry.word.empty() && entry.word[0] == c) {
                        combined_entries.push_back(entry);
                    }
                }
            }

            std::sort(combined_entries.begin(), combined_entries.end(), [](const WordEntry& a, const WordEntry& b) {
                return a.word < b.word;
            });

            std::vector<WordEntry> merged;
            merged.reserve(combined_entries.size());

            for (size_t i = 0; i < combined_entries.size(); ) {
                WordEntry merged_entry = combined_entries[i];
                size_t j = i + 1;
                while (j < combined_entries.size() && combined_entries[j].word == merged_entry.word) {
                    for (int file_id : combined_entries[j].file_ids) {
                        merged_entry.addFile(file_id);
                    }
                    j++;
                }
                merged.push_back(std::move(merged_entry));
                i = j;
            }

            std::sort(merged.begin(), merged.end(), [](const WordEntry& a, const WordEntry& b) {
                if (a.file_ids.size() != b.file_ids.size()) {
                    return a.file_ids.size() > b.file_ids.size();
                }
                return a.word < b.word;
            });

            for (const auto& entry : merged) {
                std::vector<int> sorted_ids = entry.file_ids;
                std::sort(sorted_ids.begin(), sorted_ids.end());

                out << entry.word << ":[";
                for (size_t i = 0; i < sorted_ids.size(); ++i) {
                    out << sorted_ids[i];
                    if (i < sorted_ids.size() - 1) out << " ";
                }
                out << "]\n";
            }
            out.close();
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <num_mappers> <num_reducers> <input_file>\n";
        return 1;
    }

    int num_mappers = std::stoi(argv[1]);
    int num_reducers = std::stoi(argv[2]);

    std::ifstream input(argv[3]);
    if (!input) {
        std::cout << "Cannot open input file\n";
        return 1;
    }

    SharedState state(num_mappers, num_reducers);

    int num_files;
    input >> num_files;
    state.input_files.reserve(num_files);
    state.file_contents.reserve(num_files);

    std::string filename;
    for (int i = 0; i < num_files; i++) {
        input >> filename;
        state.input_files.push_back(filename);

        std::ifstream file(filename);
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        state.file_contents.push_back(std::move(lines));
    }
    std::vector<pthread_t> threads(num_mappers + num_reducers);
    std::vector<ThreadArg> thread_args(num_mappers + num_reducers, ThreadArg(&state, 0, 0));

    for (int i = 0; i < num_mappers; i++) {
        thread_args[i] = ThreadArg(&state, i, num_mappers);
        pthread_create(&threads[i], NULL, mapper, &thread_args[i]);
    }

    for (int i = 0; i < num_reducers; i++) {
        thread_args[num_mappers + i] = ThreadArg(&state, i, num_reducers);
        pthread_create(&threads[num_mappers + i], NULL, reducer, &thread_args[num_mappers + i]);
    }

    for (int i = 0; i < num_mappers + num_reducers; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
