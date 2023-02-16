#include <iostream>
#include <pqxx/pqxx>
#include <ctime>
#include <random>

using namespace std;
using namespace pqxx;

const int Max_Ch = 26;
const int Amount_of_rows = 1000000;
const int Amount_of_rows_for_search = 100;

string RandomDate(default_random_engine &engine, uniform_int_distribution<> &distributionYear,
                  uniform_int_distribution<> &distributionMon, uniform_int_distribution<> &distributionDay) {
    string year = to_string(distributionYear(engine));
    string month = to_string(distributionMon(engine));
    string day = to_string(distributionDay(engine));
    return year + '-' + month + '-' + day;
}

string RandomString(int ch, default_random_engine &e1, uniform_int_distribution<> &distrib) {
    char alpha[Max_Ch] = {'A', 'B', 'C', 'D', 'E', 'F', 'G',
                          'H', 'I', 'J', 'K', 'L', 'M', 'N',
                          'O', 'P', 'Q', 'R', 'S', 'T', 'U',
                          'V', 'W', 'X', 'Y', 'Z'};
    string result;
    for (int i = 0; i < ch; i++)
        result += alpha[distrib(e1)];

    return result;
}

void Insert(connection &C, string &name, string &date_of_birth, char &sex) {
    work w(C);
    string statement;
    statement += "INSERT INTO Persons(Name, Date_of_birth, Sex) VALUES('" + name
                 + "', '" + date_of_birth + "', '" + sex + "')";
    w.exec(statement);
    w.commit();
}

int CountAge(const string& date_of_birth) {
    int day, month, year;
    struct tm date = {0};

    year = stoi(date_of_birth.substr(0, 4));
    month = stoi(date_of_birth.substr(5, 2));
    day = stoi(date_of_birth.substr(8, 2));

    date.tm_year = year - 1900;
    date.tm_mon = month - 1;
    date.tm_mday = day;
    time_t normal = mktime(&date);
    time_t current;
    time(&current);
    int age = (int) ((difftime(current, normal) + 86400L / 2) / 86400L);
    return age / 365;
}

int main(int argc, char *argv[]) {
    try {
        connection C("dbname = PTMK user = berserktears password = 0027 \
      hostaddr = 127.0.0.1 port = 5432");
        if (C.is_open()) {
            cout << "Opened database successfully: " << C.dbname() << endl;
            if (strcmp(argv[1], "1") == 0) {
                work w(C);
                w.exec("CREATE TABLE Persons( \
                Name varchar(80), \
                Date_of_birth date, \
                Sex varchar(1), \
                UNIQUE (Name, Date_of_birth));");
                w.commit();
            }
            if (strcmp(argv[1], "2") == 0) {
                short i = 2;
                string name;
                while (i != argc - 2) {
                    if (i != 2)
                        name += " ";
                    name += argv[i];
                    i++;
                }
                string date(argv[argc - 2]);
                Insert(C, name, date, *argv[argc - 1]);
            }
            if (strcmp(argv[1], "3") == 0) {
                work w(C);
                result response = w.exec("SELECT * FROM Persons ORDER BY Name; ");
                w.commit();
                for (auto &&i: response) {
                    cout << i[0] << " " << i[1] << " " << i[2] << " " << CountAge(i[1].as<string>()) << endl;
                }
            }
            if (strcmp(argv[1], "4") == 0) {
                random_device r;
                default_random_engine e1(r());
                uniform_int_distribution<> distribName(0, Max_Ch - 1);
                uniform_int_distribution<> distribSex(0, 1);
                uniform_int_distribution<> distribDateYear(1950, 2015);
                uniform_int_distribution<> distribDateMonth(1, 12);
                uniform_int_distribution<> distribDateDay(1, 28);
                for (int i = 0; i < Amount_of_rows; ++i) {
                    string name = RandomString(40, e1, distribName);
                    char sex = distribSex(e1) == 1 ? 'M' : 'F';
                    string date = RandomDate(e1, distribDateYear, distribDateMonth, distribDateDay);
                    Insert(C, name, date, sex);
                }
                for (int i = 0; i < Amount_of_rows_for_search; ++i) {
                    string name = 'F' + RandomString(40, e1, distribName);
                    char sex = 'M';
                    string date = RandomDate(e1, distribDateYear, distribDateMonth, distribDateDay);
                    Insert(C, name, date, sex);
                }
            }
            if (strcmp(argv[1], "5") == 0) {
                clock_t begin = clock();
                work w(C);
                result response = w.exec("SELECT * FROM Persons WHERE (sex='M') AND (Name LIKE 'F%'); ");
                w.commit();
                clock_t end = clock();
                cout << (double) (end - begin) / CLOCKS_PER_SEC << " seconds";
            }
            if (strcmp(argv[1], "6") == 0) {
                work w(C);
                result response = w.exec("CREATE INDEX persons_name_sex_idx on persons(name, sex);");
                w.commit();
            }
        } else {
            cout << "Can't open database" << endl;
            return 1;
        }
        C.close();
    } catch (const std::exception &e) {
        cerr << e.what() << std::endl;
        return 1;
    }
}