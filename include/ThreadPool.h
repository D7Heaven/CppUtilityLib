//
// Created by arnoulr on 4/11/19.
//

#ifndef CPP_THREADPOOL_THREADPOOL_H
#define CPP_THREADPOOL_THREADPOOL_H

#include <functional>
#include <thread>
#include <mutex>
#include <queue>
#include <future>

#define THREAD_NUMBER 2

template<class R, class ...A>
class ThreadPool {

public:
    ThreadPool(std::function<R(A...)> function, uint threadNumber = THREAD_NUMBER)
            : _function(function) {
        for (int i = 0; i < threadNumber; ++i) {
            std::promise<void>  exitSignal;
            _exitSignals.emplace_back(std::move(exitSignal));
            _threadPool.push_back(std::thread(&ThreadPool::infiniteLoop, this, _exitSignals[i].get_future()));
        }
    }

    ~ThreadPool() {
        if (_threadRunning)
            waitUntilFinished();
    }

    void push(const A &&... obj) {
        _inputMutex.lock();
        _inputs.push(std::make_tuple(obj...));
        _inputMutex.unlock();
    }

    void push(const A &... obj) {
        _inputMutex.lock();
        _inputs.push(std::make_tuple(obj...));
        _inputMutex.unlock();
    }

    std::optional<R> retrieve() {
        if (!_outputs.empty()) {
            _outputMutex.lock();
            R result = _outputs.front();
            _outputs.pop();
            _outputMutex.unlock();
            return std::optional<R>(result);
        }
        return std::nullopt;
    }

    void waitUntilFinished() {
        for (auto &exitSignal : _exitSignals) {
            exitSignal.set_value();
        }
        for (auto &thread : _threadPool) {
            if (thread.joinable())
                thread.join();
        }
    }

    void addThread(uint threadNumber = 1) {
        for (int i = 0; i < threadNumber; ++i) {
            std::promise<void>  exitSignal;
            _exitSignals.emplace_back(std::move(exitSignal));
            _threadPool.push_back(std::thread(&ThreadPool::infiniteLoop, this, _exitSignals[i].get_future()));

        }
    }

    void removeThread(uint threadNumber = 1) {
        for (int i = 0; i < threadNumber; ++i) {
            if (i >= _exitSignals.size())
                break;
            _exitSignals[i].set_value();
            _threadPool[i].join();
            _exitSignals.erase(_exitSignals.begin());
            _threadPool.erase(_threadPool.begin());
        }
    }

private:
    void infiniteLoop(std::future<void> exitSignal) {
        while (exitSignal.wait_for(std::chrono::microseconds(100)) == std::future_status::timeout) {
            _inputMutex.lock();
            if (!_inputs.empty()) {
                // Retrieving objects from the vector
                std::tuple<A...> objectsTuple = _inputs.front();
                _inputs.pop();
                _inputMutex.unlock();
                // Executing the function with the retrieved objects
                R result = functionImplementation(objectsTuple, std::index_sequence_for<A...>{});
                // Push the result of the function
                _outputMutex.lock();
                _outputs.push(result);
                _outputMutex.unlock();
            } else
                _inputMutex.unlock();
        }
    }

    template<class Tuple, size_t... Indexes>
    R functionImplementation(Tuple &&tuple, std::index_sequence<Indexes...>) {
        return _function(std::get<Indexes>(tuple)...);
    }

private:
    std::function<R(A...)> _function;
    std::vector<std::thread> _threadPool;
    std::vector<std::promise<void>> _exitSignals;
    std::queue<std::tuple<A...>> _inputs;
    std::queue<R> _outputs;
    std::mutex _inputMutex;
    std::mutex _outputMutex;
    bool _threadRunning = true;
};



#endif //CPP_THREADPOOL_THREADPOOL_H
