#pragma once

#include <algorithm>
#include <queue>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <fstream>
#include <iostream>

class Archiver {
public:
    // Здесь происходит архивация файла, в новом виде указаны название файла исходника, алфавит расшифровки, содержимое файла
    void ArchiveFiles(std::string archive_name, std::vector<std::string> input_files) {
        std::ofstream fout(archive_name, std::ios_base::binary);
        std::vector<int> build_bytes;
        int copy_size;
        int count_files = input_files.size();
        int number_file = 0;
        std::vector<int> massive_bites;

        for (std::string file : input_files) {
            number_file += 1;
            symbols_count_ = 0;
            table_symb_ = Huffman(file);
            std::ifstream fin(file, std::ios_base::binary);
            std::unordered_map<int, int> types_of_length;
            std::vector<std::pair<std::string, std::string>> canonical_codes_sort;
            unsigned char byte;
            unsigned int finish = 0;
            int local_int;

            for (int i = 1; i < 10; i++) {
                types_of_length[i] = 0;
            }

            copy_size = symbols_count_;

            for (int i = 0; i < 9; i++) {
                build_bytes.push_back(copy_size % 2);
                copy_size /= 2;
                if (build_bytes.size() == 8) {
                    int output = GetCodeInt(build_bytes);
                    fout.put(static_cast<unsigned char>(output));
                    build_bytes = {};
                }
            }

            for (auto st = table_symb_.begin(); st != table_symb_.end(); ++st) {
                canonical_codes_sort.push_back({st->second, st->first});
                ++types_of_length[st->second.length()];
                if (st->second.length() > finish) {
                    finish = st->second.length();
                }
            }

            sort(canonical_codes_sort.begin(), canonical_codes_sort.end());

            for (auto st : canonical_codes_sort) {
                massive_bites = GetBites(st.second);

                for (int i : massive_bites) {
                    build_bytes.push_back(i);
                    if (build_bytes.size() == 8) {
                        int output = GetCodeInt(build_bytes);
                        fout.put(static_cast<unsigned char>(output));
                        build_bytes = {};
                    }
                }
            }

            for (unsigned int j = 1; j <= finish; j++) {
                local_int = types_of_length[j];

                for (int i = 0; i < 9; i++) {
                    build_bytes.push_back(local_int % 2);
                    local_int /= 2;
                    if (build_bytes.size() == 8) {
                        int output = GetCodeInt(build_bytes);
                        fout.put(static_cast<unsigned char>(output));
                        build_bytes = {};
                    }
                }
            }

            for (unsigned int j = 0; j < file.length(); j++) {
                massive_bites = GetBites(table_symb_[GetCodeStr(static_cast<int>(file[j]), 9)]);

                for (int i : massive_bites) {
                    build_bytes.push_back(i);
                    if (build_bytes.size() == 8) {
                        int output = GetCodeInt(build_bytes);
                        fout.put(static_cast<unsigned char>(output));
                        build_bytes = {};
                    }
                }
            }

            massive_bites = GetBites(table_symb_["000000001"]);

            for (int i : massive_bites) {
                build_bytes.push_back(i);

                if (build_bytes.size() == 8) {
                    int output = GetCodeInt(build_bytes);
                    fout.put(static_cast<unsigned char>(output));
                    build_bytes = {};
                }
            }

            while (!fin.eof()) {
                byte = fin.get();
                if (fin.eof()) {
                    break;
                }
                massive_bites = GetBites(table_symb_[GetCodeStr(static_cast<int>(byte), 9)]);

                for (int i : massive_bites) {
                    build_bytes.push_back(i);

                    if (build_bytes.size() == 8) {
                        int output = GetCodeInt(build_bytes);
                        fout.put(static_cast<unsigned char>(output));
                        build_bytes = {};
                    }
                }
            }

            if (number_file < count_files) {
                massive_bites = GetBites(table_symb_["100000001"]);
                for (int i : massive_bites) {
                    build_bytes.push_back(i);

                    if (build_bytes.size() == 8) {
                        int output = GetCodeInt(build_bytes);
                        fout.put(static_cast<unsigned char>(output));
                        build_bytes = {};
                    }
                }
            }
            fin.close();
        }

        massive_bites = GetBites(table_symb_["010000001"]);

        for (int i : massive_bites) {
            build_bytes.push_back(i);

            if (build_bytes.size() == 8) {
                int output = GetCodeInt(build_bytes);
                fout.put(static_cast<unsigned char>(output));
                build_bytes = {};
            }
        }

        if (!build_bytes.empty()) {
            int output = GetCodeInt(build_bytes);
            fout.put(static_cast<unsigned char>(output));
        }

        fout.close();
    }
    
