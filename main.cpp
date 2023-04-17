#include <iostream>
#include <vector>
#include <bitset>
#include <set>
#include <string>
#include <fstream>

using namespace std;

set<int> GetControlPoses(int word_size) {
    set<int> res;
    int now = 1;
    while (now <= word_size) {
        res.insert(now - 1);
        now *= 2;
    }

    return res;
}

void PrintBits(const string& data) {
    for (size_t i = 0; i < data.length(); ++i) {
        unsigned char ch = data[i];
        string b = "";
        for (int k=0; k < 8; k++, ch >>= 1) {
            b = char('0' + ch % 2) + b;
        }
        cout << b << " ";
    }
    cout << endl;
}

void PrintHammingCode(const vector<vector<bool>>& hammingCode) {
    for (const auto& word : hammingCode) {
        for (bool bit : word) {
            if (bit) {
                cout << "1";
            } else {
                cout << "0";
            }
        }
        cout << " ";
    }
    cout << endl;
}

vector<bool> ToBitset(const string& data) {
    vector<bool> result;
    for (size_t i = 0; i < data.length(); ++i) {
        vector<bool> symb;
        unsigned char ch = data[i];
        for (int k=0; k < 8; k++, ch >>= 1) {
            symb.push_back(char('0' + ch % 2) == '1');
        }
        std::copy(symb.rbegin(), symb.rend(), std::back_inserter(result));
    }
    return result;
}

void calcControlBits(vector<bool>& word, const set<int>& controlPoses) {
    for (int start_pos : controlPoses) {
        int true_count = 0;
        for (int part_start = start_pos; part_start < word.size(); part_start += 2 * (start_pos + 1)) {
            for (int j = 0; j < start_pos + 1; ++j) {
                true_count += word[part_start + j];
            }
        }
        word[start_pos] = true_count % 2;
    }
}

vector<vector<bool>> EncodeString(const string& data, const int wordSize) {
    vector<bool> dataBits = ToBitset(data);

    set<int> controlPoses = GetControlPoses(wordSize);
    int real_word_size = wordSize + controlPoses.size();

    int data_idx = 0;
    vector<vector<bool>> words;
    vector<bool> word;
    for (int word_idx = 0; word_idx < ceil(double(dataBits.size()) / wordSize); ++word_idx) {
        // Заполняем обычные биты
        for (int i = 0; i < real_word_size; ++i) {
            if (controlPoses.count(i)) {
                word.push_back(false);
            } else {
                word.push_back(dataBits[data_idx]);
                ++data_idx;
            }
        }
        //  Считаем контрольные биты
        calcControlBits(word, controlPoses);
        words.push_back(word);
        word.clear();
    }

    return words;
}

vector<int> mistakesPos;

vector<vector<bool>> TryFixWords(const vector<vector<bool>>& words, const set<int>& controlPoses) {
    vector<vector<bool>> new_words = words;
    for (auto& word : new_words) {
        for (int controlPos : controlPoses) {
            // Для удобства обнулим биты
            word[controlPos] = false;
        }
        calcControlBits(word, controlPoses);
    }
    // Статистика по словам
    int count_good = 0, count_bad = 0;
    for (int i = 0; i < new_words.size(); ++i) {
        bool eq = true;
        for (int j = 0; j < new_words[j].size(); ++j) {
            eq &= words[i][j] == new_words[i][j];
        }
        if (eq) {
            ++count_good;
        } else {
            ++count_bad;
        }
    }
    cout << "\nКоличество правильно доставленных слов = " << count_good
            << "\nКоличество неправильно доставленных слов = " << count_bad << endl;

    cout << "\nКод с переподсчитанными контрольными битами:" << endl;
    PrintHammingCode(new_words);

    int fixed_mistakes = 0;
    for (int i = 0; i < new_words.size(); ++i) {
        int bad_bit_pos = 0;
        for (int pos : controlPoses) {
            if (new_words[i][pos] != words[i][pos]) {
                // В алгоритме работает на позициях +1
                // Пока считаем так же
                bad_bit_pos += pos + 1;
            }
        }
        // Тк в алгоритме к позициям делается +1, то тут сделаем -1
        bad_bit_pos -= 1;
        if (bad_bit_pos != -1) {
            new_words[i][bad_bit_pos] = !new_words[i][bad_bit_pos];
            ++fixed_mistakes;
        }
    }
    cout << "\nВсего исправлено ошибок не в контрольных битах: " << fixed_mistakes << endl;

    for (auto& word : new_words) {
        for (int controlPos : controlPoses) {
            // Для удобства обнулим биты
            word[controlPos] = false;
        }
        calcControlBits(word, controlPoses);
    }
    cout << "Код после исправления ошибок:" << endl;
    PrintHammingCode(new_words);

    return new_words;
}

