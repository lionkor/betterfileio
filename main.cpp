#include <iostream>
#include <cstdio>
#include <string>
#include "bfio.h"

struct Person {
    std::string name;
    int age;
    int health, stamina, mana;
};

std::ostream& operator<<(std::ostream& os, const Person& p) {
    return os << "Person: \n"
              << "\tname = " << p.name << "\n"
              << "\tage = " << p.age << "\n"
              << "\thealth = " << p.health << "\n"
              << "\tstamina = " << p.stamina << "\n"
              << "\tmana = " << p.mana;
}

int main() {
    bfio infile("person.in", bfio::READ);
    bfio outfile("person.out", bfio::WRITE);

    Person p;
    while (!infile.reached_eof()) {
        std::string key;
        infile.read_until_any_of(key, { ' ', '=' });
        infile.skip_all_of({ ' ', '=' });
        if (key == "name") {
            infile.read_line(p.name);
        } else if (key == "age") {
            infile.read(p.age);
            infile.skip();
        } else if (key == "stats") {
            infile.read(p.health);
            infile.skip_all_of({ ' ', ',' });
            infile.read(p.stamina);
            infile.skip_all_of({ ' ', ',' });
            infile.read(p.mana);
            infile.skip();
        }
    }

    std::cout << p << std::endl;
    
    outfile.write("name=", p.name, "\nage=", p.age, "\nstats=", p.health, " ", p.stamina, " ", p.mana);
}


int wmain(int, char**) {
    bfio file;
    file.adopt(stdout, bfio::WRITE);
    file.write(std::string("8 bit:\n"));
    file.write((bfio::u8)-50);
    file.write_char('\n');
    file.write((bfio::i8)-50);
    file.write_char('\n', 2);
    file.write(std::string("16 bit:\n"));
    file.write((bfio::u16)-50);
    file.write_char('\n');
    file.write((bfio::i16)-50);
    file.write_char('\n', 2);
    file.write(std::string("32 bit:\n"));
    file.write((bfio::u32)-50);
    file.write_char('\n');
    file.write((bfio::i32)-50);
    file.write_char('\n', 2);
    file.write(std::string("64 bit:\n"));
    file.write((bfio::u64)-50);
    file.write_char('\n');
    file.write((bfio::i64)-50);
    file.write_char('\n', 2);
    file.write(std::string("void*:\n"));
    file.write((void*)&file);
    file.write_char('\n', 2);

    bfio infile("testin", bfio::READ);

    std::string section;
    infile.skip(1);
    infile.read_until(section, ']');
    infile.skip_line();

    Person person;

    std::string key;

    while (!infile.reached_eof()) {
        infile.read_until_any_of(key, { '=', BFIO_WHITESPACE });
        infile.skip_all_of({ '=', BFIO_WHITESPACE });
        if (key == "name") {
            std::cout << "name!" << std::endl;
            infile.read_line(person.name);
        } else if (key == "age") {
            std::cout << "age!" << std::endl;
            infile.read(person.age);
        } else if (key == "stats") {
            std::cout << "stats!" << std::endl;
            infile.read(person.health);
            infile.skip_all_of({ ' ' });
            infile.read(person.stamina);
            infile.skip_all_of({ ' ' });
            infile.read(person.mana);
        }
    }


    std::cout << "section: " << section << "\n";
    std::cout << "name: " << person.name << "\n";
    std::cout << "age: " << person.age << "\n";
    std::cout << "health: " << person.health << "\n";
    std::cout << "stamina: " << person.stamina << "\n";
    std::cout << "mana: " << person.mana << "\n";


    std::cout << "\n\nEND" << std::endl;
}
