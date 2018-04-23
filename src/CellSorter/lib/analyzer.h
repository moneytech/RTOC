#ifndef CELLSORTER_CSHELPER_H
#define CELLSORTER_CSHELPER_H
#include <algorithm>
#include <fstream>
#include <iostream>
#include "experiment.h"
#include "framefinder.h"
#include "process.h"
#include "tracker.h"

#include <opencv/cv.hpp>

#include <functional>

class Analyzer {
public:
    processContainerPtr getProcessContainerPtr() { return &m_processes; }

    void loadExperimentPreset(const std::string& img_path);
    void loadImagesFromFolder();
    void loadImagesFromText();
    void setBG(const cv::Mat& bg);
    void selectBG();
    void runProcesses();
    void runAnalyzer();
    void resetProcesses();
    void processSingleFrame(cv::Mat& img);
    void processSingleFrame(cv::Mat& img, cv::Mat& bg);

    void setImageGetterFunction(std::function<cv::Mat&(bool&)> function) {
        m_imageGetterFunction = function;
    }

    void findObjects();
    void cleanObjects();

    bool exportExperiment(const string& path);
    bool exportAllFormat(const string& path);

    bool storeSetup(const std::string& path);
    bool loadSetup(const std::string& path);
    bool storeData(const std::string& path);

    void showImg(const int& delay);
    static void showImg(const cv::Mat& img, const int& delay);

    Experiment m_experiment;  // CHECK IF THOSE CAN BE PRIVATE
    cv::Mat m_img;
    cv::Mat m_bg;

private:
    void processImage(cv::Mat &img, cv::Mat &bg);

    std::vector<std::unique_ptr<ProcessBase>> m_processes;

    std::vector<std::unique_ptr<DataContainer>> m_data;  // experiment data here ? (JL, 17-04-18)

    std::function<cv::Mat&(bool& sucessful)> m_imageGetterFunction;
};

#endif  // CELLSORTER_CSHELPER_H