    // Здесь файл расшифровывается: достаётся название файла, алфавит, содержимое и выводится в необходимый файл
    void BuildFiles(std::string filename) {
        std::ifstream fin(filename, std::ios_base::binary);
        std::vector<int> build_9_bites;
        std::string local_string;
        unsigned char byte;
        int copy_byte;
        int byte_9_bites;
        symbols_count_ = 0;
        string_output_bites_ = "";

        while (!fin.eof()) {
            byte = fin.get();
            if (fin.eof()) {
                break;
            }
            copy_byte = static_cast<int>(byte);

            for (int q = 0; q < 8; q++) {
                build_9_bites.push_back(copy_byte % 2);
                copy_byte /= 2;
                if (build_9_bites.size() == 9) {
                    byte_9_bites = GetCodeInt(build_9_bites);

                    if (symbols_count_ == 0) {
                        symbols_count_ = byte_9_bites;
                    } else if (check_end_symbols_ < symbols_count_) {
                        check_end_symbols_ += 1;
                        symbols_.push_back(GetCodeStr(byte_9_bites, 9));
                    } else if (check_end_codes_ + byte_9_bites < symbols_count_) {
                        check_end_codes_ += byte_9_bites;
                        count_lengths_.push_back(byte_9_bites);
                    } else if (check_end_codes_ + byte_9_bites == symbols_count_ && !is_build_tree_) {
                        is_build_tree_ = true;
                        check_end_codes_ += byte_9_bites;
                        count_lengths_.push_back(byte_9_bites);
                        BuildDecodeTree();
                    } else {
                        std::vector<int> bites_for_next_file = OutputSymbol(build_9_bites);
                        if (!bites_for_next_file.empty()) {
                            build_9_bites = bites_for_next_file;
                        }
                    }
                    if (symbols_count_ > 0) {
                        build_9_bites = {};
                    }
                }
            }
        }
    }
    
    // Пояснение для понятного будущего управления программой
    void PrintInformation() {
        std::cout << "Для работы с программой поддерживаются следующие форматы:" << std::endl;
        std::cout << "archiver -c archive filename1 filename2 ... filenamen --> архивирование файлов с именами "
                     "[filename1, filename2, ...] в файл [archive_name]"
                  << std::endl;
        std::cout << "archiver -d archive_name --> разархивирование архива [archive_name]" << std::endl;
        std::cout << "archiver -h --> получение данного сообщения" << std::endl;
    }

private:
    int symbols_count_ = 0;
    std::unordered_map<std::string, std::string> table_symb_;
    int check_end_symbols_ = 0;
    int check_end_codes_ = 0;
    bool is_build_tree_ = false;
    bool is_file_open_ = false;
    std::map<std::string, std::string> translate_;
    std::vector<std::string> symbols_;
    std::vector<int> count_lengths_;
    std::string name_of_file_out_;
    std::string string_output_bites_;
    
    // Для преобразования из обычного 8-битного алфавита в 9-битный применяем Хафмана, который даёт нам новый алфавит
    std::unordered_map<std::string, std::string> Huffman(std::string filename) {
        std::ifstream fin(filename, std::ios_base::binary);
        std::unordered_map<std::string, std::string> translate;
        std::unordered_map<std::string, size_t> count_symbols;
        std::unordered_map<std::string, std::vector<std::string>> bor_tree;
        std::string bites_9;

        for (size_t i = 0; i < filename.length(); i++) {
            ++count_symbols[GetCodeStr(static_cast<int>(filename[i]), 9)];
        }

        unsigned char byte;
        while (!fin.eof()) {
            byte = fin.get();
            if (fin.eof()) {
                break;
            }

            ++count_symbols[GetCodeStr(static_cast<int>(byte), 9)];
        }

        ++count_symbols["000000001"];
        ++count_symbols["100000001"];
        ++count_symbols["010000001"];

        std::priority_queue<std::pair<int, std::string>> my_queue;
        std::pair<int, std::string> smallest;
        std::pair<int, std::string> second_smallest;
        std::string new_string;

        for (auto i = count_symbols.begin(); i != count_symbols.end(); ++i) {
            symbols_count_ += 1;
            bor_tree[i->first] = {};
            my_queue.push({-(i->second), i->first});
        }

        while (my_queue.size() > 1) {
            smallest = my_queue.top();
            my_queue.pop();
            second_smallest = my_queue.top();
            my_queue.pop();
            new_string = smallest.second + second_smallest.second;
            bor_tree[new_string] = {smallest.second, second_smallest.second};
            my_queue.push({smallest.first + second_smallest.first, new_string});
        }

        std::deque<std::pair<std::string, std::string>> build_bin;
        build_bin.push_back({my_queue.top().second, ""});
        std::pair<std::string, std::string> go_next;

        while (!build_bin.empty()) {
            go_next = build_bin.front();
            build_bin.pop_front();
            if (!bor_tree[go_next.first].empty()) {
                build_bin.push_back({bor_tree[go_next.first][0], go_next.second + "0"});
                build_bin.push_back({bor_tree[go_next.first][1], go_next.second + "1"});
            } else {
                translate[go_next.first] = go_next.second;
            }
        }

        std::vector<std::pair<unsigned int, std::string>> for_canonical;
        int step;

        for (auto i = translate.begin(); i != translate.end(); ++i) {
            for_canonical.push_back({i->second.length(), i->first});
        }

        std::unordered_map<std::string, std::string> answer;
        sort(for_canonical.begin(), for_canonical.end());
        int int_canonical;
        unsigned int norm_len_string;
        std::string string_canonical;

        for (auto letter : for_canonical) {
            while (string_canonical.length() < letter.first) {
                string_canonical += "0";
            }
            answer[letter.second] = string_canonical;

            int_canonical = 0;
            step = 1;
            norm_len_string = string_canonical.length();

            for (int i = string_canonical.length() - 1; i >= 0; i--) {
                if (string_canonical[i] == '1') {
                    int_canonical += step;
                }
                step *= 2;
            }

            string_canonical = "";
            int_canonical += 1;

            while (int_canonical > 0) {
                if (int_canonical % 2 == 1) {
                    string_canonical = "1" + string_canonical;
                } else {
                    string_canonical = "0" + string_canonical;
                }
                int_canonical /= 2;
            }
            while (string_canonical.length() < norm_len_string) {
                string_canonical = "0" + string_canonical;
            }
        }

        fin.close();
        return answer;
    }
    
