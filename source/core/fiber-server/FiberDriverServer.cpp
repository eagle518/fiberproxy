#include "FiberDriverServer.h"
#include <glog/logging.h>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <iostream>

namespace fibp {

FiberDriverServer::FiberDriverServer(
    const boost::asio::ip::tcp::endpoint& bindPort,
    factory_ptr& connectionFactory,
    std::size_t threadPoolSize
)
: parent_type(bindPort, connectionFactory),
  threadPoolSize_(threadPoolSize)
{
    if (threadPoolSize_ < 2)
    {
        threadPoolSize_ = 2;
    }
    connectionFactory->setFiberPoolMgr(&fiberpool_mgr_);
}

void FiberDriverServer::run()
{
    normal_stop_ = false;
    boost::thread_group threads;
    boost::thread_group fiber_runner_threads;
    boost::barrier b(2*threadPoolSize_ + 1);
    fiberpool_mgr_.initFiberPool(boost::this_thread::get_id());
    for (std::size_t i = 0; i < threadPoolSize_; ++i)
    {
        boost::thread* ret = threads.create_thread(boost::bind(&FiberDriverServer::worker, this, boost::ref(b)));
        fiber_runner_threads.create_thread(boost::bind(&FiberDriverServer::fiber_runner, this, ret->get_id(), boost::ref(b)));
    }

    parent_type::init();
    b.wait();
    fiberpool_mgr_.runFiberPool(boost::this_thread::get_id());
    fiber_runner_threads.join_all();
    threads.join_all();
}

void FiberDriverServer::stop()
{
    normal_stop_ = true;
    //LOG(INFO) << "stop driver server by request.";
    fiberpool_mgr_.stopFiberPool();
    parent_type::stop();
}

void FiberDriverServer::fiber_runner(const boost::thread::id& tid, boost::barrier& b)
{
    fiberpool_mgr_.initFiberPool(tid);
    b.wait();
    fiberpool_mgr_.runFiberPool(tid);
}

void FiberDriverServer::worker(boost::barrier& b)
{
    b.wait();
    LOG(INFO) << "thread running.";
    for(;;)
    {
        try
        {
            parent_type::run();
            // normal exit
            if (normal_stop_)
                break;
            else
            {
                LOG(ERROR) << "driver stopped by accident.";
            }
        }
        catch (std::exception& e)
        {
            LOG(ERROR) << "[DriverServer] "
                << e.what() << std::endl;
        }
    }
}

}

