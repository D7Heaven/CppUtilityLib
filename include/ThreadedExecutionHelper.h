//
// Created by arnoulr on 5/28/19.
//

#ifndef CPP_THREADPOOL_THREADEDEXECUTIONHELPER_H
#define CPP_THREADPOOL_THREADEDEXECUTIONHELPER_H

#include <vector>
#include <functional>
#include <future>
#include <algorithm>

template <class R, class Iter, typename Functor>
std::vector<R> parallelForImpl(Iter begin, Iter end, Functor &functor) {
    std::vector<R>		result;

    for (auto iterator = begin; iterator != end; ++iterator) {
        result.emplace_back(std::move(functor(*iterator)));
    }
    return result;
}


template <class R, class Iter, typename Functor>
std::vector<R> parallelFor(Iter begin, Iter end, Functor functor, int nbThreads) {
    std::vector<std::future<std::vector<R>>> futureResults;
    std::vector<R> aggregatedResult;
    int containerSize = end - begin;
    int subVectorSize;
    int localNbThread = nbThreads;


    // If the number of image is one no need to launch threads
    if (containerSize <= 1)
        return parallelForImpl<R>(begin, end, functor);
    // If the number of image is smaller than the number of threads, reduce number of threads
    if (containerSize < nbThreads)
        localNbThread = containerSize;
    subVectorSize = containerSize / localNbThread;
    // Split the vector in nbThreads sub-vector and launch them in parallel

    for (int i = 0; i < localNbThread; ++i) {
        Iter beginIdx = begin + i * subVectorSize;
        Iter endIdx = beginIdx + subVectorSize;
        if (i == localNbThread - 1) {
            endIdx = end;
        }
        auto fut = std::async(std::launch::async, &parallelForImpl<R, Iter, Functor>, beginIdx, endIdx,  std::ref(functor));
        futureResults.emplace_back(std::move(fut));
    }
    // Merge results form futures and return
    for (auto &futureResult : futureResults) {
        auto result = futureResult.get();
        aggregatedResult.insert(aggregatedResult.end(), result.begin(), result.end());
    }
    return aggregatedResult;
}



#endif //CPP_THREADPOOL_THREADEDEXECUTIONHELPER_H
