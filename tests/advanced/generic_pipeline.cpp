#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include <aff3ct-core.hpp>
using namespace aff3ct;

// Error message variable
std::stringstream message;

std::ifstream::pos_type
filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

bool
compare_files(const std::string& filename1, const std::string& filename2)
{
    std::ifstream file1(filename1, std::ifstream::ate | std::ifstream::binary); // open file at the end
    std::ifstream file2(filename2, std::ifstream::ate | std::ifstream::binary); // open file at the end
    const std::ifstream::pos_type fileSize = file1.tellg();

    if (fileSize != file2.tellg()) return false; // different file size

    file1.seekg(0); // rewind
    file2.seekg(0); // rewind

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin2(file2);

    return std::equal(begin1, std::istreambuf_iterator<char>(), begin2); // Second argument is end-of-range iterator
}

// Functions to parse arguments
void
parse_int_string(std::string& vector_param, std::vector<size_t>& vector)
{
    for (size_t i = 0; i < vector_param.size(); ++i)
        if (vector_param[i] != '(' && vector_param[i] != ',' && vector_param[i] != ')')
            vector.push_back(atoi(&vector_param[i]));
}

void
parse_sck_type_tsk(std::string& sck_type_tsk_param, std::vector<std::vector<std::string>>& sck_type_tsk)
{
    size_t i = 0;
    size_t sta = 0;
    std::string tmp;
    while (i < sck_type_tsk_param.size())
    {
        if (sck_type_tsk_param[i] == '(' && sck_type_tsk_param[i + 1] != '(')
        {
            sck_type_tsk.push_back({});
            tmp.clear();
        }
        else if (sck_type_tsk_param[i] == ',' && sck_type_tsk_param[i + 1] != '(')
        {
            sck_type_tsk[sta].push_back(tmp);
            tmp.clear();
        }
        else if (sck_type_tsk_param[i] == ')')
        {
            if (sck_type_tsk_param[i + 1] != ')')
            {
                sck_type_tsk[sta].push_back(tmp);
                tmp.clear();
                sta++;
            }
        }
        else
            tmp.push_back(sck_type_tsk_param[i]);

        i++;
    }
}

void
parse_sck_type_sta(std::string& sck_type_sta_param, std::vector<std::string>& sck_type_sta)
{
    size_t i = 0;
    std::string tmp;
    while (i < sck_type_sta_param.size())
    {
        if (sck_type_sta_param[i] == '(')
            tmp.clear();
        else if (sck_type_sta_param[i] == ',' || sck_type_sta_param[i] == ')')
        {
            sck_type_sta.push_back(tmp);
            tmp.clear();
        }
        else
            tmp.push_back(sck_type_sta_param[i]);
        i++;
    }
}

