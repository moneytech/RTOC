#ifndef IMAGEWRITER_H
#define IMAGEWRITER_H

#include <boost/filesystem.hpp>
#include <opencv/cv.hpp>
#include <queue>
#include <string>

#include <thread>

#include "external/timer/timer.h"

#include "helper.h"
#include "setup.h"

#ifndef NDEBUG
#define NDEBUG
#endif
#include "../external/readerwriterqueue/readerwriterqueue.h"

namespace {
namespace fs = boost::filesystem;
}

class ImageWriter {
public:
    ImageWriter() = default;

    void push(const cv::Mat& img) { m_queue.enqueue(img.clone()); }
    void clear() {
        // clear queue
        clearCamel(m_queue);
        m_running = false;
        m_finishedWriting = false;
        m_targetImageCount = -1;
        m_forceStop = false;
    }

    void startWriting(const fs::path& path, const std::string& prefix) {
        if (!m_running) {
            m_running = true;
            m_path = path;
            m_prefix = prefix;
            m_index = 0;
            std::thread t(&ImageWriter::writeThreaded, this);
            t.detach();
        }
    }

    void finishWriting(int targetImageCount) {
        // Set target images that we require to be written to disk and wait for finished image
        // writing
        m_targetImageCount = targetImageCount;
        while (!m_finishedWriting) {
        }
    }

    void forceStop() { m_forceStop = true; }

private:
    volatile bool m_finishedWriting = false;  // avoid optimizing finishWriting() while loop
    bool m_running = false;
    bool m_forceStop = false;

    moodycamel::BlockingReaderWriterQueue<cv::Mat> m_queue;

    Setup m_setup;

    fs::path m_path;
    std::string m_prefix;

    int m_index = 0;
    int m_targetImageCount = -1;

    void writeThreaded() {
        // Will monitor m_queue until m_run is deasserted
        // When deasserted, m_queue will be emptied
        cv::Mat front;
        bool queueHasValue = false;

        // Monitor queue until
        //  1. targetImageCount has been set (this will be set when the imageWriter is told to stop
        //  executing
        //  and
        //  2. our index (image count) is equals to the target image count
        while (!(m_targetImageCount >= 0 && m_targetImageCount <= m_index)) {
            if (m_forceStop) {
                // Halt image writer
                clearCamel(m_queue);
                goto finish;
            }
            // To not create a high-priority thread, we do small thread sleeps to enable OS to
            // schedule our writeThreaded() call as low priority
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            queueHasValue = m_queue.wait_dequeue_timed(front, std::chrono::milliseconds(1));
            if (queueHasValue) {
                std::string filepath =
                    (m_path / fs::path(m_prefix + "_" + std::to_string(m_index) + ".png")).string();
                cv::imwrite(filepath, front);
                m_index++;
            } else {
                // error in writing images - did not find the expected amt of images in the queue,
                // as what the analyzer told the ImageWriter to write before timeout

                /** @todo when real-time object-finder is implemented, below should break. current
                 * situation is quite dangerous since imagewriter ONLY stops if all images have been
                 * written. We cannot yet implement a timeout on imeagewriter, since imagewriter is
                 * started during acquisition, and thus, if acquisition is long, imageWriter would
                 * timeout*/
                // break;
            }
        }

    finish:
        m_finishedWriting = true;
        m_running = false;
    }
};

#endif  // IMAGEWRITER_H
