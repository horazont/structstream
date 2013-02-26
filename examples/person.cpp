#include <iostream>
#include <fstream>
#include <cstdint>

#include "structstream/structstream.hpp"

using namespace StructStream;

struct PersonRecord {
    std::string name;
    uint32_t age;

    void set_name(const std::string &new_name) {
        name = new_name;
    };

    const std::string &get_name() const {
        return name;
    };

    void set_age(const uint32_t new_age) {
        age = new_age;
    };

    uint32_t get_age() const {
        return age;
    };
};

typedef deserialize_block<
    0x01,
    PersonRecord,
    deserialize_string<0x02, UTF8Record, PersonRecord, &PersonRecord::set_name>,
    deserialize_custom<0x03, UInt32Record, PersonRecord, uint32_t, &PersonRecord::set_age>
    > PersonDeserializer;

typedef serialize_block<
    0x01,
    PersonRecord,
    serialize_string<0x02, UTF8Record, PersonRecord, &PersonRecord::get_name>,
    serialize_custom<0x03, UInt32Record, PersonRecord, uint32_t, &PersonRecord::get_age>
    > PersonSerializer;

int main()
{
    PersonRecord person;
    {
        std::ifstream infile("person.ss");
        if (!infile.good()) {
            std::cerr << "No input data file ... " << std::endl;
        } else {
            IOIntfHandle io(new StandardInputStream(infile));
            ContainerHandle root = bitstream_to_tree(io, RegistryHandle(new Registry()));
            Container *child = dynamic_cast<Container*>((*root->children_begin()).get());
            if (child != nullptr) {
                PersonDeserializer::deserialize(child, &person);
                std::cout << "Look what I found:" << std::endl;
                std::cout << "Name: " << person.name << std::endl;
                std::cout << "Age: " << person.age << std::endl;
                std::cout << std::endl;
            } else {
                std::cout << "Hmm, that input doesn't look ok to me. I better ignore it." << std::endl;
            }
        }
    }

    {
        std::cout << "Yay! Let's create a new person record." << std::endl;
        std::cout << "Name: " << std::flush;
        char name_buf[128];
        memset(name_buf, 0, sizeof(name_buf));
        std::cin.getline(name_buf, sizeof(name_buf)-1);
        person.name = name_buf;
        std::cout << "Age: " << std::flush;
        std::cin >> person.age;
        std::cout << "Great! I'll write it to person.ss." << std::endl;
    }

    {
        std::ofstream outfile("person.ss");
        IOIntfHandle io(new StandardOutputStream(outfile));
        NodeHandle tree = PersonSerializer::serialize(&person);
        tree_to_bitstream({tree}, io);
    }

    return 0;
}
