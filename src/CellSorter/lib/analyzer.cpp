#include "analyzer.h"

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/unique_ptr.hpp>


/**
 * @brief
 * @param img_path
 */
void Analyzer::loadExperimentPreset(const std::string& img_path) {
    m_experiment.defaultSettings(img_path);
}

/**
 * @brief
 *
 * @warning DEPRECATED
 */
void Analyzer::loadImagesFromFolder() {
    std::vector<framefinder::Frame> frames;
    std::string img_folder = m_experiment.imagePath;
    get_files(frames, img_folder);
    accept_or_reject(frames, img_folder, m_experiment.intensity_threshold);
    get_accepted(frames, m_experiment.acc);
    get_rejected(frames, m_experiment.dis);
}

/**
 * @brief
 *
 * @warning NOT IMPLEMENTED
 */
void Analyzer::loadImagesFromText() {
    std::string accepted;  // Path to accepted text-file
    std::string rejected;  // Path to rejected text-file
    // Compile img_folder path with filenames from text-files
    // and push frames to m_experiment
}

/**
 * @brief Sets analyzer background
 * @details
 *
 * @param bg : background image to be used
 */
void Analyzer::setBG(const cv::Mat& bg) {
    m_bg = bg;
}

/**
 * @brief
 *
 * @warning DEPRECATED
 */
void Analyzer::selectBG() {
    m_bg = m_experiment.dis[10].image;  // Sets as background
}

/**
 * @brief Run all processes on
 * @details
 *
 */
void Analyzer::runProcesses() {
    processImage(m_img, m_bg);
}

/**
 * @brief Process a single frame
 * @details Assuming background is set in Analyzer
 *
 * @param img
 */
void Analyzer::processSingleFrame(cv::Mat& img) {
    processImage(img, m_bg);
}

/**
 * @brief Overload: Process a single frame
 * @details
 *
 * @param img
 * @param bg
 */
void Analyzer::processSingleFrame(cv::Mat& img, cv::Mat& bg) {
    processImage(img, bg);
}

/**
 * @brief
 * @details
 *
 */
void Analyzer::runAnalyzer() {
    bool success;
    while (true) {
        m_img = m_imageGetterFunction(success);

        if (!success) {
            break;
        }

        if (m_bg.dims == 0) {
            m_bg = m_img;
        } else {
            processImage(m_img, m_bg);
            m_experiment.processed.push_back({m_img, "", m_experiment.cellNum++, true});
        }
    }
}

/**
 * @brief Clears the process tree
 * @details
 *
 */
void Analyzer::resetProcesses() {
    m_processes.clear();
}


/**
 * @brief
 * @details
 *
 */
void Analyzer::findObjects() {
    cv::Point centroid;
    double dist;
    double dist_thres;
    std::vector<double> d;
    bool newcell = false;
    int cellNum = -1;
    int sameCell = 0;
    int frameNo = 0;
    Tracker t;
    std::vector<Tracker> tracker, currentCells;
    int numObjects = 0;
    DataContainer dc(0xffff);

    for (const framefinder::Frame& f : m_experiment.processed) {
        cv::Mat img_to_show = f.image;

        // Get data from blobs in frame
        numObjects = matlab::regionProps(f.image, 0xffff, dc);

        for (int i = 0; i < numObjects; i++) {
            if (cellNum < 0) {
                newcell = true;
            } else {
                Tracker term(frameNo - 1);

                currentCells = matlab::find<Tracker>(tracker, term);

                if (currentCells.empty()) {
                    newcell = true;
                } else {
                    // Calculate distances
                    for (const Tracker& cc : currentCells) {
                        centroid = dc[i]->getValue<cv::Point>(data::Centroid);
                        dist = matlab::dist(centroid, cc.centroid);
                        if (dist < -10)
                            dist = 100;
                        d.push_back(dist);
                    }

                    std::pair<double, unsigned long> p = matlab::min<double>(d);
                    dist = p.first;
                    t = currentCells.at(p.second);

                    d.clear();
                    currentCells.clear();

                    // Set threshold
                    if (centroid.x <= m_experiment.inlet - 5) {
                        dist_thres = 5.0;  // Should be a settable variable
                    } else {
                        dist_thres = 20.0;  // Should be a settable variable
                    }

                    // Determine whether new or not from threshold
                    newcell = dist > dist_thres;
                }
            }
            if (newcell) {
                cellNum++;

                m_experiment.data.emplace_back(new DataContainer(0xffff));
                m_experiment.data[cellNum]->appendNew();

                (*m_experiment.data[cellNum])[0]->setValue(data::Inlet, m_experiment.inlet);
                (*m_experiment.data[cellNum])[0]->setValue(data::Outlet, m_experiment.outlet);
                // yref ???
                (*m_experiment.data[cellNum])[0]->setValue(data::Label, cellNum);

                (*m_experiment.data[cellNum])[0]->setValue(data::Frame, frameNo);
                (*m_experiment.data[cellNum])[0]->setValue(
                    data::Centroid, dc[i]->getValue<cv::Point>(data::Centroid));
                (*m_experiment.data[cellNum])[0]->setValue(
                    data::BoundingBox, dc[i]->getValue<cv::Rect>(data::BoundingBox));
                (*m_experiment.data[cellNum])[0]->setValue(
                    data::Major_axis, dc[i]->getValue<double>(data::Major_axis));
                (*m_experiment.data[cellNum])[0]->setValue(
                    data::Eccentricity, dc[i]->getValue<double>(data::Eccentricity));
                (*m_experiment.data[cellNum])[0]->setValue(
                    data::Circularity, dc[i]->getValue<double>(data::Circularity));
                (*m_experiment.data[cellNum])[0]->setValue(data::Symmetry,
                                                           dc[i]->getValue<double>(data::Symmetry));
                (*m_experiment.data[cellNum])[0]->setValue(
                    data::GradientScore, dc[i]->getValue<double>(data::GradientScore));

                t.cell_no = cellNum;

            } else {
                sameCell = t.cell_no;

                m_experiment.data[sameCell]->appendNew();
                auto index = m_experiment.data[sameCell]->size() - 1;

                (*m_experiment.data[sameCell])[index]->setValue(data::Frame, frameNo);
                (*m_experiment.data[sameCell])[index]->setValue(
                    data::Centroid, dc[i]->getValue<cv::Point>(data::Centroid));
                (*m_experiment.data[sameCell])[index]->setValue(
                    data::BoundingBox, dc[i]->getValue<cv::Rect>(data::BoundingBox));
                (*m_experiment.data[sameCell])[index]->setValue(
                    data::Major_axis, dc[i]->getValue<double>(data::Major_axis));
                (*m_experiment.data[sameCell])[index]->setValue(
                    data::Eccentricity, dc[i]->getValue<double>(data::Eccentricity));
                (*m_experiment.data[sameCell])[index]->setValue(
                    data::Circularity, dc[i]->getValue<double>(data::Circularity));
                (*m_experiment.data[sameCell])[index]->setValue(
                    data::Symmetry, dc[i]->getValue<double>(data::Symmetry));
                (*m_experiment.data[sameCell])[index]->setValue(
                    data::GradientScore, dc[i]->getValue<double>(data::GradientScore));
            }
            t.frame_no = frameNo;
            t.centroid = dc[i]->getValue<cv::Point>(data::Centroid);
            tracker.push_back(t);
        }

        frameNo++;
    }
}