    // Именно здесь происходит перевод информации в выходной файл, поступающей во время работы BuildFiles
    std::vector<int> OutputSymbol(std::vector<int>& vector_bites) {
        std::vector<int> answer;
        for (int letter : vector_bites) {
            if (symbols_count_ == 0) {
                answer.push_back(letter);
            } else {
                if (letter == 1) {
                    string_output_bites_ += "1";
                } else {
                    string_output_bites_ += "0";
                }
                if (translate_.find(string_output_bites_) != translate_.end()) {
                    string_output_bites_ = translate_[string_output_bites_];
                    int output = 0;
                    int step = 1;
                    for (unsigned int i = 0; i < string_output_bites_.length(); i++) {
                        if (string_output_bites_[i] == '1') {
                            output += step;
                        }
                        step *= 2;
                    }
                    if (output < 256) {
                        if (!is_file_open_) {
                            name_of_file_out_ += static_cast<unsigned char>(output);
                        } else {
                            std::ofstream fout(name_of_file_out_, std::ios_base::app | std::ios_base::binary);
                            fout.put(static_cast<unsigned char>(output));
                            fout.close();
                        }
                    } else {
                        if (!is_file_open_) {
                            is_file_open_ = true;
                        } else {
                            symbols_count_ = 0;
                            check_end_symbols_ = 0;
                            check_end_codes_ = 0;
                            is_build_tree_ = false;
                            is_file_open_ = false;
                            translate_ = {};
                            symbols_ = {};
                            count_lengths_ = {};
                            name_of_file_out_ = "";
                        }
                    }
                    string_output_bites_ = "";
                }
            }
        }

        return answer;
    }
    
    // В этой программе делаем перевод из 9-битного зашифрованного алфавита в нормальный 8-битный, функция вызывается из BuildFiles
    void BuildDecodeTree() {
        std::string string_canonical;
        int ind = 0;
        int int_canonical;
        int leng;
        int step;
        for (unsigned int ogr = 0; ogr < count_lengths_.size(); ogr++) {
            leng = count_lengths_[ogr];
            for (int start = 0; start < leng; start++) {
                while (string_canonical.length() < ogr + 1) {
                    string_canonical += "0";
                }
                translate_[string_canonical] = symbols_[ind];

                int_canonical = 0;
                step = 1;
                for (int i = string_canonical.length() - 1; i >= 0; i--) {
                    if (string_canonical[i] == '1') {
                        int_canonical += step;
                    }
                    step *= 2;
                }

                string_canonical = "";
                int_canonical += 1;

                while (int_canonical > 0) {
                    if (int_canonical % 2 == 1) {
                        string_canonical = "1" + string_canonical;
                    } else {
                        string_canonical = "0" + string_canonical;
                    }
                    int_canonical /= 2;
                }
                while (string_canonical.length() < ogr + 1) {
                    string_canonical = "0" + string_canonical;
                }
                ind += 1;
            }
        }
    }
    
    // Возвращает int-версию вектора нулей и единиц, представляющей собой разобранную строку
    int GetCodeInt(std::vector<int>& bytes) {
        int answer = 0;
        int step = 1;

        for (int letter : bytes) {
            answer += step * letter;
            step *= 2;
        }

        return answer;
    }
    
    // Возвращает строку из нулей и единиц, представляющей собой преобразованный байт символа
    std::string GetCodeStr(int x, unsigned int right_length) {
        std::string answer;

        while (x > 0) {
            if (x % 2 == 0) {
                answer += "0";
            } else {
                answer += "1";
            }
            x /= 2;
        }
        while (answer.length() < right_length) {
            answer += "0";
        }

        return answer;
    }
    
    // Возвращает вектор из нулей и единиц, представляющий собой разобранную строку
    std::vector<int> GetBites(std::string input) {
        std::vector<int> answer;
        for (unsigned int i = 0; i < input.length(); i++) {
            if (input[i] == '0') {
                answer.push_back(0);
            } else {
                answer.push_back(1);
            }
        }
        return answer;
    }
};
