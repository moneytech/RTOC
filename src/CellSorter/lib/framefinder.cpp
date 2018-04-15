#include "framefinder.h"



bool exists(const std::string& path) {
    struct stat buf;
    return stat(path.c_str(), &buf) == 0;
}

/**
 * Function name should explain enough
 * takes address of folder path and the address for the results
 *
 * @param   files   :   path to folder
 * @param   folder  :   vector<string> with file names
 * @return          :   Files found
 */
int files_from_folder(std::vector<std::string>& files, const std::string& folder) {
    if (!exists(folder)) {
        return -1;
    }
    try {
        boost::filesystem::path p(folder);
        boost::filesystem::directory_iterator start(p);
        boost::filesystem::directory_iterator end;
        transform(start, end, back_inserter(files), path_leaf_string());
    } catch (...) {
        throw std::string("Error reading from file");
    }
    return (int)files.size();
}

/**
 * Gets string between two delimiters (assuming only one instance of last delimiter)
 * @param src   :   string to search in
 * @param first :   first delimiter
 * @param last  :   last delimiter
 * @return      :   extracted std::string
 */
std::string extractBetween(const std::string& src, const std::string& first,
                           const std::string& last) {
    unsigned long a = src.rfind(first);
    if (a < 0) return "NOT_FOUND";
    a += first.length();
    unsigned long b = src.find(last, a);
    if (b < 0) return "NOT_FOUND";
    return src.substr((unsigned long) a,(unsigned long)  b - a);
}
/**
 * Overload function with less inputs (simplerrr)
 * @param src   :   string to search in
 * @return      :   string between delimiters
 */
std::string extractBetween(const std::string& src) {
    unsigned long a = src.rfind('_');
    if (a < 0) return "NOT_FOUND";
    a++;
    unsigned long b = src.find('.', a);
    if (b < 0) return "NOT_FOUND";
    return src.substr((unsigned long) a,(unsigned long)  b - a);
}

/**
 * Extension to get_files(), but sorts the output
 *
 * @param files     :   vector<string>  :   Container for found filenames
 * @param folder    :   string          :   Path to folder
 * @param mode      :   FILE_LIST_MODE  :   Set output mode
 * @return          :   int             :   error code
 */
int get_files(std::vector<Frame>& files, const std::string& folder) {
    std::vector<std::string> file_paths;
    // Get size and return if empty or error
    int count = files_from_folder(file_paths, folder);
    if (count <= 0) {
        return -1;
    }
    // Allocate vector
    std::vector<Frame> fn((unsigned)count);
    // Get file-numbers and accept_or_reject
    for (int i = 0; i < count; i++) {
        auto id = (int)strtol(extractBetween(file_paths[i]).c_str(), nullptr, 10);
        Frame f = {cv::Mat(), file_paths[i], id, false};
        fn[i] = f;
    }

    sort(fn.begin(), fn.end());

    files = fn;

    return 0;
}

/**
 * Accept_or_reject
 *
 * @param frames        :   vector<string>  :   full list of images
 * @param img_folder    :   string          :   path to image folder
 * @param path_acc      :   string          :   path to "accepted" txt-file
 * @param path_dis      :   string          :   path to "discarded" txt-file
 */
void accept_or_reject(std::vector<Frame>& frames, const std::string& img_folder,
                      const double& threshold) {
    double crit = 0.0;

    cv::Mat lastMoved = cv::imread(img_folder + "/" + frames[0].filename, cv::IMREAD_GRAYSCALE);
    ;

    // Loop through all pictures
    for (auto& frame : frames) {
        std::string img_path = img_folder + "/" + frame.filename;
        frame.image = cv::imread(img_path, cv::IMREAD_GRAYSCALE);
        minMaxIdx(lastMoved - frame.image, nullptr, &crit);
        crit /= 255;

        if (crit <= threshold) {
            // Discard
            frame.accepted = false;
        } else {
            // Accept
            frame.accepted = true;
            lastMoved = frame.image;
        }
    }
}

void get_accepted(const std::vector<Frame>& frames, std::vector<Frame>& output) {
    for (const Frame& f : frames) {
        if (f.accepted) {
            output.emplace_back(f);
        }
    }
}

void get_rejected(const std::vector<Frame>& frames, std::vector<Frame>& output) {
    for (const Frame& f : frames) {
        if (!f.accepted) {
            output.push_back(f);
        }
    }
}

/**
 * @brief sorts a qfileinfolist from number found between the two specified delimiters
 * Mening that from the file names found in qfileinfolist there should be an integer id
 * number formatted as following:
 * /filename[del1][id number][del2].[ext]
 * Example:
 * /photo_1342.png
 * where del1 = "_" and del2 "."
 *
 * @param qfil: the list to be sorted
 */
void sort_qfilelist(QFileInfoList& qfil) {
    // Allocate vector
    auto n = qfil.size();
    std::vector<Frame_Q> fn(n);

    // Get file-numbers
    for (int i = 0; i < n; i++) {
        auto str = qfil[i].fileName().toStdString();
        auto id = (int)strtol(extractBetween(str).c_str(), nullptr, 10);
        Frame_Q f = {qfil[i], id};
        fn[i] = f;
    }

    // Sort
    sort(fn.begin(), fn.end());

    // Reinsert all QFileInfo obejcts in qfil
    for (int i = 0; i < n; i++) {
        qfil[i] = fn[i].fileinfo;
    }
}

/**
 * @brief overload with delimiters as extra argument
 *
 * @param qfil: the list to be sorted
 * @param del1: the first delimiter
 * @param del2: the second delimiter
 */
void sort_qfilelist(QFileInfoList& qfil, const std::string& del1, const std::string& del2) {
    // Allocate vector
    auto n = qfil.size();
    std::vector<Frame_Q> fn(n);

    // Get file-numbers
    for (int i = 0; i < n; i++) {
        auto str = qfil[i].fileName().toStdString();
        auto id = (int)strtol(extractBetween(str,del1,del2).c_str(), nullptr, 10);
        Frame_Q f = {qfil[i], id};
        fn[i] = f;
    }

    // Sort
    sort(fn.begin(), fn.end());

    // Reinsert all QFileInfo obejcts in qfil
    for (int i = 0; i < n; i++) {
        qfil[i] = fn[i].fileinfo;
    }
}