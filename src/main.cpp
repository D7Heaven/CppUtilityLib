#include <iostream>
#include <ThreadedExecutionHelper.h>
#include <zconf.h>
#include <ThreadPool.h>

int doStuff(int a) {
    return a * a;
}


int main() {
    std::function<int(int)> func = doStuff;
    std::vector<int> inputs(50000, 5);
    auto starP = std::chrono::steady_clock::now();
    std::vector<int> res = parallelFor<int>(inputs.begin(), inputs.end(), [](int value) {
        return value * value ;
        }, 12);
    auto stopP = std::chrono::steady_clock::now();
    auto diffP = stopP - starP;
    std::cout << "Container function : " << std::chrono::duration <double, std::milli> (diffP).count() << " ms" << std::endl;


}