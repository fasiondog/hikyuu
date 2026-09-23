/*
 *  Copyright(C) 2020 hikyuu.org
 *
 *  Create on: 2020-12-20
 *     Author: fasiondog
 */

#pragma once

#include <thread>
#include <functional>
#include "../../DataType.h"
#include "../../utilities/thread/ThreadPool.h"
#include "../SpotRecord.h"

namespace hikyuu {
namespace flat {
struct Spot;
}
}  // namespace hikyuu

namespace hku {

/**
 * Agent that receives the external realtime data
 * @ingroup Agent
 */
class HKU_API SpotAgent {
public:
    SpotAgent() = default;

    /** Destructor */
    virtual ~SpotAgent();

    /** Start the agent */
    void start();

    /** Stop the agent */
    void stop();

    /** Whether it is running */
    bool isRunning() {
        return !m_stop;
    }

    void setWorkerNum(size_t worker_num) {
        m_work_num = worker_num;
    }

    size_t getWorkerNum() const {
        return m_work_num;
    }

    /** Set whether to print the data receiving progress; it is mainly used to turn off the
     *  printing in an interactive environment */
    void setPrintFlag(bool print) {
        m_print = print;
    }

    bool getPrintFlag() const {
        return m_print;
    }

    void setServerAddr(const string& addr) {
        m_server_addr = addr;
    }

    const string& getServerAddr() const {
        return m_server_addr;
    }

    bool isConnected() const {
        return m_connected;
    }

    /**
     * Add the handler called when Spot data is received
     * @note This operation can only be performed in the stopped state, otherwise an exception is
     *       thrown
     * @param process the handler, it processes a single spot record only
     */
    void addProcess(std::function<void(const SpotRecord&)> process);

    /**
     * Add the post-processing function to be called after all the batches of data at a certain
     * moment have been received; the given datetime is the moment when the data is received (not
     * the time when the data has been fully processed)
     * @note This operation can only be performed in the stopped state, otherwise an exception is
     *       thrown
     * @param func the post-processing function
     */
    void addPostProcess(std::function<void(Datetime)> func);

    /**
     * Clear all the handlers added before
     * @note This operation can only be performed in the stopped state, otherwise an exception is
     *       thrown
     */
    void clearProcessList();

    /**
     * Clear all the post-processing functions added before
     * @note This operation can only be performed in the stopped state, otherwise an exception is
     *       thrown
     */
    void clearPostProcessList();

public:
    static void setQuotationServer(const string& server);

private:
    static string ms_pubUrl;                 // Address of the data sending service
    static const char* ms_startTag;          // Start marker of a batch data receiving
    static const char* ms_endTag;            // End marker of a batch data receiving
    static const char* ms_spotTopic;         // Topic subscribed from the data sending service
    static const size_t ms_startTagLength;   // Length of the batch receiving start marker
    static const size_t ms_endTagLength;     // Length of the batch receiving end marker
    static const size_t ms_spotTopicLength;  // Length of the subscribed topic marker

    static Datetime ms_start_rev_time;  // Start time of the batch data receiving

private:
    SpotAgent(const SpotAgent&) = delete;
    SpotAgent(SpotAgent&&) = delete;
    SpotAgent& operator=(const SpotAgent&) = delete;
    SpotAgent& operator=(SpotAgent&&) = delete;

    unique_ptr<SpotRecord> parseFlatSpot(const hikyuu::flat::Spot* spot);
    void parseSpotData(const void* buf, size_t buf_len);

    void work_thread();

private:
    enum STATUS {
        WAITING,
        RECEIVING
    };  // Waiting for a new batch of data, or receiving a batch
        // of data
    enum STATUS m_status = WAITING;        // Current internal state
    std::mutex m_run_mutex;                // Prevents multi-threaded start / stop
    std::atomic_bool m_stop = true;        // Flag for ending the agent work
    std::atomic_bool m_connected = false;  // Whether the data service has been connected

    int m_revTimeout = 100;                         // Timeout for connecting the data service (ms)
    std::thread m_receiveThread;                    // Data receiving thread
    std::unique_ptr<ThreadPool> m_tg;               // Thread pool for the data processing tasks
    size_t m_work_num = 1;                          // Number of the threads in the data processing
                                                    // task thread pool
    std::unique_ptr<ThreadPool> m_receive_data_tg;  // Data receiving task group

    bool m_print = true;   // Whether to print the connection information
    string m_server_addr;  // Server address

    // The following attributes need to be locked when they are modified, so that strategy can be
    // run in a multi-threaded way
    std::mutex m_mutex;
    list<std::function<void(const SpotRecord&)>> m_processList;  // List of the registered spot
                                                                 // handlers
    list<std::function<void(Datetime)>> m_postProcessList;       // List of the registered batch
                                                                 // post-processing functions
};

}  // namespace hku