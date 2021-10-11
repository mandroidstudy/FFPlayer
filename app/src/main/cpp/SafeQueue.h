//
// Created by maoweiyi on 2021/9/25.
//

#ifndef FFPLAYER_SAFEQUEUE_H
#define FFPLAYER_SAFEQUEUE_H

#include <queue>
#include <pthread.h>

template<typename T>
class SafeQueue {
private:
    typedef void (*ReleaseCallback)(T* t);
private:
    std::queue<T> values;
    pthread_mutex_t  mutex{};
    pthread_cond_t cond{};
    bool isWorking = false;
    ReleaseCallback releaseCallback;
public:
    SafeQueue(){
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);
    }

    void setReleaseCallback(ReleaseCallback callback){
        this->releaseCallback = callback;
    }

    void push(T t) {
        pthread_mutex_lock(&mutex);
        if (isWorking){
            values.push(t);
            pthread_cond_signal(&cond);
        } else{
            if (releaseCallback){
                releaseCallback(&t);
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    int frontAndPop(T& t) {
        pthread_mutex_lock(&mutex);
        while (isWorking && values.empty()){
            pthread_cond_wait(&cond,&mutex);
        }
        if (isWorking && !values.empty()){
            t = values.front();
            values.pop();
            pthread_mutex_unlock(&mutex);
            return 1;
        }
        pthread_mutex_unlock(&mutex);
        return 0;
    }

    void setWorkStatus(bool _working) {
        pthread_mutex_lock(&mutex);
        this->isWorking = _working;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    int size() const {
        return values.size();
    }

    bool empty() const {
        return values.empty();
    }

    void clear() {
        pthread_mutex_lock(&mutex);
        while (!values.empty()){
            T t = values.front();
            if (releaseCallback){
                releaseCallback(&t);
            }
            values.pop();
        }
        pthread_mutex_unlock(&mutex);
    }

    ~SafeQueue(){
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }
};
#endif //FFPLAYER_SAFEQUEUE_H
