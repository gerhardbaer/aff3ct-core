/*!
 * \file
 * \brief Class module::Adaptor_1_to_n.
 */
#ifndef ADAPTOR_1_TO_N_HPP_
#define ADAPTOR_1_TO_N_HPP_

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <typeindex>
#include <vector>

#include "Module/Adaptor/Adaptor.hpp"
#include "Runtime/Socket/Socket.hpp"
#include "Runtime/Task/Task.hpp"

namespace aff3ct
{
namespace module
{

class Adaptor_1_to_n : public Adaptor
{
  public:
    inline runtime::Task& operator[](const adp::tsk t);
    inline runtime::Socket& operator[](const adp::sck::push_1 s);
    inline runtime::Socket& operator[](const adp::sck::pull_n s);

    const bool active_waiting;

    std::shared_ptr<std::vector<std::condition_variable>> cnd_pull;
    std::shared_ptr<std::vector<std::mutex>> mtx_pull;
    std::shared_ptr<std::condition_variable> cnd_put;
    std::shared_ptr<std::mutex> mtx_put;

  public:
    inline Adaptor_1_to_n(const size_t n_elmts,
                          const std::type_index datatype,
                          const size_t buffer_size = 1,
                          const bool active_waiting = true);
    inline Adaptor_1_to_n(const std::vector<size_t>& n_elmts,
                          const std::vector<std::type_index>& datatype,
                          const size_t buffer_size = 1,
                          const bool active_waiting = true);
    virtual ~Adaptor_1_to_n() = default;
    virtual Adaptor_1_to_n* clone() const;

    inline void init();

    void wake_up();
    void cancel_waiting();

  protected:
    virtual void push_1(const std::vector<const int8_t*>& in, const size_t frame_id);
    virtual void pull_n(const std::vector<int8_t*>& out, const size_t frame_id);

    void wait_push();
    void wait_pull();
    void* get_empty_buffer(const size_t sid);
    void* get_filled_buffer(const size_t sid);
    void* get_empty_buffer(const size_t sid, void* swap_buffer);
    void* get_filled_buffer(const size_t sid, void* swap_buffer);
    void wake_up_pusher();
    void wake_up_puller();
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Module/Adaptor/Adaptor_1_to_n.hxx"
#endif

#endif /* ADAPTOR_1_TO_N_HPP_ */
