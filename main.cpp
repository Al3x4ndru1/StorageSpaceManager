#include <iostream>
#include <filesystem>
#include <future>
#include <map>
#include <thread>
#include <vector>

namespace fs = std::filesystem;


void get_directory_size(const fs::path& path, std::promise<std::map<fs::path, uintmax_t>> p) {
    uintmax_t size = 0;
    std::map<fs::path, uintmax_t> m;
    try {
        if (fs::exists(path) && fs::is_directory(path)) {
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                if (fs::is_regular_file(entry.status())) {
                    size += fs::file_size(entry);
                }
            }
        }
    }catch (const fs::filesystem_error& e) {
        std::cout<<e.what()<<std::endl;
    }
    m[path] = size;
    p.set_value(std::move(m));
}


std::string human_readable(uintmax_t size) {
    const char* suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double dsize = size;
    while (dsize >= 1024 && i < 4) {
        dsize /= 1024;
        ++i;
    }
    char buf[100];
    snprintf(buf, sizeof(buf), "%.2f %s", dsize, suffixes[i]);
    return std::string(buf);
}


int main() {

    std::vector<fs::path> list ;
    std::string path = "/home";

    for (const auto & entry : fs::directory_iterator(path)) {
        list.push_back(entry.path());
    }

    // std::vector<std::thread>;
    std::vector<std::future<std::map<fs::path, uintmax_t>>> futures;
    std::vector<std::thread> threads;
    for (auto const & s : list) {
        std::promise<std::map<fs::path, uintmax_t>> p;
        futures.push_back(p.get_future());
        threads.emplace_back(get_directory_size, s, std::move(p));
    }


    for (auto& t : threads) {
        t.join();
    }

    for (size_t i = 0; i < futures.size(); ++i){
        auto result = futures[i].get();

        for (const auto& [path, size] : result) {
            std::cout<<path.filename()<<" "<<human_readable(size)<<std::endl;
        }
    }


    return 0;
}