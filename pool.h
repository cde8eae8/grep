#ifndef POOL_H
#define POOL_H

#include "taskqueue.h"

#include <QRunnable>
#include <QThreadPool>
#include <QObject>

struct pool_signals : QObject {
    Q_OBJECT

public:
    explicit pool_signals(QObject *parent = nullptr) :
        QObject (parent) {}

signals:
    void pool_finished();
};

template
<typename TaskType>
class Pool : public pool_signals, public QRunnable {
public:
    virtual void on_finish() { }
    virtual void on_start() { }
    virtual void process(TaskType &) = 0;
    virtual bool ready_to_finish() { return true; }
    virtual void all_finished() { }

    Pool(size_t n_threads) noexcept :
        n_threads_(n_threads),
        this_queue(),
        n_this_pool_tasks(0),
        finished_(false),
        n_active_threads(n_threads) {
        setAutoDelete(false);
    }

    void run() override {
//        std::cout << "Pool: started\n";
        thread_started();
        on_start();
        if (should_finish())
            finish();
        while (!finished()) {
            TaskType task;
            pop_task(task);
            if (finished()) break;

            process(task);

            finish_task();
            if (should_finish())
                finish();
        }
        on_finish();
        thread_finished();
//        std::cout << "Pool: finished\n";
    }

    void start() {
        for (size_t i = 0; i < n_threads_; ++i) {
            QThreadPool::globalInstance()->start(this);
        }
    }

    void reset() {
        this_queue.reset();
        n_this_pool_tasks = 0;
        finished_ = false;
        n_active_threads = n_threads_;
    }

    // may be called multiple times
    void finish() {
//        std::cout << "finishing..." << std::endl;
        finished_ = true;
        this_queue.finish();
    }

    template<typename It>
    void add_tasks(It begin, It end) {
        for (auto i = begin; i != end; ++i) {
            n_this_pool_tasks++;
            this_queue.push(*i);
        }
    }

    void add_task(QString& s) {
        n_this_pool_tasks++;
        this_queue.push(s);
    }

    bool finished() {
        return finished_;
    }

protected:
    void pop_task(QString& s) {
        this_queue.pop(s);
    }

    void finish_task() {
        n_this_pool_tasks--;
    }

    bool should_finish() {
//        std::string s = std::to_string(n_this_pool_tasks) + " " + std::to_string(ready_to_finish()) + "\n";
//        std::cout << s;
        return n_this_pool_tasks == 0 && ready_to_finish();
    }

    void thread_started() { }

    void thread_finished() {
        n_active_threads--;
        if (n_active_threads == 0) {
            all_finished();
            emit pool_finished();
        }
    }

    ~Pool() override {
        finish();
    }

private:
    const size_t n_threads_;

    TaskQueue<TaskType> this_queue;
    std::atomic<size_t> n_this_pool_tasks;
    std::atomic<bool> finished_;
    std::atomic<size_t> n_active_threads;
};


#endif // POOL_H
