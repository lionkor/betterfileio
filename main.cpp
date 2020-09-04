#include <iostream>
#include <cstdio>
#include <string>
#include "bfio.h"

struct Person {
    std::string name;
    int age;
};

int main(int, char**) {
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

    file.write("enter u8: ");
    bfio infile("testin", bfio::READ);

    std::string section;
    infile.skip(1);
    infile.read_until(section, ']');
    infile.skip_line();

    Person person;

    std::string key;

    while (!infile.reached_eof()) {
        infile.read_until_any_of(key, { '=', ' ', '\t' });
        infile.skip_whitespace();
        infile.skip(1);
        infile.skip_whitespace();
        if (key == "name") {
            std::cout << "name!" << std::endl;
            infile.read_line(person.name);
        } else if (key == "age") {
            std::cout << "age!" << std::endl;
            infile.read(person.age);
        } else {
            std::cout << "nope! it's _" << key << "_" << std::endl;
        }
    }


    std::cout << "section: " << section << "\n";
    std::cout << "name: " << person.name << "\n";
    std::cout << "age: " << person.age << "\n";


    std::cout << "\n\nEND" << std::endl;
}
