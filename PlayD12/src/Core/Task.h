// Task System with Persistent Thread Queues 
// Goals:
// - Domain-based thread workers , per-frame std::future construction
// - thread-safe, reusable queue model

#pragma once
#include <functional>
#include <vector>
#include <unordered_map>
#include <string>
#include <cassert>
#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

namespace System {

    enum class ETaskDomain {
        MainThread,
        PhysicsThread,
        RenderThread,
        WorkerThread
    };


    inline const char* DomainName(ETaskDomain domain)  {
        switch (domain) {
        case ETaskDomain::MainThread: return "MainThread";
        case ETaskDomain::PhysicsThread: return "PhysicsThread";
        case ETaskDomain::RenderThread: return "RenderThread";
        case ETaskDomain::WorkerThread: return "WorkerThread";
        default: return "Unknown";
        }
    }

    struct FTask {
        std::string debugName;
        std::function<void()> callback;
        std::vector<std::string> deps;
        ETaskDomain domain = ETaskDomain::MainThread;
        bool bExecuted = false;
        std::mutex Mutex;
    };

    class FTaskQueue {
    public:
        void Push(std::function<void()> task) {
            {
                std::lock_guard<std::mutex> lock(Mutex);
                tasks.push(std::move(task));
            }
            CV.notify_one();
        }

        void WorkerLoop() {
            while (!bExit) {
                std::function<void()> task;
                {
                    std::unique_lock lock(Mutex);
                    CV.wait(lock, [&] { return bExit || !tasks.empty(); });
                    if (bExit && tasks.empty()) break;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task(); 
				//std::cout << "Thread: " << std::this_thread::get_id() << " executing task\n";
            }
        }

        void Shutdown() {
            {
                std::lock_guard<std::mutex> lock(Mutex);
                bExit = true;
            }
            CV.notify_all();
        }

    private:
        std::queue<std::function<void()>> tasks;
        std::mutex Mutex;
        std::condition_variable CV;
        bool bExit = false;
    };


    class FTaskSystem {
    public:
        FTaskSystem() {
            StartWorker(ETaskDomain::PhysicsThread);
            StartWorker(ETaskDomain::RenderThread);
            //StartWorker(ETaskDomain::WorkerThread);
        }

        ~FTaskSystem() {
            for (auto& [_, q] : queues) q.Shutdown();
            for (auto& [_, t] : threads) if (t.joinable()) t.join();
        }

        void StartWorker(ETaskDomain domain) {
            threads[domain] = std::thread([this, domain] {
                queues[domain].WorkerLoop();
                });
        }

        void AddTask(const std::string& name, 
            std::function<void()> callback,  
			ETaskDomain domain = ETaskDomain::MainThread,
            const std::vector<std::string>& deps = {} ) 
        {
            std::lock_guard<std::mutex> lock(taskMutex);
            assert(!tasks.contains(name) && "Duplicate task name");
            tasks[name] = std::make_unique<FTask>(name, std::move(callback), deps, domain);
        }

        void ExecuteAll() {
            for (const auto& [name, task] : tasks) {
                if (task->domain == ETaskDomain::MainThread) {
					ExecuteTask(task.get());  
                }

                else {
                    //queues[task->domain].Push([this, name] { ExecuteTask(name); });
                    pendingAsyncTasks++; //increment immediately;
                    queues[task->domain].Push([this, task = task.get()] {
                        ExecuteTask(task);
                        pendingAsyncTasks--;  
                        });
                }
            }
            WaitForAll();
            Reset();
            tasks.clear();  //a direct way 
        }

        void DebugPrint() const {
            std::cout << "[TaskSystem] Registered Tasks:\n";
            for (const auto& [name, task] : tasks) {
                std::cout << " - " << name << " [" << DomainName(task->domain) << "] (depends on: ";
                for (const auto& dep : task->deps) std::cout << dep << " ";
                std::cout << ")\n";
            }
        }

    private: 
        void ExecuteTask(FTask* task) {
            std::lock_guard<std::mutex> lock(task->Mutex);
            if (task->bExecuted) return;

            for (const auto& dep : task->deps) {
                ExecuteTask(tasks.at(dep).get());  
            }

            if (task->callback) {
                task->callback();
            }

            task->bExecuted = true;
        }

        void Reset() {
            for (auto& [_, task] : tasks) task->bExecuted = false;
        }

        void WaitForAll() {
            while (pendingAsyncTasks.load() > 0) {
                std::this_thread::yield();
            }
        }  

    private:
        std::unordered_map<std::string, std::unique_ptr<FTask>> tasks;
        std::mutex taskMutex;

        std::unordered_map<ETaskDomain, FTaskQueue> queues;
        std::unordered_map<ETaskDomain, std::thread> threads;
        std::atomic<int> pendingAsyncTasks = 0;

    };

}  

/*
Usage:

FTaskSystem taskSys;
taskSys.AddTask("GameLogic", [] { RunGameplay(); }, {}, ETaskDomain::MainThread);
taskSys.AddTask("Physics", [] { RunPhysics(); }, {}, ETaskDomain::PhysicsThread);
taskSys.AddTask("Render", [] { SubmitFrame(); }, {}, ETaskDomain::RenderThread);
taskSys.ExecuteAll();
*/