int
main(int argc, char** argv)
{
    tools::Signal_handler::init();

    option longopts[] = { { "n-threads", required_argument, NULL, 't' },
                          { "n-inter-frames", required_argument, NULL, 'f' },
                          { "sleep-time", required_argument, NULL, 's' },
                          { "data-length", required_argument, NULL, 'd' },
                          { "buffer-size", required_argument, NULL, 'u' },
                          { "dot-filepath", required_argument, NULL, 'o' },
                          { "in-filepath", required_argument, NULL, 'i' },
                          { "out-filepath", required_argument, NULL, 'j' },
                          { "copy-mode", no_argument, NULL, 'c' },
                          { "print-stats", no_argument, NULL, 'p' },
                          { "step-by-step", no_argument, NULL, 'b' },
                          { "debug", no_argument, NULL, 'g' },
                          { "force-sequence", no_argument, NULL, 'q' },
                          { "active-waiting", no_argument, NULL, 'w' },
                          { "help", no_argument, NULL, 'h' },
                          { "tsk-per-sta", no_argument, NULL, 'n' },
                          { "sck-type-tsk", no_argument, NULL, 'r' },
                          { "sck-type-sta", no_argument, NULL, 'R' },
                          { "pinning-policy", no_argument, NULL, 'P' },
                          { 0 } };

    std::string n_threads_param;
    std::vector<size_t> n_threads;
    size_t n_inter_frames = 1;
    size_t sleep_time_us = 5;
    size_t data_length = 2048;
    size_t buffer_size = 16;
    std::string dot_filepath;
    std::string in_filepath;
    std::string out_filepath = "file.out";
    bool no_copy_mode = true;
    bool print_stats = false;
    bool step_by_step = false;
    bool debug = false;
    bool force_sequence = false;
    bool active_waiting = false;
    size_t stages_number = 0;
    std::string tsk_per_sta_param;
    std::vector<size_t> tsk_per_sta;
    std::string sck_type_tsk_param;
    std::vector<std::vector<std::string>> sck_type_tsk;
    std::string sck_type_sta_param;
    std::vector<std::string> sck_type_sta;
    std::string pinning_policy;

    while (1)
    {
        const int opt = getopt_long(argc, argv, "t:f:s:d:u:o:i:j:n:r:R:P:cpbgqwh", longopts, 0);
        if (opt == -1) break;
        switch (opt)
        {
            case 't':
                n_threads_param = std::string(optarg);
                parse_int_string(n_threads_param, n_threads);
                // Add two threads for source and sink
                n_threads.push_back(1);
                n_threads.insert(n_threads.begin(), 1);
                break;
            case 'f':
                n_inter_frames = atoi(optarg);
                break;
            case 's':
                sleep_time_us = atoi(optarg);
                break;
            case 'd':
                data_length = atoi(optarg);
                break;
            case 'u':
                buffer_size = atoi(optarg);
                break;
            case 'o':
                dot_filepath = std::string(optarg);
                break;
            case 'i':
                in_filepath = std::string(optarg);
                break;
            case 'j':
                out_filepath = std::string(optarg);
                break;
            case 'c':
                no_copy_mode = false;
                break;
            case 'p':
                print_stats = true;
                break;
            case 'b':
                step_by_step = true;
                break;
            case 'g':
                debug = true;
                break;
            case 'w':
                active_waiting = true;
                break;
            case 'q':
                force_sequence = true;
                break;
            case 'n':
                tsk_per_sta_param = std::string(optarg);
                parse_int_string(tsk_per_sta_param, tsk_per_sta);
                break;
            case 'r':
                sck_type_tsk_param = std::string(optarg);
                parse_sck_type_tsk(sck_type_tsk_param, sck_type_tsk);
                break;
            case 'R':
                sck_type_sta_param = std::string(optarg);
                parse_sck_type_sta(sck_type_sta_param, sck_type_sta);
                break;
            case 'P':
                pinning_policy = std::string(optarg);
                break;
            case 'h':
                std::cout << "usage: " << argv[0] << " [options]" << std::endl;
                std::cout << std::endl;
                std::cout << "  -t, --n-threads          "
                          << "Number of threads to run in parallel for each stage                   "
                             "["
                          << (n_threads_param.empty() ? "empty" : "\"" + n_threads_param + "\"") << "]" << std::endl;
                std::cout << "  -f, --n-inter-frames     "
                          << "Number of frames to process in one task                               "
                          << "[" << n_inter_frames << "]" << std::endl;
                std::cout << "  -s, --sleep-time         "
                          << "Sleep time duration in one task (microseconds)                        "
                          << "[" << sleep_time_us << "]" << std::endl;
                std::cout << "  -d, --data-length        "
                          << "Size of data to process in one task (in bytes)                        "
                          << "[" << data_length << "]" << std::endl;
                std::cout << "  -u, --buffer-size        "
                          << "Size of the buffer between the different stages of the pipeline       "
                          << "[" << data_length << "]" << std::endl;
                std::cout << "  -o, --dot-filepath       "
                          << "Path to dot output file                                               "
                          << "[" << (dot_filepath.empty() ? "empty" : "\"" + dot_filepath + "\"") << "]" << std::endl;
                std::cout << "  -i, --in-filepath        "
                          << "Path to the input file (used to generate bits of the chain)           "
                          << "[" << (in_filepath.empty() ? "empty" : "\"" + in_filepath + "\"") << "]" << std::endl;
                std::cout << "  -j, --out-filepath       "
                          << "Path to the output file (written at the end of the chain)             "
                          << "[" << (out_filepath.empty() ? "empty" : "\"" + out_filepath + "\"") << "]" << std::endl;
                std::cout << "  -c, --copy-mode          "
                          << "Enable to copy data in sequence (performance will be reduced)         "
                          << "[" << (no_copy_mode ? "false" : "true") << "]" << std::endl;
                std::cout << "  -b, --step-by-step       "
                          << "Enable step-by-step sequence execution (performance will be reduced)  "
                          << "[" << (step_by_step ? "true" : "false") << "]" << std::endl;
                std::cout << "  -p, --print-stats        "
                          << "Enable to print per task statistics (performance will be reduced)     "
                          << "[" << (print_stats ? "true" : "false") << "]" << std::endl;
                std::cout << "  -g, --debug              "
                          << "Enable task debug mode (print socket data)                            "
                          << "[" << (debug ? "true" : "false") << "]" << std::endl;
                std::cout << "  -q, --force-sequence     "
                          << "Force sequence instead of pipeline                                    "
                          << "[" << (force_sequence ? "true" : "false") << "]" << std::endl;
                std::cout << "  -w, --active-waiting     "
                          << "Enable active waiting in the pipeline synchronizations                "
                          << "[" << (active_waiting ? "true" : "false") << "]" << std::endl;
                std::cout << "  -n, --tsk-per-sta        "
                          << "The number of tasks on each stage of the pipeline                     "
                          << "[" << (tsk_per_sta_param.empty() ? "empty" : "\"" + tsk_per_sta_param + "\"") << "]"
                          << std::endl;
                std::cout << "  -r, --sck-type-tsk       "
                          << "The socket type of each task (SFWD or SIO)                            "
                          << "[" << (sck_type_tsk_param.empty() ? "empty" : "\"" + sck_type_tsk_param + "\"") << "]"
                          << std::endl;
                std::cout << "  -R, --sck-type-sta       "
                          << "The socket type of tasks on each stage (SFWD or SIO)                  "
                          << "[" << (sck_type_sta_param.empty() ? "empty" : "\"" + sck_type_sta_param + "\"") << "]"
                          << std::endl;
                std::cout << "  -P, --pinning-policy     "
                          << "Pinning policy for pipeline execution                                 "
                          << "[" << (pinning_policy.empty() ? "empty" : "\"" + pinning_policy + "\"") << "]"
                          << std::endl;
                std::cout << "  -h, --help               "
                          << "This help                                                             "
                          << "[false]" << std::endl;
                exit(0);
                break;
            default:
                break;
        }
    }

    // Checking for errors
    if (!sck_type_sta.empty() && !sck_type_tsk.empty())
    {
        message << "You have to select only one parameter for socket type ('-r' exclusive or '-R').";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    // Constructing the socket type vector in case of stage socket
    for (size_t i = 0; i < sck_type_sta.size(); ++i)
    {
        sck_type_tsk.push_back({});
        for (size_t j = 0; j < tsk_per_sta[i]; ++j)
            sck_type_tsk[i].push_back(sck_type_sta[i]);
    }

    // Get the stage number
    stages_number = tsk_per_sta.size();

    // Parametre checking
    if (stages_number != (n_threads.size() - 2) || stages_number != sck_type_tsk.size())
    {
        message << "Number of stages is incoherent";
        throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }

    // Checking for task number and socket type
    for (size_t i = 0; i < sck_type_tsk.size(); ++i)
    {
        if (sck_type_tsk[i].size() != tsk_per_sta[i])
        {
            message << "Number of tasks is incompatible with number of sockets for stage " << i;
            throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
        }
    }

    std::cout << "#####################################" << std::endl;
    std::cout << "# Micro-benchmark: Generic pipeline #" << std::endl;
    std::cout << "#####################################" << std::endl;
    std::cout << "#" << std::endl;

    std::cout << "# Command line arguments:" << std::endl;
    std::cout << "#   - n_threads      = " << (n_threads_param.empty() ? "[empty]" : n_threads_param.c_str())
              << std::endl;
    std::cout << "#   - tsk_per_sta    = " << (tsk_per_sta_param.empty() ? "[empty]" : tsk_per_sta_param.c_str())
              << std::endl;
    std::cout << "#   - sck_type_tsk   = " << (sck_type_tsk_param.empty() ? "[empty]" : sck_type_tsk_param.c_str())
              << std::endl;
    std::cout << "#   - sck_type_sta   = " << (sck_type_sta_param.empty() ? "[empty]" : sck_type_sta_param.c_str())
              << std::endl;
    std::cout << "#   - pinning_policy = " << (pinning_policy.empty() ? "[empty]" : pinning_policy.c_str())
              << std::endl;
    std::cout << "#   - n_inter_frames = " << n_inter_frames << std::endl;
    std::cout << "#   - sleep_time_us  = " << sleep_time_us << std::endl;
    std::cout << "#   - data_length    = " << data_length << std::endl;
    std::cout << "#   - buffer_size    = " << buffer_size << std::endl;
    std::cout << "#   - dot_filepath   = " << (dot_filepath.empty() ? "[empty]" : dot_filepath.c_str()) << std::endl;
    std::cout << "#   - in_filepath    = " << (in_filepath.empty() ? "[empty]" : in_filepath.c_str()) << std::endl;
    std::cout << "#   - out_filepath   = " << (out_filepath.empty() ? "[empty]" : out_filepath.c_str()) << std::endl;
    std::cout << "#   - no_copy_mode   = " << (no_copy_mode ? "true" : "false") << std::endl;
    std::cout << "#   - print_stats    = " << (print_stats ? "true" : "false") << std::endl;
    std::cout << "#   - step_by_step   = " << (step_by_step ? "true" : "false") << std::endl;
    std::cout << "#   - debug          = " << (debug ? "true" : "false") << std::endl;
    std::cout << "#   - force_sequence = " << (force_sequence ? "true" : "false") << std::endl;
    std::cout << "#   - active_waiting = " << (active_waiting ? "true" : "false") << std::endl;
    std::cout << "#" << std::endl;

    if (!force_sequence && !no_copy_mode)
        std::clog << rang::tag::warning << "'no_copy_mode' has no effect with pipeline (it is always enable)"
                  << std::endl;
    if (!force_sequence && step_by_step)
        std::clog << rang::tag::warning << "'step_by_step' is not available with pipeline" << std::endl;

    // modules creation
    const bool auto_reset = false;
    module::Source_user_binary<uint8_t> source(data_length, in_filepath, auto_reset);
    module::Sink_user_binary<uint8_t> sink(data_length, out_filepath);

    // Task creation
    std::vector<std::shared_ptr<module::Relayer<uint8_t>>> rlys(
      std::accumulate(tsk_per_sta.begin(), tsk_per_sta.end(), 0));
    size_t tas = 0;
    for (auto sta : sck_type_tsk)
    {
        for (auto str : sta)
        {
            if (str == "SIO" || str == "SFWD")
            {
                rlys[tas].reset(new module::Relayer<uint8_t>(data_length));
                rlys[tas]->set_custom_name("Relayer" + std::to_string(tas));
                rlys[tas]->set_ns(sleep_time_us * 1000);
            }
            else
            {
                message << "Invalid socket Type, it should be SFWD or SIO";
                throw tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
            }
            tas++;
        }
    }

    // Task binding
    std::string rly_lsck, rly_rsck;
    tas = 0;
    rly_lsck = (sck_type_tsk[0][0] == "SFWD") ? "relayf::fwd" : "relay::in";
    // First task to bind to the initializer
    (*rlys[tas].get())[rly_lsck] = source["generate::out_data"];

    // Binding tasks between them
    for (size_t i = 0; i < sck_type_tsk.size(); ++i)
    {
        for (size_t j = 0; j < sck_type_tsk[i].size() - 1; ++j)
        {
            rly_lsck = (sck_type_tsk[i][j + 1] == "SFWD") ? "relayf::fwd" : "relay::in";
            rly_rsck = (sck_type_tsk[i][j] == "SFWD") ? "relayf::fwd" : "relay::out";
            (*rlys[tas + 1].get())[rly_lsck] = (*rlys[tas].get())[rly_rsck];
            tas++;
        }
        // We have to bind the last task of stage i to the first one of task i+1
        if (i < sck_type_tsk.size() - 1)
        {
            rly_lsck = (sck_type_tsk[i + 1][0] == "SFWD") ? "relayf::fwd" : "relay::in";
            rly_rsck = (sck_type_tsk[i][sck_type_tsk[i].size() - 1] == "SFWD") ? "relayf::fwd" : "relay::out";
            (*rlys[tas + 1].get())[rly_lsck] = (*rlys[tas].get())[rly_rsck];
            tas++;
        }
    }

    // Last stage bind
    rly_rsck = (sck_type_tsk[sck_type_tsk.size() - 1][sck_type_tsk[sck_type_tsk.size() - 1].size() - 1] == "SFWD")
                 ? "relayf::fwd"
                 : "relay::out";
    sink["send_count::in_data"] = (*rlys[tas].get())[rly_rsck];
    sink["send_count::in_count"] = source["generate::out_count"];

    std::unique_ptr<runtime::Sequence> sequence_chain;
    std::unique_ptr<runtime::Pipeline> pipeline_chain;

    // The sequence is executed correctly
    if (force_sequence)
    {
        sequence_chain.reset(new runtime::Sequence(source("generate"), 1));
        sequence_chain->set_n_frames(n_inter_frames);
        sequence_chain->set_no_copy_mode(no_copy_mode);

        if (!dot_filepath.empty())
        {
            std::ofstream file(dot_filepath);
            sequence_chain->export_dot(file);
        }

        // configuration of the sequence tasks
        for (auto& mod : sequence_chain->get_modules<module::Module>(false))
            for (auto& tsk : mod->tasks)
            {
                tsk->reset();
                tsk->set_debug(debug);       // disable the debug mode
                tsk->set_debug_limit(16);    // display only the 16 first bits if the debug mode is enabled
                tsk->set_stats(print_stats); // enable the statistics
                tsk->set_fast(true);         // enable the fast mode (= disable the useless verifs in the tasks)
            }

        auto t_start = std::chrono::steady_clock::now();
        if (!step_by_step)
            sequence_chain->exec();
        else
        {
            do
            {
                try
                {
                    for (size_t tid = 0; tid < 1; tid++)
                        while (sequence_chain->exec_step(tid))
                            ;
                }
                catch (tools::processing_aborted&)
                { /* do nothing */
                }
            } while (!source.is_done());
        }
        std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;

        auto elapsed_time = duration.count() / 1000.f / 1000.f;
        std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;
    }
    else
    {
        // Task vector creation
        std::vector<std::pair<std::vector<runtime::Task*>, std::vector<runtime::Task*>>> stage_creat;
        tas = 0;

        // First stage contains only the generate task
        stage_creat.push_back({ { &source("generate") }, { &source("generate") } });
        // Middle stages
        for (size_t i = 0; i < sck_type_tsk.size(); ++i)
        {
            std::string rly_ltsk = sck_type_tsk[i][0] == "SFWD" ? "relayf" : "relay";
            std::string rly_rtsk = sck_type_tsk[i][sck_type_tsk[i].size() - 1] == "SFWD" ? "relayf" : "relay";
            stage_creat.push_back(
              { { &(*rlys[tas].get())(rly_ltsk) }, { &(*rlys[tas + sck_type_tsk[i].size() - 1].get())(rly_rtsk) } });
            tas += sck_type_tsk[i].size();
        }
        // Last stage creation 	with the sink
        stage_creat.push_back({ { &sink("send_count") }, {} });

        // Buffer vector
        std::vector<size_t> pool_buff;
        for (size_t i = 0; i < stages_number + 1; ++i)
            pool_buff.push_back(buffer_size);

        // Waiting vector
        std::vector<bool> wait_vect;
        for (size_t i = 0; i < stages_number + 1; ++i)
            wait_vect.push_back(active_waiting);

#ifdef AFF3CT_CORE_HWLOC
        // Stages to pin
        if (!pinning_policy.empty())
        {
            tools::Thread_pinning::init();
            std::vector<bool> enable_pin = { false };
            for (size_t i = 0; i < stages_number; ++i)
                enable_pin.push_back(true);
            enable_pin.push_back(false);
            pipeline_chain.reset(new runtime::Pipeline(
              source("generate"), stage_creat, n_threads, pool_buff, wait_vect, enable_pin, pinning_policy));
        }
        else
#endif
        {
            pipeline_chain.reset(
              new runtime::Pipeline(source("generate"), stage_creat, n_threads, pool_buff, wait_vect));
        }
        pipeline_chain->set_n_frames(n_inter_frames);

        if (!dot_filepath.empty())
        {
            std::ofstream file(dot_filepath);
            pipeline_chain->export_dot(file);
        }

        // configuration of the sequence tasks
        for (auto& mod : pipeline_chain->get_modules<module::Module>(false))
            for (auto& tsk : mod->tasks)
            {
                tsk->reset();
                tsk->set_debug(debug);       // disable the debug mode
                tsk->set_debug_limit(16);    // display only the 16 first bits if the debug mode is enabled
                tsk->set_stats(print_stats); // enable the statistics
                tsk->set_fast(true);         // enable the fast mode (= disable the useless verifs in the tasks)
            }

        auto t_start = std::chrono::steady_clock::now();
        pipeline_chain->exec();
        std::chrono::nanoseconds duration = std::chrono::steady_clock::now() - t_start;

        auto elapsed_time = duration.count() / 1000.f / 1000.f;
        std::cout << "Sequence elapsed time: " << elapsed_time << " ms" << std::endl;
    }

    size_t in_filesize = filesize(in_filepath.c_str());
    size_t n_frames = ((int)std::ceil((float)(in_filesize * 8) / (float)(data_length * n_inter_frames)));
    auto theoretical_time = (n_frames * ((rlys.size()) * sleep_time_us * 1000) * n_inter_frames) / 1000.f / 1000.f;
    std::cout << "Sequence theoretical time: " << theoretical_time << " ms" << std::endl;

    // verification of the sequence execution
    bool tests_passed = compare_files(in_filepath, out_filepath);
    if (tests_passed)
        std::cout << "# " << rang::style::bold << rang::fg::green << "Tests passed!" << rang::style::reset << std::endl;
    else
        std::cout << "# " << rang::style::bold << rang::fg::red << "Tests failed :-(" << rang::style::reset
                  << std::endl;
    unsigned int test_results = !tests_passed;

    // display the statistics of the tasks (if enabled)
    if (print_stats)
    {
        std::cout << "#" << std::endl;
        if (force_sequence)
            tools::Stats::show(sequence_chain->get_modules_per_types(), true, false);
        else
        {
            auto stages = pipeline_chain->get_stages();
            for (size_t s = 0; s < stages.size(); s++)
            {
                const int n_threads = stages[s]->get_n_threads();
                std::cout << "#" << std::endl
                          << "# Pipeline stage " << s << " (" << n_threads << " thread(s)): " << std::endl;
                tools::Stats::show(stages[s]->get_tasks_per_types(), true, false);
            }
        }
    }

    // sockets unbinding
    if (force_sequence)
        sequence_chain->set_n_frames(1);
    else
    {
        pipeline_chain->set_n_frames(1);
        pipeline_chain->unbind_adaptors();
    }

    tas = 0;

    if (sck_type_tsk[0][0] == "SFWD")
        (*rlys[tas].get())[module::rly::sck::relayf::fwd].unbind(source[module::src::sck::generate::out_data]);
    else
        (*rlys[tas].get())[module::rly::sck::relay::in].unbind(source[module::src::sck::generate::out_data]);

    for (size_t i = 0; i < sck_type_tsk.size(); ++i)
    {
        for (size_t j = 0; j < sck_type_tsk[i].size() - 1; ++j)
        {
            if (sck_type_tsk[i][j + 1] == "SFWD")
            {
                if (sck_type_tsk[i][j] == "SFWD")
                    (*rlys[tas + 1].get())[module::rly::sck::relayf::fwd].unbind(
                      (*rlys[tas].get())[module::rly::sck::relayf::fwd]);
                else
                    (*rlys[tas + 1].get())[module::rly::sck::relayf::fwd].unbind(
                      (*rlys[tas].get())[module::rly::sck::relay::out]);
            }
            else
            {
                if (sck_type_tsk[i][j] == "SFWD")
                    (*rlys[tas + 1].get())[module::rly::sck::relay::in].unbind(
                      (*rlys[tas].get())[module::rly::sck::relayf::fwd]);
                else
                    (*rlys[tas + 1].get())[module::rly::sck::relay::in].unbind(
                      (*rlys[tas].get())[module::rly::sck::relay::out]);
            }
            tas++;
        }
        // We have to unbind the last task of stage i to the first one of task i+1
        if (i < sck_type_tsk.size() - 1)
        {
            if (sck_type_tsk[i + 1][0] == "SFWD")
            {
                if (sck_type_tsk[i][sck_type_tsk[i].size() - 1] == "SFWD")
                    (*rlys[tas + 1].get())[module::rly::sck::relayf::fwd].unbind(
                      (*rlys[tas].get())[module::rly::sck::relayf::fwd]);
                else
                    (*rlys[tas + 1].get())[module::rly::sck::relayf::fwd].unbind(
                      (*rlys[tas].get())[module::rly::sck::relay::out]);
            }
            else
            {
                if (sck_type_tsk[i][sck_type_tsk[i].size() - 1] == "SFWD")
                    (*rlys[tas + 1].get())[module::rly::sck::relay::in].unbind(
                      (*rlys[tas].get())[module::rly::sck::relayf::fwd]);
                else
                    (*rlys[tas + 1].get())[module::rly::sck::relay::in].unbind(
                      (*rlys[tas].get())[module::rly::sck::relay::out]);
            }
            tas++;
        }
    }

    // Last stage bind
    if (sck_type_tsk[sck_type_tsk.size() - 1][sck_type_tsk[sck_type_tsk.size() - 1].size() - 1] == "SFWD")
        sink[module::snk::sck::send_count::in_data].unbind((*rlys[tas].get())[module::rly::sck::relayf::fwd]);
    else
        sink[module::snk::sck::send_count::in_data].unbind((*rlys[tas].get())[module::rly::sck::relay::out]);

    sink[module::snk::sck::send_count::in_count].unbind(source[module::src::sck::generate::out_count]);

    return test_results;
}