string DecodeHamming(const vector<vector<bool>>& words, const int wordSize) {
    string result = "";
    set<int> controlPoses = GetControlPoses(wordSize);
    vector<vector<bool>> new_words = TryFixWords(words, controlPoses);

    unsigned char ch = 0;
    int char_bits = 0;
    for (const auto& word : new_words) {
        for (int i = 0; i < word.size(); ++i) {
            if (char_bits == 8) {
                if (ch > 0) {
                    result.push_back(ch);
                }
                ch = 0;
                char_bits = 0;
            }
            if (controlPoses.count(i) == 0) {
                unsigned char bit = word[i] ? 1 : 0;
                unsigned char symb = bit << (7 - char_bits);
                ch |= symb;
                ++char_bits;
            }
        }
    }
    if (ch != 0) {
        result.push_back(ch);
    }

    return result;
}

void CreateOneMistakeInEachWord(vector<vector<bool>>& words) {
    int bad_word_count = 0;
    for (auto& word : words) {
        bool need_mistake = rand() % 2;
        if (need_mistake) {
            int mistake_pos = rand() % (word.size() + 1);
            word[mistake_pos] = !word[mistake_pos];
            ++bad_word_count;
        }
    }
    cout << "В count = " << bad_word_count << " слов внесена 1 ошибка" << endl;
}

void CreateMoreThanOneMistake(vector<vector<bool>>& words, int m_count) {
    int bad_word_count = 0;
    int all_mistakes_count = 0;
    for (auto& word : words) {
        int mistakes_count = rand() % (m_count + 1);
        if (mistakes_count > 0) {
            ++bad_word_count;
        }
        all_mistakes_count += mistakes_count;
        for (int _ = 0; _ < mistakes_count; ++_) {
            int mistake_pos = rand() % (word.size() + 1);
            word[mistake_pos] = !word[mistake_pos];
        }
    }
    cout << "В count = " << bad_word_count << " слов внесено суммарно " << all_mistakes_count << " ошибок" << endl;
}

uint64_t GetCRC32(const string& st) {
    uint32_t crc = 0;
    for (char ch : st) {
        crc = (crc << 5) + crc + ch;
        crc = (crc << 5) + crc;
        crc = crc ^ (crc >> 16);
    }

    return crc;
}

int main() {
    const int WORD_SIZE = 43;
    string data;
    //cin >> data;
    ifstream dataFile("data.txt");
    if (!dataFile.is_open()) {
        dataFile.open("../data.txt");
    }
    std::getline(dataFile, data);

    vector<vector<bool>> hammingCode = EncodeString(data, WORD_SIZE);

    cout << "Код Хэмминга текста:" << endl;
    PrintHammingCode(hammingCode);

    uint64_t crcData = GetCRC32(data);
    cout << "\nКонтрольная сумма исходного сообщения: " << crcData << endl;

    cout << "\nВыберите режим:\n0 - Без ошибок;\n1 - С возможными ошибками (не более 1 на слово);"
            << "\n2 - С множественными ошибками (более 1 на слово, но не обязательно во всех словах);\n";
    int mode;
    cin >> mode;
    if (mode == 1) {
        CreateOneMistakeInEachWord(hammingCode);
        cout << "\nКод Хэмминга с 1 ошибкой в каждом слове:" << endl;
        PrintHammingCode(hammingCode);
    } else if (mode == 2) {
        cout << "Максимальное количетсво ошибок в слове:" << endl;
        int m_count;
        cin >> m_count;
        CreateMoreThanOneMistake(hammingCode, m_count);
        cout << "\nКод Хэмминга с множественными ошибками в словах:" << endl;
        PrintHammingCode(hammingCode);
    }

    string result = DecodeHamming(hammingCode, WORD_SIZE);

    cout << "\nРасшифрованный текст:" << endl;
    cout << result << endl;

    uint64_t crcResult = GetCRC32(result);
    cout << "Контрольная сумма расшифрованного сообщения: " << crcResult << endl;

    cout << "\nРавенство контрольных сумм: " << (crcResult == crcData) << endl;
    cout << "Равенство текстов: " << (data == result) << endl;

    if (crcResult != crcData) {
        cout << "\nТекст не удалось расшифровать" << endl;
    } else {
        cout << "\nТекст удалось расшифровать" << endl;
    }

    return 0;
}