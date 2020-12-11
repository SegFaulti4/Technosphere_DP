#include "coroutine.h"
#include <queue>
#include <memory>
#include <ucontext.h>
#include <mutex>

namespace coroutine {
    struct Routine;

    void entry();

    thread_local struct ThreadOrdinator {
        routine_t current = 0;
        ucontext_t ctx{};
    } threadOrdinator;

    struct Ordinator {
        static constexpr size_t STACK_SIZE = 1 << 16;
        std::mutex ordinator_mutex;
        std::vector<Routine> routines;
        std::queue<routine_t> finished;

        Ordinator() {
            routines.reserve(MAX_ROUTINE_AMOUNT);
        }
    } ordinator;

    struct Routine {
        RoutineFunction func;
        std::unique_ptr<uint8_t[]> stack;
        bool finished = false;
        ucontext_t ctx;
        std::exception_ptr exception;

        void reset(const RoutineFunction& f) {
            func = f;
            finished = false;
            exception = {};
            makecontext(&ctx, entry, 0);
        }

        Routine(const RoutineFunction& f)
                : func{f}, stack{std::make_unique<uint8_t[]>(Ordinator::STACK_SIZE)} {
            ctx.uc_stack.ss_sp = stack.get();
            ctx.uc_stack.ss_size = Ordinator::STACK_SIZE;
            ctx.uc_link = nullptr;
            getcontext(&ctx);
            makecontext(&ctx, entry, 0);
        }

        Routine(const Routine&) = delete;
        Routine(Routine&&) = default;
    };

    routine_t create(const RoutineFunction& function) {
        auto& o = ordinator;
        std::lock_guard lock(o.ordinator_mutex);
        if (o.finished.empty()) {
            o.routines.emplace_back(function);
            return o.routines.size();
        } else {
            routine_t id = o.finished.front();
            o.finished.pop();
            auto& routine = o.routines[id - 1];
            routine.reset(function);
            return id;
        }
    }

    bool resume(routine_t id) {
        auto& o = ordinator;
        auto& to = threadOrdinator;
        Routine *routine_ptr;
        {
            std::lock_guard lock(o.ordinator_mutex);
            if (id == 0 || id > o.routines.size()) {
                return false;
            }
            routine_ptr = &o.routines[id - 1];
        }
        if (routine_ptr->finished) {
            return false;
        }

        to.current = id;
        routine_ptr->ctx.uc_link = &to.ctx;
        if (swapcontext(&to.ctx, &routine_ptr->ctx) < 0) {
            to.current = 0;
            return false;
        }

        if (routine_ptr->exception) {
            std::rethrow_exception(routine_ptr->exception);
        }

        return true;
    }

    void yield() {
        auto& o = ordinator;
        auto& to = threadOrdinator;
        routine_t id = to.current;
        Routine *routine_ptr;
        {
            std::lock_guard lock(o.ordinator_mutex);
            routine_ptr = &o.routines[id - 1];
        }

        to.current = 0;
        swapcontext(&routine_ptr->ctx, &to.ctx);
    }

    routine_t current() {
        return threadOrdinator.current;
    }

    void entry() {
        auto& o = ordinator;
        auto& to = threadOrdinator;
        routine_t id = to.current;
        Routine *routine_ptr;
        {
            std::lock_guard lock(o.ordinator_mutex);
            routine_ptr = &o.routines[id - 1];
        }

        if (routine_ptr->func) {
            try {
                routine_ptr->func();
            } catch (...) {
                routine_ptr->exception = std::current_exception();
            }
        }

        routine_ptr->finished = true;
        to.current = 0;
        {
            std::lock_guard lock(o.ordinator_mutex);
            o.finished.emplace(id);
        }
        swapcontext(&routine_ptr->ctx, routine_ptr->ctx.uc_link);
    }

}
