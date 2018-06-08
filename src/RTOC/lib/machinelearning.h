#ifndef RTOC_MACHINELEARNING_H
#define RTOC_MACHINELEARNING_H

#include <memory>
#include <vector>
#include "datacontainer.h"

class Machinelearning {
public:
    Machinelearning();
    virtual void loadModel(const std::string& path);

    virtual void predictObject(DataContainer& dataContainer, double decisionBoundary=0.5);

    int get_XBoundary() const;
    void set_XBoundary(int m_XBoundary);

    struct outputData {
        double output;
        double probabilityOut;
        int label;

        outputData(double output, double probabilityOut, int label) :
                output(output), probabilityOut(probabilityOut), label(label) {}
    };

    const std::vector<outputData>& getResults() {return results;};

protected:
    std::vector<int> xpos;
    std::vector<data::DataFlags> attributes;
    std::vector<outputData> results;
    int findClosestXpos(const int& pos, DataContainer& dataContainer);

private:
    int _XBoundary = 250;
};

class LogisticRegression : public Machinelearning {
public:
    LogisticRegression();
    void loadModel(const std::string& path) override;

    void predictObject(DataContainer& dataContainer, double decisionBoundary) override;

private:
    std::vector<double> coefficients;
    int intercept;
};

class ArtificialNeuralNetwork : public Machinelearning {
public:
    ArtificialNeuralNetwork();

private:
    std::vector<int> weights;
};

class DecisionTree : public Machinelearning {
public:
    DecisionTree();

private:
};

class NaiveBayes : public Machinelearning {
public:
    NaiveBayes();

private:
};

std::unique_ptr<Machinelearning> identifyModel(const std::string& path);

#endif  // RTOC_MACHINELEARNING_H