/**
 * @brief Clean Object-vector from objects with insufficient data
 * @details
 *
 * @warning NOT FULLY IMPLEMENTED
 */
void Analyzer::cleanObjects() {
    unsigned int count_threshold = 25;

    auto n = m_experiment.data.size();
    std::vector<bool> remove(n);

    for (unsigned long i = 0; i < n; i++) {
        DataContainer* dc = m_experiment.data[i].get();
        cv::Rect bb_i = (*dc).front()->getValue<cv::Rect>(data::BoundingBox);
        cv::Rect bb_o = (*dc).back()->getValue<cv::Rect>(data::BoundingBox);
        remove.at(i) = (*dc).size() < 25 || ((bb_i.x + bb_i.width) > m_experiment.inlet - 1) ||
                       ((bb_o.x + bb_o.width) < m_experiment.outlet);
    }

    // Burde være i sin egen funktion
    m_experiment.data.erase(std::remove_if(m_experiment.data.begin(), m_experiment.data.end(),
                                           [thresh = count_threshold](const auto& dc) {
                                               return (*dc).size() < thresh;
                                           }),
                            m_experiment.data.end());
}

/**
 * @brief function for storing data from experiment
 * @details Stores the contents of `m_experiment.data` to some external file
 *
 * @param path
 * @return
 */
bool Analyzer::storeData(const std::string& path) {
    return false;
}

/// Debug helpers
void Analyzer::showImg(const int& delay) {
    cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);
    cv::imshow("Display window", m_img);
    cv::waitKey(delay);
}
/// Debug helpers
void Analyzer::showImg(const cv::Mat& img, const int& delay) {
    cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);
    cv::imshow("Display window", img);
    cv::waitKey(delay);
}

/**
 * @brief
 * @details
 *
 * @param path
 * @return
 */
bool Analyzer::storeSetup(const string& path) {
    // Serialize current processes in m_processes in .xml file
    try {
        std::ofstream ofs(path);
        {
            boost::archive::xml_oarchive oa(ofs);
            oa << BOOST_SERIALIZATION_NVP(m_processes);
        }
        ofs.close();
    } catch (...) {
        return false;
    }
    return true;
}

/**
 * @brief
 * @details
 *
 * @param path
 * @return
 */
bool Analyzer::loadSetup(const string& path) {
    try {
        m_processes.clear();
        std::ifstream ifs(path);
        {
            boost::archive::xml_iarchive ia(ifs);
            ia >> BOOST_SERIALIZATION_NVP(m_processes);
        }
        ifs.close();
    } catch (...) {
        return false;
    }
    return true;
}


bool Analyzer::exportAllFormat(const string& path) {
    std::vector<std::string> attributes = m_experiment.data[0]->extractAttributeName();
    std::ofstream out(path);
    // Adds list of attributes
    for (int i = 0; i < attributes.size(); i++) {
        out << attributes[i] << "'";
    }
    out << "\n";

    // Adds number number of values for chosen attributes
    std::vector<int> attributeLengths = m_experiment.data[0]->extractAttributeLengths();
    for (const auto& value : attributeLengths) {
        out << value << " ";
    }

    // Adds number of containers
    out << "\n" << m_experiment.data.size() << "\n";

    // Goes through all containers
    for (int i = 0; i < m_experiment.data.size(); i++) {
        out << "Observation" << (i+1) << " " << m_experiment.data[i]->size() << "\n";

        // Goes through all objects
        for (int j = 0; j < m_experiment.data[i]->size(); j++) {
            // Extracts vector of doubles from object
            std::vector<double> objectVector = m_experiment.data[i]->extractObjectInDoubles(j);
            for (const auto& item : objectVector) {
                out << item << " ";
            }
            out << "\n";
        }
    }
    out.close();
    return true;
}

void Analyzer::processImage(cv::Mat &img, cv::Mat &bg) {
    for (const auto& process : m_processes) {
        process->doProcessing(img, bg, m_experiment);
    }
}
